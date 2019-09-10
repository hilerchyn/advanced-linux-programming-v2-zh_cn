#ifndef SERVER_H
#define SERVER_H

#include <netinet/in.h>
#include <sys/types.h>

/*** Symbols defined in common.c ***/

/* The name of this program */
extern const char *program_name;

/* If nonzero, print verbose messages */
extern int verbose;

/* Like malloc, except aborts the program if allocation fails */
extern void *xmalloc(size_t size);

/* Like realloc, except aborts the program if allocation fails */
extern char *xstrdup(const char *s);

/* print an error message for a failed call OPERATION, using the value of errno, 
    and end the program */
extern void system_error(const char *operation);

/* print an error message for failure involving CAUSE, including a
    descriptive MESSAGE, and end the program */
extern void error(const char *cause, const char *message);

/* return the directory containg the running program's executable.
    the return value is a memory buffer that the caller must deallocate
    using free. this function calls abort on failure */
extern char *get_self_executable_directory();

/*** Symbos defined in module.c ***/

/* an instance of a loaded server module */
struct server_module
{
    /* thre shared library handle corresponding to the loaded module */
    void *handle;

    /* a name describing the module */
    const char *name;

    /* the function that generates the HTML results for this module */
    void (*generate_function)(int);
};

/* the directory from which modules are loaded */
extern char *module_dir;

/* attempt to load a server module with the name MODULE_PATH. if a
    server module exists with this path, loads the module and return a
    server_module structure representing it. otherwise, returns NULL.*/
extern struct server_module *module_open(const char *module_path);

/* close a server module and deallocate the MODULE object */
extern void module_close(struct server_module *module);

/*** Symbols defined in server.c ***/

/* run the server on LOCAL_ADDRESS and PORT */
extern void server_run(struct in_addr local_address, uint16_t port);

#endif