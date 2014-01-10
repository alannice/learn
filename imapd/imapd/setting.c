
#include "setting.h"

int setting_init(void)
{
    memset(&g_setting, '\0', sizeof(struct setting_t));

    g_setting.timeout = 30;
    g_setting.loglevel = 6;
    g_setting.errcmd = 5;

    g_setting.usessl = 0;

    return 0;
}

int setting_load(char *file)
{
    struct xyz_conf_t *conf;

    conf = xyz_conf_load(file);
    if (conf == NULL) {
        LOGE("config load error : %s.", file);
        return -1;
    }

    if (xyz_conf_number(conf, "loglevel") > 0) {
        g_setting.loglevel = xyz_conf_number(conf, "loglevel");
    }
    if (xyz_conf_number(conf, "timeout") > 0) {
        g_setting.timeout = xyz_conf_number(conf, "timeout");
    }
    if (strlen(xyz_conf_string(conf, "defdomain")) > 0) {
        strncpy(g_setting.defdomain, xyz_conf_string(conf, "defdomain"), sizeof(g_setting.defdomain)-1);
    }
    if (xyz_conf_number(conf, "errcmd") > 0) {
        g_setting.errcmd = xyz_conf_number(conf, "errcmd");
    }
    if (xyz_conf_number(conf, "usessl") > 0) {
        g_setting.usessl = xyz_conf_number(conf, "usessl");
    }
    if (xyz_conf_string(conf, "pemfile") > 0) {
        strncpy(g_setting.pemfile, xyz_conf_string(conf, "pemfile"), sizeof(g_setting.pemfile)-1);
    }

    xyz_conf_destroy(conf);

    if(strlen(g_setting.defdomain) == 0) {
        LOGE("not set default domain.");
        return -1;
    }
    if(g_setting.usessl) {
        if(strlen(g_setting.pemfile) == 0) {
            LOGE("use ssl but not set pemfile.");
            return -1;
        }
    }

    return 0;
}

// end

