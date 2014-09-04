/*
facility:
       LOG_AUTH
	   LOG_AUTHPRIV
	   LOG_CRON
	   LOG_DAEMON
	   LOG_KERN
	   LOG_LOCAL0 through LOG_LOCAL7
	   LOG_LPR
	   LOG_MAIL
	   LOG_NEWS
	   LOG_SYSLOG
	   LOG_USER(default)
	   LOG_UUCP

level:
	LOG_EMERG
    LOG_ALERT
    LOG_CRIT
    LOG_ERR
    LOG_WARNING
    LOG_NOTICE
    LOG_INFO
    LOG_DEBUG
*/

#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include <syslog.h>

#include "xyz_log.h"

static int g_xyz_log_level = 6;
static char g_xyz_log_tag[64] = {0};

void xyz_log_tag(char *tag) 
{
    if(tag) {
        snprintf(g_xyz_log_tag, sizeof(g_xyz_log_tag), "[%s]", tag);
    }

    return;
}

void xyz_log_open(const char* ident, int facility, int level)
{
	g_xyz_log_level = level;

	openlog(ident, LOG_PID, facility);

	return;
}

void xyz_log_write(int level, const char* format, ...)
{
	if(level > g_xyz_log_level) {
		return;
	}

	va_list args;
	va_start(args, format);
	vsyslog(level, format, args);
	va_end(args);

	return;
}

void xyz_log_close(void)
{
	closelog();

	return;
}

//////////////////////////////////////////////////////////////////////////////

#if 0
int main(void)
{
	xyz_log_open("hello", LOG_MAIL, LOG_INFO);

	xyz_log_write(LOG_INFO, "write info");
	xyz_log_write(LOG_DEBUG, "write debug");

    // xyz_log_tag("tag");

	LOGI("write info 2");
	LOGD("write debug 2");

	xyz_log_close();

	return 0;
}
#endif 

