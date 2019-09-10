#include <arpa/inet.h>
#include <assert.h>
#include <errno.h>
#include <netinet/in.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <unistd.h>

#include "server.h"

/* HTTP response and header for a successful request */
static char *ok_response =
    "HTTP/1.0 200 OK\n"
    "Content-type: text/html\n"
    "\n";

/* HTTP response, header, and body, indicating that we didn't 
   understand the request */
static char *bad_request_response =
    "HTTP/1.0 400 Bad Request\n"
    "Content-type: text/html\n"
    "\n"
    "<html>\n"
    "   <body>\n"
    "       <h1>Bad Request</h1>\n"
    "       <p>This server did not understand your request.</p>\n"
    "   </body>\n"
    "</html>\n";

/* HTTP response, haeder, and body template, indicating that the
   requested document was not found */
static char *not_found_response_template =
    "HTTP/1.0 404 Not Found\n"
    "Content-type: text/html\n"
    "\n"
    "<html>\n"
    "   <body>\n"
    "       <h1>Not Found</h1>\n"
    "       <p>This requested URL %s was not found on this server.</p>\n"
    "   </body>\n"
    "</html>\n";

/* HTTP response, header, and body template, indicating that the 
   method was not understand */
static char *bad_method_response_template =
    "HTTP/1.0 501 Method Not Implemented\n"
    "Content-type: text/html\n"
    "\n"
    "<html>\n"
    "   <body>\n"
    "       <h1>Method Not Imeplemented</h1>\n"
    "       <p>The method %s is not implemented by this server.</p>\n"
    "   </body>\n"
    "</html>\n";

/* handler for SIGCHLD, to clean up child process that have terminated */
static void clean_up_child_process(int signal_number)
{
    // @todo signal_number的作用是什么
    int status;
    wait(&status);
}

/* process an HTTP "GET" request for PAGE, and send the results to the 
   file descriptor CONNECTION_FD */
static void handle_get(int connection_fd, const char *page)
{
    struct server_module *module = NULL;
    if (*page == '/' && strchr(page + 1, '/') == NULL)
    {
        char module_file_name[64];

        snprintf(module_file_name, sizeof(module_file_name), "%s.so", page + 1);

        module = module_open(module_file_name);
    }

    if (module == NULL)
    {
        char response[1024];
        snprintf(response, sizeof(response), not_found_response_template, page);

        write(connection_fd, response, strlen(response));
    }
    else
    {
        write(connection_fd, ok_response, strlen(ok_response));
        (*module->generate_function)(connection_fd);
        module_close(module);
    }
}

static void handle_connection(int connection_fd)
{
    char buffer[256];
    ssize_t bytes_read;

    /* read some data from the client */
    bytes_read = read(connection_fd, buffer, sizeof(buffer) - 1);
    if (bytes_read > 0)
    {
        char method[sizeof(buffer)];
        char url[sizeof(buffer)];
        char protocol[sizeof(buffer)];

        /* some data was read successfully, NUL - terminate the buffer so
           we can use string operations on it */
        buffer[bytes_read] = '\0';
        /* thre first line the client sends is the HTTP request, which
           is composed of a method, the request page, and the protocol version */
        sscanf(buffer, "%s %s %s", method, url, protocol);

        /* the client may send various header information following the
           request. for this HTTP implementation, we don't care about it.
           However, we need to read any data the client tries to send.
           Keep on reading data until we get to the end of the header, which
           is delimited by a blank line. HTTP specifies CR/LF as the line delimiter. */
        while (strstr(buffer, "\r\n\r\n") == NULL)
            bytes_read = read(connection_fd, buffer, sizeof(buffer));

        /* make sure the last read didn't fail. if it did, there's a
           problem with the connection. so give up */
        if (bytes_read == -1)
        {
            close(connection_fd);
            return;
        }

        /* check the protocal field. we unserstand HTTP versions 1.0 and 1.1*/
        if (strcmp(protocol, "HTTP/1.0") && strcmp(protocol, "HTTP/1.1"))
        {
            write(connection_fd, bad_request_response, sizeof(bad_request_response));
        }
        else if (strcmp(method, "GET"))
        {
            char response[1024];

            snprintf(response, sizeof(response), bad_method_response_template, method);
            write(connection_fd, response, strlen(response));
        }
        else
            handle_get(connection_fd, url);
    }
    else if (bytes_read == 0)
        /* nothing to do */
        ;
    else
        system_error("read");
}

void server_run(struct in_addr local_address, uint16_t port)
{
    struct sockaddr_in socket_address;
    int rval;
    struct sigaction sigchld_action;
    int server_socket;

    /* install a handler for SIGCHLD that cleans up child process that
       have terminated */
    memset(&sigchld_action, 0, sizeof(sigchld_action));
    sigchld_action.sa_handler = &clean_up_child_process;
    sigaction(SIGCHLD, &sigchld_action, NULL);

    /* create a TCP socket */
    server_socket = socket(PF_INET, SOCK_STREAM, 0);
    if (server_socket == -1)
        system_error("socket");

    /* construct a socket address structure for the local address on
       which we want to listen for connections */
    memset(&socket_address, 0, sizeof(socket_address));
    socket_address.sin_family = AF_INET;
    socket_address.sin_port = port;
    socket_address.sin_addr = local_address;

    /* bind the socket to that address */
    rval = bind(server_socket, (__SOCKADDR_ARG)&socket_address, sizeof(socket_address));
    if (rval != 0)
        system_error("bind");

    /* instruct the socket to accept connections */
    rval = listen(server_socket, 10);
    if (rval != 0)
        system_error("listen");

    if (verbose)
    {
        socklen_t address_length;

        address_length = sizeof(socket_address);
        rval = getsockname(server_socket, (struct sockaddr *)&socket_address, &address_length);
        assert(rval == 0);
        printf("server listening on %s:%d\n",
               inet_ntoa(socket_address.sin_addr),
               (int)ntohs(socket_address.sin_port));
    }

    /* loop forever, handling connections */
    while (1)
    {
        struct sockaddr_in remote_address;
        socklen_t address_length;
        int connection;
        pid_t child_pid;

        /* accept a connection. this call blocks until a connection is ready */
        address_length = sizeof(remote_address);
        connection = accept(server_socket, (struct sockaddr *)&remote_address, &address_length);
        if (connection == -1)
        {
            if (errno == EINTR)
                continue;
            else
                system_error("accept");
        }

        if (verbose)
        {
            socklen_t address_length;
            address_length = sizeof(socket_address);
            rval = getpeername(connection, (struct sockaddr *)&socket_address, &address_length);
            assert(rval == 0);
            printf("connection accepted from %s\n", inet_ntoa(socket_address.sin_port));
        }

        child_pid = fork();
        if (child_pid == 0)
        {
            close(STDIN_FILENO);
            close(STDOUT_FILENO);
            close(server_socket);
            handle_connection(connection);
            close(connection);
            exit(0);
        }
        else if (child_pid > 0)
        {
            close(handle_connection);
        }
        else
            system_error("fork");
    }
}