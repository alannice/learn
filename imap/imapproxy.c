
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "xyz_event.h"
#include "xyz_sock.h"
#include "xyz_buf.h"
#include "xyz_log.h"

//////////////////////////////////////////////////////////////////////////////
// global 
//

#define DEFAULT_CONF "imap_proxy.conf"

#define CLI_STRLEN 255

#define LINE_MAX   1024

struct client_t {
	char user[CLI_STRLEN];
	char pass[CLI_STRLEN];
	struct buf_t *bufin;
	struct buf_t *bufout;
	char cmd[CLI_STRLEN];
	char tag[CLI_STRLEN];
	int status;
	time_t lastcmd;             
	time_t starttime;               

	char cliaddr[CLI_STRLEN];
	char servaddr[CLI_STRLEN];              
	int servport;
};  


char g_confile[256] = {0};

struct xyz_event_t *g_event;
struct xyz_conf_t *g_conf;

char g_line[MAX_LINE+1];
char *g_linepos;


//////////////////////////////////////////////////////////////////////////////
// load config
//





//////////////////////////////////////////////////////////////////////////////
// client process
//

int client_timeout()
{
	if(difftime(time(NULL), g_client.lastcmd) > g_setting.timeout) {
		buf_add(g_client.bufout, "* OK IMAP Proxy bye\r\n", 21);
		buf_write(g_client.bufout, STDOUT_FILENO);
		event_stop(g_event);
	}

	return 0;
}

int cmd_login()
{

}

int cmd_logout()
{

}

int cmd_capa()
{

}

int cmd_noop()
{

}

int cmd_auth()
{

}

int cmd_nosupport()
{

}

int client_getline(int fd)
{
	int n=0;
	char c;
	
	while(1) {
		if(strlen(g_line) >= sizeof(g_line) -1) {
			return -1;
		}
		n = read(fd, &c, 1, 0);
		if(n==0) {
			return -1;
		} else if (n<0 && (errno == EINTR || errno == EAGAIN)) {
			break;
		} else if(n==1) {
			if(c=='\n') {
				return 1;
			} else if(c=='\r') {
				continue;
			}
			g_line[g_linepos] = c;
			g_linepos++;
		}
	}

	return 0;
}

int client_read(int fd, void *arg)
{
	int n;

	n = client_getline(fd);
	if(n == -1) {
		xyz_event_stop(g_event);
		return 0;
	} else if(n == 0) {
		return 0;
	}

	g_linepos = g_line;
	char *p = strchr(g_linepos, ' ');
	if(p == NULL) {
		// error
	}
	*p = '\0';
	strcpy(g_client.tag, g_linepos);

	g_linepos = p+1;
	p = strchr(g_linepos, ' ');
	if(p == NULL) {
		strcpy(g_client.cmd, g_linepos);
	} else {
		*p = '\0';
		strcpy(g_client.cmd, g_linepos);
		g_linepos = p+1;	
		strcpy(g_client.arg, g_linepos);
	}
	
	if(strlen(g_client.tag) == 0 || strlen(g_client.cmd) == 0) {
		return 0;
	}

	if(strcasecmp(g_client.cmd, "login") = 0) {
		cmd_login(g_client);
	} else if(strcasecmp(g_client.cmd, "logout") == 0) {
		cmd_logout(g_client);
	} else if(strcasecmp(g_client.cmd, "capability") == 0) {
		cmd_capa(g_client);
	} else if(strcasecmp(g_client.cmd, "authenticate") == 0) {
		cmd_auth(g_client);
	} else if(strcasecmp(g_client.cmd, "noop") == 0) {
		cmd_noop(g_client);
	} else {
		// error
		cmd_nosuppert(g_client);
	}

	return 0;
}


int client_write(int fd, void *arg)
{

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

	if(strlen(g_config) == 0) {
		strncpy(g_confile, DEFAULT_CONF, sizeof(g_confile)-1);
	}

	return;
}

void init()
{
	memset(g_config, '0', sizeof(g_config));
	g_event = NULL;

	return;
}

int main(int argc, char *argv[])
{
	xyz_log_open("imap-proxy" , LOG_USER, LOG_DEBUG);

	setopt(argc, argv);

	xyz_conf_load(g_confile);

	client_init();

	g_event = xyz_event_create();
	if(g_event == NULL) {
		return 0;
	}

	event_call(g_event, client_timeout);
	event_add(g_event, STDIN_FILENO, EVTYPE_RD, client_input, &g_client);
	event_loop(g_event);

	LOGD("program exit");

	return 0;
}


