#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "server.h"

const char *program_name;
int verbose;

void debug(char *function_name, char *message)
{
    printf("%s -> %s\n", function_name, message);
}

void *xmalloc(size_t size)
{
    void *ptr = malloc(size);
    if (ptr == NULL)
        abort();
    else
    {
        return ptr;
    }
}

void *xrealloc(void *ptr, size_t size)
{
    ptr = realloc(ptr, size);
    if (ptr == NULL)
        abort();
    else
        return ptr;
}

char *xstrdup(const char *s)
{
    char *copy = strdup(s);
    if (copy == NULL)
        abort();
    else
        return copy;
}

void system_error(const char *operation)
{
    /* generate an error message for errno */
    error(operation, strerror(errno));
}

void error(const char *cause, const char *message)
{
    /* print an error message to stderr */
    fprintf(stderr, "%s: error: (%s) %s\n", program_name, cause, message);
    /* end program */
    exit(1);
}

char *get_self_executable_directory()
{
    int rval;
    char link_target[1024];
    char *last_slash;
    size_t result_length;
    char *result;

    rval = readlink("/proc/self/exe", link_target, sizeof(link_target));
    if (rval == -1)
        abort();
    else
        link_target[rval] = '\0';

    last_slash = strrchr(link_target, '/');
    if (last_slash == NULL || last_slash == link_target)
        abort();

    result_length = last_slash - link_target;
    result = (char *)xmalloc(result_length + 1);
    strncpy(result, link_target, result_length);
    result[result_length] = '\0';

    return result;
}