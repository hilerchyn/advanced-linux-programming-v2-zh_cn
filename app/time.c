#include <assert.h>
#include <stdio.h>
#include <sys/time.h>
#include <time.h>

#include "server.h"

static char *page_template =
    "<html>\n"
    "   <head>\n"
    "       <meta http-equiv=\"refresh\" content=\"5\">\n"
    "   </head>\n"
    "   <body>\n"
    "       <p>The current time is %s.</p>\n"
    "   </body>\n"
    "</html>\n";

void module_generate(int fd)
{
    struct timeval tv;
    struct tm *ptm;
    char time_string[40];
    FILE *fp;

    gettimeofday(&tv, NULL);
    ptm = localtime(&tv.tv_sec);
    strftime(time_string, sizeof(time_string), "%H:%M:%S", ptm);

    fp = fdopen(fd, "W");
    assert(fp != NULL);
    fprintf(fp, page_template, time_string);
    fflush(fp);
}