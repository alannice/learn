/**
 * imapproxy.c
 *
 * IMAP �ʼ���ȡ����������
 *
 **/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <time.h>
#include <strings.h>
#include <unistd.h>
#include <signal.h>

#include <xyz_event.h>
#include <xyz_sock.h>
#include <xyz_buf.h>
#include <xyz_conf.h>
#include <xyz_log.h>
#include <xyz_ssl.h>

//////////////////////////////////////////////////////////////////////////////
// global
//

#define DEFAULT_CONF "imapproxy.conf"       // Ĭ�������ļ�.

#define CLI_STRLEN 255                      // һ�㳤�ȶ���,���û���,�����.
#define BUFSIZE_MAX 8192                    // �û����ݽ���,����BUFFER�ĳ���.
#define LINE_MAX   998                      // RFC�ж����IMAPָ��һ����󳤶�.

/// ����ͻ�����Ϣ.
struct client_t {
	char user[CLI_STRLEN+1];            // �����ʼ�.
	char passwd[CLI_STRLEN+1];          // �ʼ�����.
	struct xyz_buf_t *bufin;            // ���տͻ������ݲ����͵������.
	struct xyz_buf_t *bufout;           // ���շ�������ݲ����͵��ͻ���.
	char tag[CLI_STRLEN+1];                     // IMAPָ���TAG.
	char cmd[CLI_STRLEN+1];                     // IMAPָ��.
	char args[CLI_STRLEN+1];            // IMAPָ��Ĳ���.
	int status;                                         // �ͻ���״̬,δʹ��.
	int errcmd;                                         // ��������Ĵ����ۼ�.
	time_t lastcmd;                                     // �ͻ������һ��ָ��(����)��ʱ��.

	char cliaddr[CLI_STRLEN+1];         // �ͻ���IP��ַ.
	char servaddr[CLI_STRLEN+1];        // ������IP��ַ.

	int servfd;                                         // ��������SOCKET����������.
	int servstat;                                       // �����������ӵ�״̬,0:��ʼ״̬;1:�յ�����OK��Ӧ;2:�յ���¼OK��Ӧ.

	struct xyz_ssl_t *ossl;          // SSL��Ϣ.
};

/// ���������ļ���Ϣ.
struct setting_t {
	char defdomain[CLI_STRLEN+1];       // �ʼ�Ĭ����,�����ʼ���ַ������ʱ����.
	char servaddr[CLI_STRLEN+1];        // IMAP������������IP��ַ.
	int servport;                                       // IMAP�������˿�.
	int timeout;                                        // �ͻ��������ݵĳ�ʱʱ��, ��.
	int errcmd;                                         // �ͻ������Ӵ��������.
	int loglevel;                                       // ��־����, 1~7, һ��Ϊ6��7.
	int usessl;                     // �Ƿ�����SSL����.
	char pemfile[CLI_STRLEN+1];     // SSL��PEM֤���ļ�.
};

struct client_t g_client;                       // ����ͻ��������Ϣ.
struct setting_t g_setting;                     // �����ļ���Ϣ.
struct xyz_event_t *g_event;            // �¼�����.
char g_confile[256] = {0};                      // ���ü���.
char g_line[LINE_MAX+1];                        // ��ʱ�����BUFFER���ȡһ������.
time_t g_starttime;                                 // ��������ʱ��.

///////////////////////////////////////////////

// setting
int setting_init(void);
int setting_load(void);

// client process
int our_auth(void);
int client_init(void);
int client_check(void);
int client_read(int fd, void *arg);
int client_write(int fd, void *arg);
int client_trans(int fd, void *arg);

// server process
int server_connect();
int server_read(int fd, void *arg);
int server_write(int fd, void *arg);
int server_trans(int fd, void *arg);

// command process
int cmd_login(void);
int cmd_logout(void);
int cmd_capa(void);
int cmd_noop(void);
int cmd_auth(void);
int cmd_id(void);
int cmd_nosupport(void);

// main
int smart_cliread(struct xyz_buf_t *buf, int fd);
int smart_cliwrite(struct xyz_buf_t *buf, int fd);

//////////////////////////////////////////////////////////////////////////////
// load config
//

int setting_init(void)
{
	memset(&g_setting, '\0', sizeof(struct setting_t));

	g_setting.timeout = 30;
	g_setting.loglevel = 6;
	g_setting.errcmd = 5;

	g_setting.usessl = 0;

	return 0;
}

int setting_load(void)
{
	struct xyz_conf_t *conf;

	conf = xyz_conf_load(g_confile);
	if (conf == NULL) {
		LOGE("config load error : %s.", g_confile);
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
	if (strlen(xyz_conf_string(conf, "servaddr")) > 0) {
		strncpy(g_setting.servaddr, xyz_conf_string(conf, "servaddr"), sizeof(g_setting.servaddr)-1);
	}
	if (xyz_conf_number(conf, "servport") > 0) {
		g_setting.servport = xyz_conf_number(conf, "servport");
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
	if(strlen(g_setting.servaddr) == 0) {
		LOGE("not set server addr.");
		return -1;
	}
	if(g_setting.servport == 0) {
		LOGE("not set server port.");
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

//////////////////////////////////////////////////////////////////////////////
// command process
//

int our_auth(void)
{

	return 0;
}

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

//////////////////////////////////////////////////////////////////////////////
// command process
//

int cmd_login(void)
{
	char *user, *passwd;
	int flag = 0;
	int retval;

	LOGD("login :: tag:[%s], cmd:[%s], args:[%s].", g_client.tag, g_client.cmd, g_client.args);

	do {
		if (strlen(g_client.args) == 0) {
			LOGD("login :: args length is 0.");
			break;
		}

		user = g_client.args;
		passwd = strchr(user, ' ');
		if (passwd == NULL) {
			LOGD("login :: args no space.");
			break;
		}
		*passwd = '\0';
		if (strlen(user) == 0) {
			LOGD("login :: no user name.");
			break;
		}
		passwd++;
		if (strlen(passwd) == 0) {
			LOGD("login :: passwd length is 0.");
			break;
		}
		if (strchr(passwd, ' ') != NULL) {
			LOGD("login :: passwd has space.");
			break;
		}

		flag = 1;
	} while (0);

	if (flag == 0) {
		char *msg1 = "BAD Request error\r\n";

		xyz_buf_sprintf(g_client.bufout, "%s %s", g_client.tag, msg1);
		smart_cliwrite(g_client.bufout, STDOUT_FILENO);

		return -1;
	}

	bzero(g_client.user, sizeof(g_client.user));
	strncpy(g_client.user, user, sizeof(g_client.user)-1);
	bzero(g_client.passwd, sizeof(g_client.passwd));
	strncpy(g_client.passwd, passwd, sizeof(g_client.passwd)-1);

	email_check();

	LOGI("login :: user:[%s], passwd:[%s].", g_client.user, g_client.passwd);

	/// ��֤.
	retval = our_auth();
	if( retval == -1) {
		char *msg1 = "NO LOGIN Login or password error\r\n";

		xyz_buf_sprintf(g_client.bufout, "%s %s", g_client.tag, msg1);
		smart_cliwrite(g_client.bufout, STDOUT_FILENO);

		LOGE("login :: auth failed.");

		return -1;
	}

	LOGI("login :: auth sucessed.");

	/// ���ӷ�����.
	retval = server_connect();
	if (retval == -1) {
		char *msg1 = "NO Login failed, Service unavailable\r\n";

		xyz_buf_sprintf(g_client.bufout, "%s %s", g_client.tag, msg1);
		smart_cliwrite(g_client.bufout, STDOUT_FILENO);

		LOGE("login:: connect server falied.");

		return -1;
	}

	LOGI("login:: connect server sucessed.");

	return 0;
}

int cmd_logout(void)
{
	LOGD("logout :: tag:[%s], cmd:[%s], args:[%s].", g_client.tag, g_client.cmd, g_client.args);

	/*
	   if (strlen(g_client.args) > 0) {
	   LOGD("logout :: has args error");
	   return -1;
	   }
	   */

	char *msg1 = "* BYE IMAP4rev1 Proxy logging out\r\n";
	char *msg2 = "OK LOGOUT completed\r\n";

	xyz_buf_add(g_client.bufout, msg1, strlen(msg1));
	xyz_buf_sprintf(g_client.bufout, "%s %s", g_client.tag, msg2);
	smart_cliwrite(g_client.bufout, STDOUT_FILENO);
	xyz_event_stop(g_event);

	return 0;
}

int cmd_capa(void)
{
	LOGD("capa :: tag:[%s], cmd:[%s], args:[%s].", g_client.tag, g_client.cmd, g_client.args);

	/*
	   if(strlen(g_client.args) > 0) {
	   printf("capability :: has args error\n");
	   return -1;
	   }
	   */

	char *msg1 = "* CAPABILITY IMAP4rev1 ID UIDPLUS\r\n";
	char *msg2 = "OK CAPABILITY completed\r\n";

	xyz_buf_add(g_client.bufout, msg1, strlen(msg1));
	xyz_buf_sprintf(g_client.bufout, "%s %s", g_client.tag, msg2);
	smart_cliwrite(g_client.bufout, STDOUT_FILENO);

	return 0;
}

int cmd_noop(void)
{
	LOGD("noop :: tag:[%s], cmd:[%s], args:[%s].", g_client.tag, g_client.cmd, g_client.args);

	/*
	   if(strlen(g_client.args) > 0) {
	   printf("noop :: has args error\n");
	   return -1;
	   }
	   */

	char *msg = "OK NOOP completed\r\n";

	xyz_buf_sprintf(g_client.bufout, "%s %s", g_client.tag, msg);
	smart_cliwrite(g_client.bufout, STDOUT_FILENO);

	return 0;
}

int cmd_auth(void)
{
	LOGD("auth :: tag:[%s], cmd:[%s], args:[%s].", g_client.tag, g_client.cmd, g_client.args);

	/*
	   if(strlen(g_client.args) > 0) {
	   printf("auth :: has args error\n");
	   return -1;
	   }
	   */

	char *msg = "BAD command not support\r\n";

	xyz_buf_sprintf(g_client.bufout, "%s %s", g_client.tag, msg);
	smart_cliwrite(g_client.bufout, STDOUT_FILENO);

	return 0;
}

int cmd_id(void)
{
	LOGD("id :: tag:[%s], cmd:[%s], args:[%s].", g_client.tag, g_client.cmd, g_client.args);

	if (strlen(g_client.args) > 0 && g_client.args[0] == '(' &&
			g_client.args[strlen(g_client.args)-1] == ')') {
		char *msg1 = "* ID (\"name\" \"sina imap server\" \"vendor\" \"sina\")\r\n";
		char *msg2 = "OK ID Completed\r\n";

		xyz_buf_add(g_client.bufout, msg1, strlen(msg1));
		xyz_buf_sprintf(g_client.bufout, "%s %s", g_client.tag, msg2);
		smart_cliwrite(g_client.bufout, STDOUT_FILENO);
	} else {
		char *msg = "BAD Invalid argument\r\n";

		xyz_buf_sprintf(g_client.bufout, "%s %s", g_client.tag, msg);
		smart_cliwrite(g_client.bufout, STDOUT_FILENO);
	}

	return 0;
}

int cmd_nosupport(void)
{
	LOGD("nosupport :: tag:[%s], cmd:[%s], args:[%s].", g_client.tag, g_client.cmd, g_client.args);

	char *msg = "BAD Please login first\r\n";

	xyz_buf_sprintf(g_client.bufout, "%s %s", g_client.tag, msg);
	smart_cliwrite(g_client.bufout, STDOUT_FILENO);

	return -1;
}

//////////////////////////////////////////////////////////////////////////////
// client process
//

int client_init(void)
{
	bzero(&g_client, sizeof(g_client));

	g_client.bufin = xyz_buf_create("client in", BUFSIZE_MAX);
	g_client.bufout = xyz_buf_create("client out", BUFSIZE_MAX);
	if(g_client.bufin == NULL || g_client.bufout == NULL) {
		return -1;
	}

	g_client.lastcmd = g_starttime = time(NULL);

	return 0;
}
void client_echoready(void)
{
	char *msg = "* OK IMAPrev1 Proxy ready\r\n";
	xyz_buf_add(g_client.bufout, msg, strlen(msg));
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
	if(g_client.servfd > 0) {
		close(g_client.servfd);
	}
	bzero(&g_client, sizeof(g_client));

	return;
}

/// ��鳬ʱ�ʹ����������.
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


/// ��ȡ�ͻ�����Ϣ�ŵ�bufin��,�������û�����ָ��.
/// ����ŵ�bufout��,������ͻ���.
/// �ڿͻ��˳ɹ����ӷ�����ǰʹ��.
int client_read(int fd, void *arg)
{
	char *tag, *cmd, *args;
	int n, flag;

	n = smart_cliread(g_client.bufin, fd);
	if (n == -1) {
		LOGE("read client data error.");
		xyz_event_stop(g_event);
		return 0;
	} else if (n == 0) {
		return 0;
	}

	g_client.lastcmd = time(NULL);

	bzero(g_line, sizeof(g_line));
	n = xyz_buf_getline(g_client.bufin, g_line, LINE_MAX);
	if (n == 0) {
		LOGD("not full line get.");
		return 0;
	} else if (n == -2) {
		LOGD("line too lang drop it.");
		int len = strchr(xyz_buf_data(g_client.bufin), '\n') - xyz_buf_data(g_client.bufin);
		xyz_buf_drop(g_client.bufin, len+1);
		return 0;
	} else if (n == -1) {
		LOGD("buf inter error.");
		return 0;
	}

	bzero(g_client.cmd, sizeof(g_client.cmd));
	bzero(g_client.tag, sizeof(g_client.tag));
	bzero(g_client.args, sizeof(g_client.args));

	flag = 0;
	do {
		// get tag.
		tag = g_line;
		cmd = strchr(tag, ' ');
		if (cmd == NULL) {
			// error
			LOGD("line no space.");
			strncpy(g_client.tag, tag, sizeof(g_client.tag)-1);
			break;
		}
		*cmd = '\0';
		if (strlen(tag) == 0) {
			LOGD("tag length is 0.");
			strncpy(g_client.tag, "*", 1);
			break;
		}
		if( !imparse_isatom(tag) ) {
			LOGD("tag is not a atom string.");
			strncpy(g_client.tag, "*", 1);
			break;
		}
		strncpy(g_client.tag, tag, sizeof(g_client.tag)-1);

		// get cmd
		cmd++;
		if (strlen(cmd) == 0) {
			// error
			LOGD("command length is 0.");
			break;
		}
		// get args
		args = strchr(cmd, ' ');
		if (args == NULL) {
			strncpy(g_client.cmd, cmd, sizeof(g_client.cmd)-1);
		} else {
			*args = '\0';
			if (strlen(cmd) == 0) {
				LOGD("command length is 0.");
				break;
			}
			strncpy(g_client.cmd, cmd, sizeof(g_client.cmd)-1);

			// get arg
			args++;
			if (strlen(args) > 0) {
				strncpy(g_client.args, args, sizeof(g_client.args)-1);
			} else {
				LOGD("the end is a space.");
				break;
			}
		}

		flag = 1;
	} while (0);

	if (flag == 0) {
		char *msg = "BAD command not support\r\n";

		xyz_buf_sprintf(g_client.bufout, "%s %s", g_client.tag, msg);
		smart_cliwrite(g_client.bufout, STDOUT_FILENO);

		g_client.errcmd++;

		LOGD("get a bad command line.");

		return 0;
	}

	if (strcasecmp(g_client.cmd, "login") == 0) {
		n = cmd_login();
	} else if (strcasecmp(g_client.cmd, "logout") == 0) {
		n = cmd_logout();
	} else if (strcasecmp(g_client.cmd, "capability") == 0) {
		n = cmd_capa();
	} else if (strcasecmp(g_client.cmd, "authenticate") == 0) {
		n = cmd_auth();
	} else if (strcasecmp(g_client.cmd, "id") == 0) {
		n = cmd_id();
	} else if (strcasecmp(g_client.cmd, "noop") == 0) {
		n = cmd_noop();
	} else {
		// error
		n = cmd_nosupport();
	}

	if (n == -1) {
		g_client.errcmd++;
	} else {
		g_client.errcmd = 0;
	}

	return 0;
}

/// ��bufout������������ͻ���.
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
		xyz_event_del(g_event, fd, EVTYPE_WT);
	}

	return n;
}

/// ���ӳɹ���������,��ȡ�ͻ�������,���ŵ�bufin��.
/// ������server_write�����������.
int client_trans(int fd, void *arg)
{
	int n;

	g_client.lastcmd = time(NULL);

	n = smart_cliread(g_client.bufin, fd);
	if (n == -1) {
		LOGE("read from client error.");
		xyz_event_stop(g_event);
		return -1;
	}

	LOGD("read from client : %s.", xyz_buf_data(g_client.bufin));

	if (xyz_buf_length(g_client.bufin) > 0) {
		xyz_event_add(g_event, g_client.servfd, EVTYPE_WT, server_write, NULL);
	}

	return n;
}

//////////////////////////////////////////////////////////////////////////////
// server process
//

int server_connect()
{
	int retval;

	g_client.servfd = xyz_sock_connect(g_setting.servaddr, g_setting.servport);
	if (g_client.servfd == -1) {
		LOGE("connect to server Error %s:%d.", g_setting.servaddr, g_setting.servport);
		return -1;
	}

	retval = xyz_sock_peeraddr(g_client.servfd, g_client.servaddr, sizeof(g_client.servaddr)-1);
	if(retval == 0) {
		LOGI("connected to server : %s(%s):%d.", g_setting.servaddr, g_client.servaddr, g_setting.servport);
	}

	/// ���ӵ���������,�Ͳ���ʹ��client_read��ȡ�ͻ�������.
	xyz_event_del(g_event, STDIN_FILENO, EVTYPE_RD);

	g_client.servstat = 0;

	xyz_sock_noblock(g_client.servfd);
	xyz_event_add(g_event, g_client.servfd, EVTYPE_RD, server_read, NULL);

	return 0;
}

/// ��ȡ���������,������ָ��.ֱ���յ�LOGIN�ɹ���Ϣ.
int server_read(int fd, void *arg)
{
	int n;

	n = xyz_buf_read(g_client.bufout, fd);
	if (n == -1) {
		char *msg = "NO Login failed, Service unavailable\r\n";

		xyz_buf_sprintf(g_client.bufout, "%s %s", g_client.tag, msg);
		smart_cliwrite(g_client.bufout, STDOUT_FILENO);
		xyz_event_stop(g_event);

		LOGE("read from server error.");
		return 0;
	} else if (n == 0) {
		return 0;
	}

	bzero(g_line, sizeof(g_line));
	n = xyz_buf_getline(g_client.bufout, g_line, LINE_MAX);
	if (n == 0) {
		LOGD("not full line get.");
		return 0;
	} else if (n == -2) {
		LOGD("line too lang drop it.");
		int len = strchr(xyz_buf_data(g_client.bufout), '\n') - xyz_buf_data(g_client.bufout);
		xyz_buf_drop(g_client.bufout, len+1);
		return 0;
	} else if (n == -1) {
		LOGD("buf inter error.");
		return 0;
	}

	// case 1 server ready.
	if (g_client.servstat == 0) {
		if (strncasecmp("* OK ", g_line, 5) != 0) {
			char *msg = "NO Login failed, Service unavailable\r\n";

			xyz_buf_sprintf(g_client.bufout, "%s %s", g_client.tag, msg);
			smart_cliwrite(g_client.bufout, STDOUT_FILENO);
			xyz_event_stop(g_event);

			LOGE("server connect respone error.");
			return -1;
		}

		g_client.servstat = 1;

		xyz_buf_sprintf(g_client.bufout, "%s LOGIN %s %s\r\n", g_client.tag, g_client.user, g_client.passwd);
		xyz_buf_write(g_client.bufout, g_client.servfd);

		return 0;
	}

	// case 2 login ok.
	if (g_client.servstat == 1) {
		char tmphead[1024];

		bzero(tmphead, sizeof(tmphead));
		snprintf(tmphead, sizeof(tmphead), "%s OK ", g_client.tag);

		if (strncasecmp(tmphead, g_line, strlen(tmphead)) != 0) {
			char *msg = "NO Login failed, Service unavailable\r\n";

			xyz_buf_sprintf(g_client.bufout, "%s %s", g_client.tag, msg);
			smart_cliwrite(g_client.bufout, STDOUT_FILENO);
			xyz_event_stop(g_event);

			LOGE("server login respone error.");
			return -1;
		}

		g_client.servstat = 2;

		xyz_buf_sprintf(g_client.bufout, "%s\r\n", g_line);
		smart_cliwrite(g_client.bufout, STDOUT_FILENO);
		LOGI("login server sucessed.");

		/// �յ���������LOGIN�ɹ���Ϣ��, ͨ��server_trans��ȡ������������.
		/// ͨ��client_trans��ȡ�ͻ�������, ������ֻ������,���ٽ���.
		xyz_event_add(g_event, g_client.servfd, EVTYPE_RD, server_trans, NULL);
		xyz_event_add(g_event, STDIN_FILENO, EVTYPE_RD, client_trans, NULL);

		return 0;
	}

	return 0;
}

/// ��bufin�е�����д����������.
int server_write(int fd, void *arg)
{
	int n;

	LOGD("write to server.");

	n = xyz_buf_write(g_client.bufin, fd);
	if (n == -1) {
		LOGE("client socket error.");
		xyz_event_stop(g_event);
		return -1;
	} else if (n == 0) {
		LOGE("client socket error2.");
		xyz_event_stop(g_event);
		return -1;
	}

	if (xyz_buf_length(g_client.bufin) == 0) {
		xyz_event_del(g_event, fd, EVTYPE_WT);
	}

	return n;
}

/// ��ȡ������������,�ŵ�bufout��.
/// ������client_write������ͻ���.
int server_trans(int fd, void *arg)
{
	int n;

	n = xyz_buf_read(g_client.bufout, fd);
	if (n == -1) {
		LOGE("read from server error.");
		xyz_event_stop(g_event);
		return -1;
	}

	// LOGD("read from server : %s", xyz_buf_data(g_client.bufout));

	if (xyz_buf_length(g_client.bufout) > 0) {
		xyz_event_add(g_event, STDOUT_FILENO, EVTYPE_WT, client_write, NULL);
	}

	return n;
}

//////////////////////////////////////////////////////////////////////////////
// main
//

void usage()
{
	printf("Usage:\n");
	printf("\t-f <file> : config file\n");

	return;
}

void setopt(int argc, char *argv[])
{
	int opt;

	while ((opt = getopt(argc, argv, "f:")) != -1) {
		switch (opt) {
			case 'f':
				strncpy(g_confile, optarg, sizeof(g_confile)-1);
				break;
			default:
				usage();
				exit(0);
				break;
		}
	}

	if (strlen(g_confile) == 0) {
		strncpy(g_confile, DEFAULT_CONF, sizeof(g_confile)-1);
	}

	return;
}

void init(void)
{
	g_event = NULL;

	return;
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

void sig_alarm(int sig)
{
	LOGE("SSL accept error, exit.");
	exit(0);
}

int main(int argc, char *argv[])
{
	xyz_log_open("imapproxy" , LOG_MAIL, LOG_DEBUG);
	LOGI("program start.");

	setopt(argc, argv);

	setting_init();
	if (setting_load() == -1) {
		LOGE("config file set error, exit.");
		return 1;
	}

	xyz_log_open("imapproxy" , LOG_MAIL, g_setting.loglevel);

	signal(SIGPIPE, SIG_IGN);

	client_init();

	if(g_setting.usessl) {
		LOGI("use ssl.");
		g_client.ossl = xyz_ssl_create(XYZ_SSLv23, g_setting.pemfile);
		if(g_client.ossl == NULL) {
			LOGE("SSL init error, exit.");
			return 2;
		}
		signal(SIGALRM, sig_alarm);
		alarm(3);
		int n = xyz_ssl_accept(g_client.ossl, STDIN_FILENO, STDOUT_FILENO);
		if(n == -1) {
			xyz_ssl_destroy(g_client.ossl);
			LOGE("SSL accept error, exit.");
			return 3;
		}
		alarm(0);
		LOGI("ssl accept sucessed.");
	}


	int n = xyz_sock_peeraddr(STDIN_FILENO, g_client.cliaddr, sizeof(g_client.cliaddr)-1);
	if(n == 0) {
		LOGI("client ip : %s.", g_client.cliaddr);
	}

	client_echoready();

	g_event = xyz_event_create();
	if (g_event == NULL) {
		LOGE("create event error, exit.");
		xyz_ssl_destroy(g_client.ossl);
		return 4;
	}

	xyz_event_call(g_event, client_check);
	xyz_event_add(g_event, STDIN_FILENO, EVTYPE_RD, client_read, NULL);
	xyz_event_loop(g_event);
	xyz_event_destroy(g_event);

	xyz_ssl_destroy(g_client.ossl);
	client_destroy();

	LOGE("program exit, run %d second.", time(NULL)-g_starttime);
	xyz_log_close();

	return 0;
}

