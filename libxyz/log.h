
#ifndef __LOG_H__
#define __LOG_H__

#include <syslog.h>

void log_open(const char* ident, int facility, int level);
void log_write(int level, const char* format, ...);
void log_close(void);

#define LOGE(fmt, arg...) log_write(LOG_ERR, "[err] "#fmt, ##arg)
#define LOGW(fmt, arg...) log_write(LOG_WARNING, "[warn] "#fmt, ##arg)
#define LOGI(fmt, arg...) log_write(LOG_INFO, "[info] "#fmt, ##arg)

#define LOGD(fmt, arg...) log_write(LOG_DEBUG, "[dbg][%s:%s():%d] "#fmt, __FILE__, __func__, __LINE__, ##arg)

#endif // __LOG_H__
