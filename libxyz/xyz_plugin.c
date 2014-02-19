
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/types.h>
#include <dirent.h>
#include <dlfcn.h>

#include "xyz_plugin.h"

int xyz_plugin_add(struct xyz_plugin_t *plugin, struct xyz_plugin_node_t *node)
{
    if(plugin == NULL || node == NULL) {
        return -1;
    }

    if(plugin->list == NULL) {
        plugin->list = node;
    } else {
        struct xyz_plugin_node_t *tmp = plugin->list;
        while(tmp->next) {
            tmp = tmp->next;
        }
        tmp->next = node;
    }

    return 0;
}

int xyz_plugin_scan(struct xyz_plugin_t *plugin)
{
    struct xyz_plugin_node_t *node;
    char path[256];

    if(plugin == NULL) {
        return -1;
    }

   DIR *pdir = opendir(plugin->path);
   if (pdir == NULL) {
       return -1;
   }

   struct dirent *dp;
   while ((dp = readdir(pdir)) != NULL) {
       if (strcmp(dp->d_name, ".") == 0 || strcmp(dp->d_name, "..") == 0) {
           continue;
       }

       if(strstr(dp->d_name, ".so") == NULL) {
           continue;
       }
       memset(path, '\0', sizeof(path));
       sprintf(path, "%s/%s", plugin->path, dp->d_name);
        
       void *handle = dlopen(path, RTLD_LAZY);
       if(handle == NULL) {
           continue;
       }

       node = malloc(sizeof(struct xyz_plugin_node_t));
       if(node == NULL) {
           break;
       }
       memset(node, '\0', sizeof(struct xyz_plugin_node_t));
       node->handle = handle;
       snprintf(node->name, sizeof(node->name)-1, "%s", dp->d_name); 
       node->plugin_init = dlsym(handle, XYZ_SUBPLUGIN_INIT);
       node->plugin_proc = dlsym(handle, XYZ_SUBPLUGIN_PROC);
       node->plugin_destroy = dlsym(handle, XYZ_SUBPLUGIN_DESTROY);

       if(node->plugin_init == NULL || node->plugin_proc == NULL || 
               node->plugin_destroy == NULL) {
           dlclose(handle);
           free(node);
       }

       if(node->plugin_init() == 0) {
           xyz_plugin_add(plugin, node);
       } else {
           dlclose(handle);
           free(node);
       }
   }

   closedir(pdir);

   return 0;
}

struct xyz_plugin_t *xyz_plugin_init(char *label, char *path)
{
    struct xyz_plugin_t *plugin;

    plugin = malloc(sizeof(struct xyz_plugin_t));
    if(plugin == NULL) {
        return NULL;
    }

    if(label) {
        snprintf(plugin->label, sizeof(plugin->label)-1, "%s", label);
    } else {
        snprintf(plugin->label, sizeof(plugin->label)-1, "%s", "None");
    }
    if(path) {
        snprintf(plugin->path, sizeof(plugin->path)-1, "%s", path);
    } else {
        snprintf(plugin->path, sizeof(plugin->path)-1, "%s", XYZ_PLUGIN_PATH);
    }
    plugin->list = NULL;

    xyz_plugin_scan(plugin);

    return plugin;
}

int xyz_plugin_proc(struct xyz_plugin_t *plugin, void *data, void *args)
{
    int retval = 0;

    if(plugin == NULL) {
        return 0;
    }

    struct xyz_plugin_node_t *tmp = plugin->list;
    while(tmp) {
        retval = tmp->plugin_proc(data, args);
        if(retval < 0 ) {
            // xxxxxx;
        }
        tmp = tmp->next;
    }

    return 0;
}

void xyz_plugin_destroy(struct xyz_plugin_t *plugin)
{
    if(plugin == NULL) {
        return;
    }

    struct xyz_plugin_node_t *tmp;
    while(plugin->list) {
        tmp = plugin->list;
        plugin->list = plugin->list->next;
        tmp->plugin_destroy();
        dlclose(tmp->handle);
        free(tmp);
    }
    
    free(plugin);

    return;
}

void xyz_plugin_state(struct xyz_plugin_t *plugin)
{
    if(plugin == NULL) {
        return;
    }

    printf("---xyz_plugin state---\n");
    printf("label: %s\n", plugin->label);
    printf("path: %s\n", plugin->path);
    printf("state: %d\n", plugin->state);

    if(plugin->list) {
        struct xyz_plugin_node_t *tmp = plugin->list;
        while(tmp) {
            printf("  name: %s\n", tmp->name);
            printf("  state: %d\n", tmp->state);
            printf("  handle: %p\n", tmp->handle);
            printf("  plugin_init: %p\n", tmp->plugin_init);
            printf("  plugin_proc: %p\n", tmp->plugin_proc);
            printf("  plugin_destroy: %p\n\n", tmp->plugin_destroy);

            tmp = tmp->next;
        }
    }

    return;
}

#if 0

int main(void)
{
    struct xyz_plugin_t *plug = xyz_plugin_init("plugin.test", "./plugin/");
    if(plug == NULL) {
        return 1;
    }

    xyz_plugin_state(plug);

    xyz_plugin_proc(plug, "hello", "world");
    xyz_plugin_proc(plug, "world", "hello");

    xyz_plugin_destroy(plug);

    return 0;
}

// test so code plugin_echo.c
// cc shared -o plugin_echo.so plugin_echo.c
//
// #include <stdlib.h>
// #include <stdio.h>
// #include <string.h>
// #include <unistd.h>
//
// int xyz_subplugin_init()
// {
//     printf("plug echo init\n");
//
//     return 0;
// }
//
// int xyz_subplugin_proc(void *data, void *args)
// {
//     printf("plug echo proc : %s %s\n", data, args);
//
//     return 0;
// }
//
// int xyz_subplugin_destroy()
// {
//     printf("plug echo destroy\n");
//
//     return 0;
// }
//

#endif 

