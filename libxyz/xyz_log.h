
#ifndef __XYZ_LOG_H__
#define __XYZ_LOG_H__

#include <syslog.h>

extern char g_xyz_log_tag[];

void xyz_log_tag(char *tag);
void xyz_log_open(const char* ident, int facility, int level);
void xyz_log_write(int level, const char* format, ...);
void xyz_log_close(void);

#define LOGE(fmt, arg...) xyz_log_write(LOG_ERR, "[err]%s "fmt, g_xyz_log_tag, ##arg)
#define LOGW(fmt, arg...) xyz_log_write(LOG_WARNING, "[warn]%s "fmt, g_xyz_log_tag, ##arg)
#define LOGI(fmt, arg...) xyz_log_write(LOG_INFO, "[info]%s "fmt, g_xyz_log_tag, ##arg)

#define LOGD(fmt, arg...) xyz_log_write(LOG_DEBUG, "[dbg]%s [%s:%s():%d] "fmt, g_xyz_log_tag, __FILE__, __func__, __LINE__, ##arg)

#endif // __XYZ_LOG_H__

