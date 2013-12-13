
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

#define DEFAULT_CONF "imap_proxy.conf"

#define CLI_STRLEN 255
#define BUFSIZE_MAX 8192
#define LINE_MAX   1024

struct client_t {
	char user[CLI_STRLEN];
	char passwd[CLI_STRLEN];
	struct xyz_buf_t *bufin;
	struct xyz_buf_t *bufout;
	char tag[CLI_STRLEN];
	char cmd[CLI_STRLEN];
	char args[CLI_STRLEN];
	int status;
	int errcmd;
	time_t lastcmd;             
	time_t starttime;               

	char cliaddr[CLI_STRLEN];
	char servaddr[CLI_STRLEN];              
	int servport;
};  

struct setting_t{
	int timeout;
	int errcmd;
};


char g_confile[256] = {0};
struct client_t g_client;
struct setting_t g_setting;

struct xyz_event_t *g_event;
struct xyz_conf_t *g_config;

char g_line[LINE_MAX+1];
char *g_lnpos;


//////////////////////////////////////////////////////////////////////////////
// load config
//





//////////////////////////////////////////////////////////////////////////////
// client process
//

int client_init()
{
	g_client.bufin = xyz_buf_create("client in", BUFSIZE_MAX);
	g_client.bufout = xyz_buf_create("client out", BUFSIZE_MAX);

	g_client.lastcmd = g_client.starttime = time(NULL);

	xyz_buf_add(g_client.bufout, "* OK IMAP Proxy ready\r\n", 23);
	xyz_buf_write(g_client.bufout, STDOUT_FILENO);

	strcpy(g_client.cliaddr, "10.10.10.10");

	return 0;
}

int client_check(void)
{
	if((time(NULL)-g_client.lastcmd) > g_setting.timeout) {
		xyz_buf_add(g_client.bufout, "* OK IMAP Proxy timeout bye\r\n", 29);
		xyz_buf_write(g_client.bufout, STDOUT_FILENO);
		xyz_event_stop(g_event);
	}

	if(g_client.errcmd > g_setting.errcmd) {
		xyz_buf_add(g_client.bufout, "* OK IMAP command error to many bye\r\n", 29);                                     
		xyz_buf_write(g_client.bufout, STDOUT_FILENO);                                                           
		xyz_event_stop(g_event);    
	}

	return 0;
}


int cmd_login(void)
{
	printf("login :: tag:[%s], cmd:[%s], args:[%s]\n", g_client.tag, g_client.cmd, g_client.args);

	if(strlen(g_client.args) == 0) {
		printf("login :: args length is 0\n");
		return -1;
	}

	char *user = g_client.args;
	char *passwd = strchr(user, ' ');
	if(passwd == NULL) {
		printf("login :: args no space\n"); 
		return -1;
	}
	*passwd = '\0';
	if(strlen(user) == 0) {
		printf("login :: no user name\n");
		return -1;
	}
	passwd++;
	if(strlen(passwd) == 0) {
		printf("login :: passwd length is 0\n"); 
		return -1;
	}
	if(strchr(passwd, ' ') != NULL) {
		printf("login :: passwd has space\n");
		return -1;
	}

	bzero(g_client.user, sizeof(g_client.user));
	strncpy(g_client.user, user, sizeof(g_client.user)-1);
	bzero(g_client.passwd, sizeof(g_client.passwd));
	strncpy(g_client.passwd, passwd, sizeof(g_client.passwd)-1);

	printf("login :: user:[%s], passwd:[%s]\n", user, passwd);
	
	return 0;
}

int cmd_logout(void)
{

	printf("logout :: tag:[%s], cmd:[%s], args:[%s]\n", g_client.tag, g_client.cmd, g_client.args);

	if(strlen(g_client.args) > 0) {
		printf("logout :: has args error\n");
		return -1;
	}

	xyz_buf_add(g_client.bufout, "* OK IMAP Proxy logout bye\r\n", 28);
	xyz_buf_write(g_client.bufout, STDOUT_FILENO);
	xyz_event_stop(g_event);

	return 0;
}

int cmd_capa(void)
{
	printf("capa :: tag:[%s], cmd:[%s], args:[%s]\n", g_client.tag, g_client.cmd, g_client.args);

	if(strlen(g_client.args) > 0) {
		printf("capability :: has args error\n");
		return -1;
	}

	return 0;
}

int cmd_noop(void)
{
	printf("noop :: tag:[%s], cmd:[%s], args:[%s]\n", g_client.tag, g_client.cmd, g_client.args);

	if(strlen(g_client.args) > 0) {
		printf("noop :: has args error\n");
		return -1;
	}

	return 0;
}

int cmd_auth(void)
{
	printf("auth :: tag:[%s], cmd:[%s], args:[%s]\n", g_client.tag, g_client.cmd, g_client.args);

	if(strlen(g_client.args) > 0) {
		printf("auth :: has args error\n");
		return -1;
	}

	return 0;
}

int cmd_nosupport(void)
{
	printf("nosupport :: tag:[%s], cmd:[%s], args:[%s]\n", g_client.tag, g_client.cmd, g_client.args);

	return 0;
}

int client_read(int fd, void *arg)
{
	char *tag, *cmd, *args;
	int n;

	n = xyz_buf_read(g_client.bufin, fd);
	if(n == -1) {
		xyz_event_stop(g_event);
		return 0;
	} else if(n == 0) {
		return 0;
	}

	g_client.lastcmd = time(NULL);

	bzero(g_line, sizeof(g_line));
	n = xyz_buf_getline(g_client.bufin, g_line, LINE_MAX);
	if(n == 0) {
		printf("not full line get\n");
		return 0;
	}


	bzero(g_client.cmd, sizeof(g_client.cmd));
	bzero(g_client.tag, sizeof(g_client.tag));
	bzero(g_client.args, sizeof(g_client.args));

	// get tag.
	tag = g_line;
	cmd = strchr(tag, ' ');
	if(cmd == NULL) {
		// error
		printf("line no space\n");
		g_client.errcmd++;
		return 0;
	}
	*cmd = '\0';
	if(strlen(tag) == 0) {
		printf("tag length is 0\n");
		g_client.errcmd++;
		return 0;
	}
	strncpy(g_client.tag, tag, sizeof(g_client.tag)-1);

	// get cmd
	cmd++;
	if(strlen(cmd) == 0) {
		// error
		printf("command length is 0\n");
		g_client.errcmd++;
		return 0;
	}
	// get args
	args = strchr(cmd, ' ');
	if(args == NULL) {
		strncpy(g_client.cmd, cmd, sizeof(g_client.cmd)-1);
	} else {
		*args = '\0';
		if(strlen(cmd) == 0) {
			printf("command length is 0\n");
			g_client.errcmd++;
			return 0;
		}
		strncpy(g_client.cmd, cmd, sizeof(g_client.cmd)-1);

		// get arg
		args++;	
		if(strlen(args) > 0) {
			strncpy(g_client.args, args, sizeof(g_client.args)-1);
		} else {
			printf("the end is a space\n");
			return 0;
		}
	}


	if(strcasecmp(g_client.cmd, "login") == 0) {
		n = cmd_login();
	} else if(strcasecmp(g_client.cmd, "logout") == 0) {
		n = cmd_logout();
	} else if(strcasecmp(g_client.cmd, "capability") == 0) {
		n = cmd_capa();
	} else if(strcasecmp(g_client.cmd, "authenticate") == 0) {
		n = cmd_auth();
	} else if(strcasecmp(g_client.cmd, "noop") == 0) {
		n = cmd_noop();
	} else {
		// error
		n = cmd_nosupport();
	}

	if(n == -1) {
		g_client.errcmd++;
	} else {
		g_client.errcmd = 0;
	}

	return 0;
}


int client_write(int fd, void *arg)
{

	return 0;
}


//////////////////////////////////////////////////////////////////////////////
// server process
//


//////////////////////////////////////////////////////////////////////////////
// command process
//



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

	while((opt = getopt(argc, argv, "f:")) != -1) {
		switch(opt) {
			case 'f':
				strncpy(g_confile, optarg, sizeof(g_confile)-1);
				break;
			default:
				usage();
				exit(0);
				break;
		}
	}

	if(strlen(g_confile) == 0) {
		strncpy(g_confile, DEFAULT_CONF, sizeof(g_confile)-1);
	}

	return;
}

void init(void)
{
	g_config = NULL;
	g_event = NULL;

	return;
}

int main(int argc, char *argv[])
{
	xyz_log_open("imap-proxy" , LOG_MAIL, LOG_DEBUG);

	setopt(argc, argv);
	g_config = xyz_conf_load(g_confile);

	g_setting.timeout = 10;
	g_setting.errcmd = 5;
	client_init();

	g_event = xyz_event_create();
	if(g_event == NULL) {
		return 0;
	}

	xyz_event_call(g_event, client_check);
	xyz_event_add(g_event, STDIN_FILENO, EVTYPE_RD, client_read, NULL);
	xyz_event_loop(g_event);

	xyz_conf_destroy(g_config);
	xyz_event_destroy(g_event);

	LOGD("program exit");
	xyz_log_close();

	return 0;
}


