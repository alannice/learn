
#ifndef __ZYX_PLUGIN_H__
#define __ZYX_PLUGIN_H__

#define XYZ_PLUGIN_SUCES   0       // 成功
#define XYZ_PLUGIN_INIT    (-1)    // 初始化阶段
#define XYZ_PLUGIN_DESTROY (-2)    // 释放资源阶段
#define XYZ_PLUGIN_ONCE    (-3)    // 只执行一次
#define XYZ_PLUGIN_SKIP    (-4)    // 本次调用被跳过
#define XYZ_PLUGIN_ERROR   (-5)    // 处理过程中出错
#define XYZ_PLUGIN_BAD     (-6)    // 处理的数据有错误
#define XYZ_PLUGIN_FINISH  (-7)    // 处理已结束

#define XYZ_PLUGIN_PATH "./plugin"

#define XYZ_SUBPLUGIN_INIT "xyz_subplugin_init"
#define XYZ_SUBPLUGIN_PROC "xyz_subplugin_proc"
#define XYZ_SUBPLUGIN_DESTROY "xyz_subplugin_destroy"

typedef int (*xyz_plugin_func_t)(void *data, void *args);
typedef int (*xyz_plugin_none_t)();

struct xyz_plugin_node_t {
    char name[32];
    int state;
    void *handle;
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
void xyz_plugin_destroy(struct xyz_plugin_t *plugin);
void xyz_plugin_state(struct xyz_plugin_t *plugin);

#endif // __ZYX_PLUGIN_H__

