
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

#define LINE_MAX   1024

struct client_t {
	char user[CLI_STRLEN];
	char pass[CLI_STRLEN];
	struct xyz_buf_t *bufin;
	struct xyz_buf_t *bufout;
	char cmd[CLI_STRLEN];
	char tag[CLI_STRLEN];
	char arg[CLI_STRLEN];
	int status;
	time_t lastcmd;             
	time_t starttime;               

	char cliaddr[CLI_STRLEN];
	char servaddr[CLI_STRLEN];              
	int servport;
};  

struct setting_t{
	int timeout;
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

int client_timeout(void)
{
	if((time(NULL)-g_client.lastcmd) > g_setting.timeout) {
		xyz_buf_add(g_client.bufout, "* OK IMAP Proxy bye\r\n", 21);
		xyz_buf_write(g_client.bufout, STDOUT_FILENO);
		xyz_event_stop(g_event);
	}

	return 0;
}


int cmd_login(void)
{

	return 0;
}

int cmd_logout(void)
{

	return 0;
}

int cmd_capa(void)
{

	return 0;
}

int cmd_noop(void)
{

	return 0;
}

int cmd_auth(void)
{

	return 0;
}

int cmd_nosupport(void)
{

	return 0;
}

int client_read(int fd, void *arg)
{
	int n;

	n = xyz_buf_read(g_client.bufin, fd);
	if(n == -1) {
		xyz_event_stop(g_event);
		return 0;
	} else if(n == 0) {
		return 0;
	}

	bzero(g_line, sizeof(g_line));                                                                                                   
	bzero(g_client.cmd, sizeof(g_client.cmd));
	bzero(g_client.tag, sizeof(g_client.tag));

	n = xyz_buf_getline(g_client.bufin, g_line, LINE_MAX);
	if(n == 0) {
		return 0;
	}

	// get tag.
	g_lnpos = g_line;
	char *p = strchr(g_lnpos, ' ');
	if(p == NULL) {
		// error
		return 0;
	}
	*p = '\0';
	if(strlen(g_lnpos) == 0) {
		// error
		return 0;
	}
	strncpy(g_client.tag, g_lnpos, sizeof(g_client.tag)-1);

	// get cmd
	g_lnpos = p+1;
	p = strchr(g_lnpos, ' ');
	if(p == NULL) {
		strncpy(g_client.cmd, g_lnpos, sizeof(g_client.cmd)-1);
	} else {
		*p = '\0';
		if(strlen(g_lnpos) == 0) {
			return 0;
		}
		strncpy(g_client.cmd, g_lnpos, sizeof(g_client.cmd)-1);

		// get arg
		g_lnpos = p+1;	
		if(strlen(g_lnpos) > 0) {
			strncpy(g_client.arg, g_lnpos, sizeof(g_client.arg)-1);
		}
	}

	if(strlen(g_client.tag) == 0 || strlen(g_client.cmd) == 0) {
		return 0;
	}

	if(strcasecmp(g_client.cmd, "login") == 0) {
		cmd_login();
	} else if(strcasecmp(g_client.cmd, "logout") == 0) {
		cmd_logout();
	} else if(strcasecmp(g_client.cmd, "capability") == 0) {
		cmd_capa();
	} else if(strcasecmp(g_client.cmd, "authenticate") == 0) {
		cmd_auth();
	} else if(strcasecmp(g_client.cmd, "noop") == 0) {
		cmd_noop();
	} else {
		// error
		cmd_nosupport();
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

	//client_init();

	g_event = xyz_event_create();
	if(g_event == NULL) {
		return 0;
	}

	xyz_event_call(g_event, client_timeout);
	xyz_event_add(g_event, STDIN_FILENO, EVTYPE_RD, client_read, NULL);
	xyz_event_loop(g_event);

	xyz_conf_destroy(g_config);
	xyz_event_destroy(g_event);

	LOGD("program exit");
	xyz_log_close();

	return 0;
}


