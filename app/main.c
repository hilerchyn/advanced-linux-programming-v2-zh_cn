#include <assert.h>
#include <getopt.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#include "server.h"

/* description of long options for getopt_long */
static const struct option long_options[] = {
    {"address", 1, NULL, 'a'},
    {"help", 0, NULL, 'h'},
    {"module-dir", 1, NULL, 'm'},
    {"port", 1, NULL, 'p'},
    {"verbose", 0, NULL, 'v'},
};

static const char *const short_options = "a:hm:p:v";

static const char *const usage_template =
    "Usage: %s [ options ]]\n"
    "   -a, --address ADDR      Bind to local address (by default, bind\n"
    "                               to all local addresses).\n"
    "   -h, --help              Print this information.\n"
    "   -m, --module-dir DIR    Load modules from specified directory\n"
    "                               (by default, use executable directory).\n"
    "   -p, --port PORT         Bind to specified port.\n"
    "   -v, --verbose           Print verbose message.\n";

static void print_usage(int is_error)
{
    fprintf(is_error ? stderr : stdout, usage_template, program_name);
    exit(is_error ? 1 : 0);
}

int main(int argc, char *const argv[])
{
    struct in_addr local_address;
    uint16_t port;
    int next_option;

    program_name = argv[0];

    local_address.s_addr = INADDR_ANY;
    port = 0;
    verbose = 0;
    debug("main", "before get module dir");        
    module_dir = get_self_executable_directory();
    assert(module_dir != NULL);

    debug("main", "before do");

    do
    {
        next_option = getopt_long(argc, argv, short_options, long_options, NULL);

        debug("main", "before switch");
        printf("%d\n", next_option);

        if(next_option<0){
            break;
        }

        switch (next_option)
        {
        case 'a':
        {
            struct hostent *lcoal_host_name;
            lcoal_host_name = gethostbyname(optarg);
            if (lcoal_host_name == NULL || lcoal_host_name->h_length == 0)
                error(optarg, "invalid host name");
            else
                local_address.s_addr = *((int *)(lcoal_host_name->h_addr_list[0]));

            debug("main", "after address argument");
        }
        break;

        case 'h':
            print_usage(0);

        case 'm':
        {
            struct stat dir_info;
            if (access(optarg, F_OK) != 0)
                error(optarg, "module directory does not exist");

            if (access(optarg, R_OK | X_OK))
                error(optarg, "module directory is not accessible");

            if (stat(optarg, &dir_info) != 0 || !S_ISDIR(dir_info.st_mode))
                error(optarg, "not a directory");

            module_dir = strdup(optarg);
        }
        break;

        case 'p':
        {
            long value;
            char *end;

            debug("main", "before port argument");

            value = strtol(optarg, &end, 10);
            if (*end != '\0')
                print_usage(1);

            port = (uint16_t)htons(value);

            debug("main", "after port argument");
        }
        break;

        case 'v':
            verbose = 1;
            break;
        case '?':
            print_usage(1);

        default:
            abort();
        }

    } while (next_option != -1);

    debug("main", "after do...while");

    printf("optind: %d, argc: %d\n", optind, argc);

    if (optind+1 != argc)
        print_usage(1);

    if (verbose)
        printf("modules will be loaded from %s\n", module_dir);

    server_run(local_address, port);

    return 0;
}