#include <dlfcn.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "server.h"

char *module_dir;

struct server_module *module_open(const char *module_name)
{
    char *module_path;
    void *handle;
    void (*module_generate)(int);
    struct server_module *module;

    module_path = (char *)xmalloc(strlen(module_dir) + strlen(module_name) + 2);
    sprintf(module_path, "%s/%s", module_dir, module_name);

    handle = dlopen(module_path, RTLD_NOW);
    free(module_path);
    if (handle == NULL)
    {
        /* failed: either this path doesn't exist or it isn't a shared library */
        return NULL;
    }

    /* resolve the module_generate symbol from the shared library */
    module_generate = (void (*)(int))dlsym(handle, "module_generate");
    /* make sure the symbol was found */
    if (module_generate == NULL)
    {
        /* the symbol is missing. while this is a shared library, it 
           probably isn't a server module. close up and indicate failure */

        dlclose(handle);
        return NULL;
    }

    /* allocate and initialize a server_module object */
    module = (struct server_module *)xmalloc(sizeof(struct server_module));
    module->handle = handle;
    module->name = xstrdup(module_name);
    module->generate_function = module_generate;

    /* return it, indicating success */
    return module;
}

void module_close(struct server_module *module)
{
    /* close the shared library */
    dlclose(module->handle);

    /* deallocate the module name */
    free((char *)module->name);

    /* deallocate the module object */
    free(module);
}