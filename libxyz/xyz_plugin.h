
#ifndef __ZYX_PLUGIN_H__
#define __ZYX_PLUGIN_H__

#define XYZ_PLUGIN_STAT_INIT 1
#define XYZ_PLUGIN_STAT_DESTROY 2
#define XYZ_PLUGIN_STAT_ONCE 3
#define XYZ_PLUGIN_STAT_SKIP 4
#define XYZ_PLUGIN_STAT_SUCE 5
#define XYZ_PLUGIN_STAT_BAD 6

#define XYZ_PLUGIN_PATH "./plugin"

#define XYZ_PLUGIN_INIT "plugin_init"
#define XYZ_PLUGIN_PROC "plugin_proc"
#define XYZ_PLUGIN_DESTROY "plugin_destroy"

typedef int (*xyz_plugin_func_t)(void *data, void *args);
typedef int (*xyz_plugin_none_t)();

struct xyz_plugin_node_t {
    char name[32];
    int state;
    xyz_plugin_none_t plugin_init;
    xyz_plugin_func_t plugin_proc;
    xyz_plugin_none_t plugin_destroy;

    struct xyz_plugin_node_t *next;
};

struct xyz_plugin_t {
    char label[32];
    int state;
    char path[128];
    struct xyz_plugin_node_t *list;    
};  

struct xyz_plugin_t *xyz_plugin_init(char *label, char *path);
int xyz_plugin_proc(struct xyz_plugin_t *plugin, void *data, void *args);
void xyz_plug_destroy(struct xyz_plugin_t *plugin);
void xyz_plug_state(struct xyz_plugin_t *plugin);

#endif // __ZYX_PLUGIN_H__

