/**
 * imapproxy.c
 *
 * 
 *
**/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <time.h>
#include <strings.h>
#include <unistd.h>

#include <xyz_event.h>
#include <xyz_sock.h>
#include <xyz_buf.h>
#include <xyz_conf.h>
#include <xyz_log.h>

//////////////////////////////////////////////////////////////////////////////
// global
//

#define DEFAULT_CONF "imapproxy.conf"

#define CLI_STRLEN 255
#define BUFSIZE_MAX 8192
#define LINE_MAX   998  // rfc...

/// 
struct client_t {
    char user[CLI_STRLEN+1];		// user email
    char passwd[CLI_STRLEN+1];		// user password
    struct xyz_buf_t *bufin;		// recv user data and send to server.
    struct xyz_buf_t *bufout;		// recv server data and send to client.
    char tag[CLI_STRLEN+1];			// imap command tag
    char cmd[CLI_STRLEN+1];			// imap command
    char args[CLI_STRLEN+1];		// imap command args
    int status;						// client status unused
    int errcmd;						// error command in succession
    time_t lastcmd;					// last data received time
    time_t starttime;				// start time

    char cliaddr[CLI_STRLEN+1];		// client address

    int servfd;						// server socket fd
    int servstat;					// server state, 0:received server ready; 1:login sucessed
};

///
struct setting_t {
    char defdomain[CLI_STRLEN+1];	// default email domain
    char servaddr[CLI_STRLEN+1];	// server addr
    int servport;					// server port
    int timeout;					// client timeout
    int errcmd;						// max error command in succession
    int loglevel;					// log message level
};

char g_confile[256] = {0};			// config file
struct client_t g_client;			// client CTX
struct setting_t g_setting;			// setting 

struct xyz_event_t *g_event;		// socket event

char g_line[LINE_MAX+1];			// client imap command line

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


//////////////////////////////////////////////////////////////////////////////
// load config
//

int setting_init(void)
{
    memset(&g_setting, '\0', sizeof(struct setting_t));

	g_setting.timeout = 30;
	g_setting.loglevel = 6;
	g_setting.errcmd = 5;

    return 0;
}

int setting_load(void)
{
    struct xyz_conf_t *conf;

    conf = xyz_conf_load(g_confile);
    if (conf == NULL) {
        LOGE("config load error : %s", g_confile);
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

	xyz_conf_destroy(conf);

	if(strlen(g_setting.defdomain) == 0) {
		LOGE("not set default domain");
		return -1;
	}
	if(strlen(g_setting.servaddr) == 0) {
		LOGE("not set server addr");
		return -1;
	}
	if(g_setting.servport == 0) {
		LOGE("not set server port");
		return -1;
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

int cmd_login(void)
{
    char *user, *passwd;
    int flag = 0;

    LOGD("login :: tag:[%s], cmd:[%s], args:[%s]", g_client.tag, g_client.cmd, g_client.args);

    do {
        if (strlen(g_client.args) == 0) {
            LOGD("login :: args length is 0");
            break;
        }

        user = g_client.args;
        passwd = strchr(user, ' ');
        if (passwd == NULL) {
            LOGD("login :: args no space");
            break;
        }
        *passwd = '\0';
        if (strlen(user) == 0) {
            LOGD("login :: no user name");
            break;
        }
        passwd++;
        if (strlen(passwd) == 0) {
            LOGD("login :: passwd length is 0");
            break;
        }
        if (strchr(passwd, ' ') != NULL) {
            LOGD("login :: passwd has space");
            break;
        }

        flag = 1;
    } while (0);

    if (flag == 0) {
        char *msg1 = "BAD Request error\r\n";

        xyz_buf_sprintf(g_client.bufout, "%s %s", g_client.tag, msg1);
        xyz_buf_write(g_client.bufout, STDOUT_FILENO);

        return -1;
    }

    // auth ...
    int retval = our_auth();
    if (retval == -1) {
        char *msg1 = "NO LOGIN Login or password error\r\n";

        xyz_buf_sprintf(g_client.bufout, "%s %s", g_client.tag, msg1);
        xyz_buf_write(g_client.bufout, STDOUT_FILENO);

        return -1;
    }

    bzero(g_client.user, sizeof(g_client.user));
    strncpy(g_client.user, user, sizeof(g_client.user)-1);
    bzero(g_client.passwd, sizeof(g_client.passwd));
    strncpy(g_client.passwd, passwd, sizeof(g_client.passwd)-1);

    LOGD("login :: user:[%s], passwd:[%s]", user, passwd);

    retval = server_connect();
    if (retval == -1) {
        char *msg1 = "NO Login failed, Service unavailable\r\n";

        xyz_buf_sprintf(g_client.bufout, "%s %s", g_client.tag, msg1);
        xyz_buf_write(g_client.bufout, STDOUT_FILENO);

        return -1;
    }

	LOGD("connect server ok");

    return 0;
}

int cmd_logout(void)
{
    LOGD("logout :: tag:[%s], cmd:[%s], args:[%s]", g_client.tag, g_client.cmd, g_client.args);

    if (strlen(g_client.args) > 0) {
        LOGD("logout :: has args error");
        return -1;
    }

    char *msg1 = "* BYE IMAP4rev1 Proxy logging out\r\n";
    char *msg2 = "OK LOGOUT completed\r\n";

    xyz_buf_add(g_client.bufout, msg1, strlen(msg1));
    xyz_buf_sprintf(g_client.bufout, "%s %s", g_client.tag, msg2);
    xyz_buf_write(g_client.bufout, STDOUT_FILENO);
    xyz_event_stop(g_event);

    return 0;
}

int cmd_capa(void)
{
    LOGD("capa :: tag:[%s], cmd:[%s], args:[%s]", g_client.tag, g_client.cmd, g_client.args);

    /*
    if(strlen(g_client.args) > 0) {
    	printf("capability :: has args error\n");
    	return -1;
    }
    */

    char *msg1 = "* CAPABILITY IMAP4rev1 ID STARTTLS UIDPLUS\r\n";
    char *msg2 = "OK CAPABILITY completed\r\n";

    xyz_buf_add(g_client.bufout, msg1, strlen(msg1));
    xyz_buf_sprintf(g_client.bufout, "%s %s", g_client.tag, msg2);
    xyz_buf_write(g_client.bufout, STDOUT_FILENO);

    return 0;
}

int cmd_noop(void)
{
    LOGD("noop :: tag:[%s], cmd:[%s], args:[%s]", g_client.tag, g_client.cmd, g_client.args);

    /*
    if(strlen(g_client.args) > 0) {
    	printf("noop :: has args error\n");
    	return -1;
    }
    */

    char *msg = "OK NOOP completed\r\n";

    xyz_buf_sprintf(g_client.bufout, "%s %s", g_client.tag, msg);
    xyz_buf_write(g_client.bufout, STDOUT_FILENO);

    return 0;
}

int cmd_auth(void)
{
    LOGD("auth :: tag:[%s], cmd:[%s], args:[%s]", g_client.tag, g_client.cmd, g_client.args);

    /*
    if(strlen(g_client.args) > 0) {
    	printf("auth :: has args error\n");
    	return -1;
    }
    */

    char *msg = "BAD command not support\r\n";

    xyz_buf_sprintf(g_client.bufout, "%s %s", g_client.tag, msg);
    xyz_buf_write(g_client.bufout, STDOUT_FILENO);

    return 0;
}

int cmd_id(void)
{
    LOGD("id :: tag:[%s], cmd:[%s], args:[%s]", g_client.tag, g_client.cmd, g_client.args);

    if (strlen(g_client.args) > 0) {
        char *msg1 = "* ID (\"name\" \"sina imap server\" \"vendor\" \"sina\")\r\n";
        char *msg2 = "OK ID Completed\r\n";

        xyz_buf_add(g_client.bufout, msg1, strlen(msg1));
        xyz_buf_sprintf(g_client.bufout, "%s %s", g_client.tag, msg2);
        xyz_buf_write(g_client.bufout, STDOUT_FILENO);
    } else {
        char *msg = "BAD Invalid argument\r\n";

        xyz_buf_sprintf(g_client.bufout, "%s %s", g_client.tag, msg);
        xyz_buf_write(g_client.bufout, STDOUT_FILENO);
    }

    return 0;
}

int cmd_nosupport(void)
{
    LOGD("nosupport :: tag:[%s], cmd:[%s], args:[%s]", g_client.tag, g_client.cmd, g_client.args);

    char *msg = "BAD Please login first\r\n";

    xyz_buf_sprintf(g_client.bufout, "%s %s", g_client.tag, msg);
    xyz_buf_write(g_client.bufout, STDOUT_FILENO);

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

    g_client.lastcmd = g_client.starttime = time(NULL);

    char *msg = "* OK IMAPrev1 Proxy ready\r\n";
    xyz_buf_add(g_client.bufout, msg, strlen(msg));
    xyz_buf_write(g_client.bufout, STDOUT_FILENO);

    return 0;
}

int client_check(void)
{
    if ((time(NULL)-g_client.lastcmd) > g_setting.timeout) {
        LOGI("client_check :: timeout closed");
        xyz_event_stop(g_event);
    }

    if (g_client.errcmd > g_setting.errcmd) {
        LOGI("client_check :: to many error");
        xyz_event_stop(g_event);
    }

    return 0;
}


/// read from client to bufin. used when no login.
/// write to client  from bufout.
int client_read(int fd, void *arg)
{
    char *tag, *cmd, *args;
    int n, flag;

    n = xyz_buf_read(g_client.bufin, fd);
    if (n == -1) {
        LOGE("read client data error");
        xyz_event_stop(g_event);
        return 0;
    } else if (n == 0) {
        return 0;
    }

    g_client.lastcmd = time(NULL);

    bzero(g_line, sizeof(g_line));
    n = xyz_buf_getline(g_client.bufin, g_line, LINE_MAX);
    if (n == 0) {
        LOGD("not full line get");
        return 0;
    } else if (n == -2) {
        LOGD("line too lang drop it");
        int len = strchr(xyz_buf_data(g_client.bufin), '\n') - xyz_buf_data(g_client.bufin);
        xyz_buf_drop(g_client.bufin, len+1);
        return 0;
    } else if (n == -1) {
        LOGD("buf inter error");
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
            LOGD("line no space");
            strncpy(g_client.tag, tag, sizeof(g_client.tag)-1);
            break;
        }
        *cmd = '\0';
        if (strlen(tag) == 0) {
            LOGD("tag length is 0");
            strncpy(g_client.tag, "*", 1);
            break;
        }
        strncpy(g_client.tag, tag, sizeof(g_client.tag)-1);

        // get cmd
        cmd++;
        if (strlen(cmd) == 0) {
            // error
            LOGD("command length is 0");
            break;
        }
        // get args
        args = strchr(cmd, ' ');
        if (args == NULL) {
            strncpy(g_client.cmd, cmd, sizeof(g_client.cmd)-1);
        } else {
            *args = '\0';
            if (strlen(cmd) == 0) {
                LOGD("command length is 0");
                break;
            }
            strncpy(g_client.cmd, cmd, sizeof(g_client.cmd)-1);

            // get arg
            args++;
            if (strlen(args) > 0) {
                strncpy(g_client.args, args, sizeof(g_client.args)-1);
            } else {
                LOGD("the end is a space");
                break;
            }
        }


        flag = 1;
    } while (0);


    if (flag == 0) {
        char *msg = "BAD command not support\r\n";

        xyz_buf_sprintf(g_client.bufout, "%s %s", g_client.tag, msg);
        xyz_buf_write(g_client.bufout, STDOUT_FILENO);

        g_client.errcmd++;

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

/// write to client used bufout.
int client_write(int fd, void *arg)
{
    int n;

    n = xyz_buf_write(g_client.bufout, fd);
    if (n == -1) {
        LOGD("write to client error");
        xyz_event_stop(g_event);
		return -1;
    } else if(n == 0) {
        LOGD("write to client error2");
        xyz_event_stop(g_event);
		return -1;
	}


    if (xyz_buf_length(g_client.bufout) == 0) {
        xyz_event_del(g_event, fd, EVTYPE_WT);
    }

    return n;
}

/// read from client used bufin.
int client_trans(int fd, void *arg)
{
    int n;

    g_client.lastcmd = time(NULL);

    n = xyz_buf_read(g_client.bufin, fd);
    if (n == -1) {
        LOGI("read from client error");
        xyz_event_stop(g_event);
		return -1;
    } 

	LOGD("read from client : %s", xyz_buf_data(g_client.bufin));

    if (xyz_buf_length(g_client.bufin) > 0) {
		LOGD("add event for write to server");
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
        char *msg = "NO Login failed, Service unavailable\r\n";

        xyz_buf_sprintf(g_client.bufout, "%s %s", g_client.tag, msg);
        xyz_buf_write(g_client.bufout, STDOUT_FILENO);

		LOGD("connect to server Error %s:%d", g_setting.servaddr, g_setting.servport);

        return -1;
    }

	LOGD("connect to server OK %s:%d", g_setting.servaddr, g_setting.servport);

    // when connect to server, not used client_read to read client data.
    xyz_event_del(g_event, STDIN_FILENO, EVTYPE_RD);

    g_client.servstat = 0;

    xyz_sock_noblock(g_client.servfd);
    xyz_event_add(g_event, g_client.servfd, EVTYPE_RD, server_read, NULL);

    return 0;
}

/// read form server used bufout.
int server_read(int fd, void *arg)
{
    int n;

    n = xyz_buf_read(g_client.bufout, fd);
    if (n == -1) {
		LOGE("read from client error");
        xyz_event_stop(g_event);
        return 0;
    } else if (n == 0) {
        return 0;
    }

    bzero(g_line, sizeof(g_line));
    n = xyz_buf_getline(g_client.bufout, g_line, LINE_MAX);
    if (n == 0) {
        LOGD("not full line get");
        return 0;
    } else if (n == -2) {
        LOGD("line too lang drop it");
        int len = strchr(xyz_buf_data(g_client.bufout), '\n') - xyz_buf_data(g_client.bufout);
        xyz_buf_drop(g_client.bufout, len+1);
        return 0;
    } else if (n == -1) {
        LOGD("buf inter error");
        return 0;
    }

    // case 1
    if (g_client.servstat == 0) {
        if (strncasecmp("* OK ", g_line, 5) != 0) {
            LOGE("server respone error\n");
            return -1;
        }

        g_client.servstat = 1;

        xyz_buf_sprintf(g_client.bufout, "%s LOGIN %s %s\r\n", g_client.tag, g_client.user, g_client.passwd);
        xyz_buf_write(g_client.bufout, g_client.servfd);

        return 0;
    }

    // case 2
    if (g_client.servstat == 1) {
		char tmphead[1024];

        bzero(tmphead, sizeof(tmphead));
        snprintf(tmphead, sizeof(tmphead), "%s OK ", g_client.tag);

        if (strncasecmp(tmphead, g_line, strlen(tmphead)) != 0) {
            LOGE("server login respone error\n");
            return -1;
        }

        g_client.servstat = 2;

        xyz_buf_sprintf(g_client.bufout, "%s\r\n", g_line);
        xyz_buf_write(g_client.bufout, STDOUT_FILENO);
		
		/// when login server sucessed, use server_trand() read server data
		/// and used client_trans() read client date
        xyz_event_add(g_event, g_client.servfd, EVTYPE_RD, server_trans, NULL);
        xyz_event_add(g_event, STDIN_FILENO, EVTYPE_RD, client_trans, NULL);

        return 0;
    }

    return 0;
}

// write to server used bufin.
int server_write(int fd, void *arg)
{
    int n;

	LOGD("write to server");

    n = xyz_buf_write(g_client.bufin, fd);
    if (n == -1) {
        LOGD("client socket error");
        xyz_event_stop(g_event);
		return -1;
    } else if(n == 0) {
        LOGD("client socket error2");
        xyz_event_stop(g_event);
		return -1;
	}

    if (xyz_buf_length(g_client.bufin) == 0) {
		LOGD("write all data to server");
        xyz_event_del(g_event, fd, EVTYPE_WT);
    }

    return n;
}

// read from server used bufout.
int server_trans(int fd, void *arg)
{
    int n;

    n = xyz_buf_read(g_client.bufout, fd);
    if(n == -1) {
        LOGE("read from server error");
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

int main(int argc, char *argv[])
{
    xyz_log_open("imapproxy" , LOG_MAIL, LOG_DEBUG);
    LOGD("program start");

    setopt(argc, argv);

	setting_init();
	if(setting_load() == -1) {
		LOGE("config file set error");
		return 0;
	}
    xyz_log_open("imapproxy" , LOG_MAIL, g_setting.loglevel);

    client_init();

    g_event = xyz_event_create();
    if (g_event == NULL) {
        LOGE("create event error");
        return 0;
    }

    xyz_event_call(g_event, client_check);
    xyz_event_add(g_event, STDIN_FILENO, EVTYPE_RD, client_read, NULL);
    xyz_event_loop(g_event);

    xyz_event_destroy(g_event);

    LOGD("program exit");
    xyz_log_close();

    return 0;
}


