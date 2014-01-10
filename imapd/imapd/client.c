
#include "client.h"

extern struct setting_t g_setting;
extern struct xyz_event_t *g_event;

////////////////////////////////////////////////

int email_check(void)
{
	if(g_client.user[strlen(g_client.user)-1] == '@') {
		strncat(g_client.user, g_setting.defdomain, sizeof(g_client.user)-strlen(g_client.user)-1);
	}
	if(strchr(g_client.user, '@') == NULL) {
		strncat(g_client.user, "@", sizeof(g_client.user)-strlen(g_client.user)-1);
		strncat(g_client.user, g_setting.defdomain, sizeof(g_client.user)-strlen(g_client.user)-1);
	}

	return 0;
}

int imparse_isatom( const char *s )
{
	int len = 0;

	if (!*s)
		return 0;

	for (; *s; s++) {
		len++;
		if (*s & 0x80 || *s < 0x1f || *s == 0x7f ||
				*s == ' ' || *s == '{' || *s == '(' || *s == ')' ||
				*s == '\"' || *s == '%' || *s == '*' || *s == '\\')
			return 0;
	}

	if (len >= 998)
		return 0;

	return 1;
}

int client_init(void)
{
	bzero(&g_client, sizeof(g_client));

	g_client.bufin = xyz_buf_create("client in", BUFSIZE_MAX);
	g_client.bufout = xyz_buf_create("client out", BUFSIZE_MAX);
	if(g_client.bufin == NULL || g_client.bufout == NULL) {
		return -1;
	}

	g_client.start = g_client.lastcmd = time(NULL);

	return 0;
}

void client_echoready(void)
{
	char *msg = "* OK IMAPrev1 Proxy ready\r\n";
	xyz_buf_append(g_client.bufout, msg, strlen(msg));
	smart_cliwrite(g_client.bufout, STDOUT_FILENO);

	return;
}

void client_destroy(void)
{
	if(g_client.bufin) {
		xyz_buf_destroy(g_client.bufin);
	}
	if(g_client.bufout) {
		xyz_buf_destroy(g_client.bufout);
	}
	bzero(&g_client, sizeof(g_client));

	return;
}

int client_check(void)
{
	if ((time(NULL)-g_client.lastcmd) > g_setting.timeout) {
		LOGE("client_check :: timeout closed.");
		xyz_event_stop(g_event);
	}

	if (g_client.errcmd > g_setting.errcmd) {
		LOGE("client_check :: to many error.");
		xyz_event_stop(g_event);
	}

	return 0;
}

int client_getline()
{
   char *d = xyz_buf_data(g_client.bufin);

   xyz_buf_stat(g_client.bufin);

   LOGI("data:%s", d);

   char *p = strchr(d, '\n');
   if(p == NULL) {
       LOGI("not find -n");
       return 0;
   }

   if( (p-d+1) > 1024) {
       xyz_buf_drop(g_client.bufin, p-d+1);
       LOGI("line too long, droped");
       return 0;
   } 

   xyz_buf_get(g_client.bufin, g_client.line, p-d+1);

   LOGI("data2:%s", d);
   LOGI("line length:%d", p-d+1);
   LOGI("parse line: %s", g_client.line);
   LOGI("next line: %s", xyz_buf_data(g_client.bufin));

   g_client.line[strlen(g_client.line)-1] = '\0';
   if(g_client.line[strlen(g_client.line)-1] == '\r') {
       g_client.line[strlen(g_client.line)-1] = '\0';
   }

   return strlen(g_client.line);
}

int client_gettagcmdarg()
{
    char *p,*b;
    p = g_client.line;

    while(*p && *p == ' ') p++;

    b=p;
    while(*p && *p != ' ') p++;
    strncpy(g_client.tag, b, p-b);

    while(*p && *p == ' ') p++;

    b=p;
    while(*p && *p != ' ') p++;
    strncpy(g_client.cmd, b, p-b);

    while(*p && *p == ' ') p++;

    b=p;
    strcpy(g_client.cmd, b);

    if(strlen(g_client.tag) == 0 || strlen(g_client.cmd) == 0) {
        return -1;
    }

    return 0; 
}

int client_read(int fd, void *arg)
{
	int n;

    LOGI("read client info.");

	n = xyz_buf_read(g_client.bufin, fd);

	if (n == -1) {
		LOGE("read client data error.");
		xyz_event_stop(g_event);
		return 0;
	} else if (n == 0) {
		LOGE("read client data zero.");
		return 0;
	}

	g_client.lastcmd = time(NULL);

    bzero(g_client.line, sizeof(g_client.line));
	bzero(g_client.cmd, sizeof(g_client.cmd));
	bzero(g_client.tag, sizeof(g_client.tag));
	bzero(g_client.args, sizeof(g_client.args));

    if(client_getline() == 0) {
        LOGI("getline no line");
        return 0;
    }

    LOGI("getline:%s", g_client.line);

    if(client_gettagcmdarg() == -1) {
		char *msg = "BAD command not support\r\n";

		xyz_buf_sprintf(g_client.bufout, "%s %s", g_client.tag, msg);
		smart_cliwrite(g_client.bufout, STDOUT_FILENO);

		g_client.errcmd++;

		LOGD("get a bad command line.");

		return 0;
	}

    LOGI("%s, %s, %s", g_client.tag, g_client.cmd, g_client.args);

	if (n == -1) {
		g_client.errcmd++;
	} else {
		g_client.errcmd = 0;
	}

	return 0;
}

int client_write(int fd, void *arg)
{
	int n;

	n = smart_cliwrite(g_client.bufout, fd);
	if (n == -1) {
		LOGE("write to client error.");
		xyz_event_stop(g_event);
		return -1;
	} else if (n == 0) {
		LOGE("write to client error2.");
		xyz_event_stop(g_event);
		return -1;
	}

	if (xyz_buf_length(g_client.bufout) == 0) {
		xyz_event_del(g_event, fd, XYZ_EVTYPE_WT);
	} else {
        xyz_event_add(g_event, fd, XYZ_EVTYPE_WT, client_write, NULL);
    }

	return n;
}

int smart_cliread(struct xyz_buf_t *buf, int fd)
{
	if(g_setting.usessl && g_client.ossl) {
		return xyz_buf_sslread(buf, g_client.ossl);
	} else {
		return xyz_buf_read(buf, fd);
	}
}

int smart_cliwrite(struct xyz_buf_t *buf, int fd)
{
	if(g_setting.usessl && g_client.ossl) {
		return xyz_buf_sslwrite(buf, g_client.ossl);
	} else {
		return xyz_buf_write(buf, fd);
	}
}

