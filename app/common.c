#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "server.h"

const char *program_name;
int verbose;

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