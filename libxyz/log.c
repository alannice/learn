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

#include <stdarg.h>
#include <syslog.h>

#include "log.h"

static int g_log_level;

void log_open(const char* ident, int facility, int level)
{
	g_log_level = level;

	openlog(ident, LOG_PID, facility);

	return;
}

void log_write(int level, const char* format, ...)
{
	if(level > g_log_level) {
		return;
	}

	va_list args;
	va_start(args, format);
	vsyslog(level, format, args);
	va_end(args);

	return;
}

void log_close(void)
{
	closelog();

	return;
}

/////////////////////////////////////////////////
/*
int main(void)
{
	log_open("hello", LOG_MAIL, LOG_DEBUG);

	log_write(LOG_INFO, "write info");
	log_write(LOG_DEBUG, "write debug");

	LOGI("write info 2");
	LOGD("write debug 2");

	log_close();

	return 0;
}
*/
