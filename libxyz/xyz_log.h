
#ifndef __XYZ_LOG_H__
#define __XYZ_LOG_H__

#include <syslog.h>

void xyz_log_open(const char* ident, int facility, int level);
void xyz_log_write(int level, const char* format, ...);
void xyz_log_close(void);

#define LOGE(fmt, arg...) xyz_log_write(LOG_ERR, "[err] "#fmt, ##arg)
#define LOGW(fmt, arg...) xyz_log_write(LOG_WARNING, "[warn] "#fmt, ##arg)
#define LOGI(fmt, arg...) xyz_log_write(LOG_INFO, "[info] "#fmt, ##arg)

#define LOGD(fmt, arg...) xyz_log_write(LOG_DEBUG, "[dbg][%s:%s():%d] "#fmt, __FILE__, __func__, __LINE__, ##arg)

#endif // __XYZ_LOG_H__

