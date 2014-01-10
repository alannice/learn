/**
 * imapd.c
 *
 * IMAP 邮件收取程序服务
 *
 **/

#include "imapd.h"
#include "setting.h"
#include "client.h"

////////////////////////////////////////////////

#define  DEFAULT_CONF  "imapd.conf"

struct xyz_event_t *g_event;        // 事件处理.
char g_confile[256];                // 配置文件 

///////////////////////////////////////////////

void usage()
{
	printf("Usage:\n");
	printf("\t-f <file> : config file\n");
    printf("compile %s %s\n", __DATE__, __TIME__);

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

void sig_alarm(int sig)
{
	LOGE("SSL accept error, exit.");
	exit(0);
}

int main(int argc, char *argv[])
{
	xyz_log_open("imapd" , LOG_MAIL, LOG_DEBUG);
	LOGI("program start.");

	setopt(argc, argv);

	setting_init();
	if (setting_load(g_confile) == -1) {
		LOGE("config file set error, exit.");
		return 1;
	}

	xyz_log_open("imapproxy" , LOG_MAIL, g_setting.loglevel);

	signal(SIGPIPE, SIG_IGN);

	client_init();

	int n = xyz_sock_peeraddr(STDIN_FILENO, g_client.cliaddr, sizeof(g_client.cliaddr)-1);
	if(n == 0) {
		LOGI("client ip : %s.", g_client.cliaddr);
	}

	client_echoready();

	g_event = xyz_event_create();
	if (g_event == NULL) {
		LOGE("create event error, exit.");
		return 4;
	}

    xyz_event_call(g_event, client_check);
	xyz_event_add(g_event, STDIN_FILENO, XYZ_EVTYPE_RD, client_read, NULL);
	xyz_event_loop(g_event);
	xyz_event_destroy(g_event);

	client_destroy();

	xyz_log_close();

	return 0;
}

