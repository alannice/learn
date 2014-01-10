
#ifndef __CLIENT_H__
#define __CLIENT_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <time.h>
#include <strings.h>
#include <unistd.h>
#include <signal.h>

#include <xyz_sock.h>
#include <xyz_buf.h>
#include <xyz_event.h>
#include <xyz_log.h>
#include <xyz_ssl.h>

#include "setting.h"

////////////////////////////////////////////////

#define CLI_STRLEN 255                  // һ�㳤�ȶ���,���û���,�����.
#define BUFSIZE_MAX 8192                // �û����ݽ���,����BUFFER�ĳ���.
#define LINE_MAX   998                  // RFC�ж����IMAPָ��һ����󳤶�.

/// ����ͻ�����Ϣ.
struct client_t {
	char user[CLI_STRLEN+1];            // �����ʼ�.
	char passwd[CLI_STRLEN+1];          // �ʼ�����.
	struct xyz_buf_t *bufin;            // ���տͻ������ݲ����͵������.
	struct xyz_buf_t *bufout;           // ���շ�������ݲ����͵��ͻ���.
	char line[LINE_MAX+1];              // IMAPָ����.
	char tag[CLI_STRLEN+1];             // IMAPָ���TAG.
	char cmd[CLI_STRLEN+1];             // IMAPָ��.
	char args[LINE_MAX+1];              // IMAPָ��Ĳ���.
	int status;                         // �ͻ���״̬,δʹ��.
	int errcmd;                         // ��������Ĵ����ۼ�.
	time_t lastcmd;                     // �ͻ������һ��ָ��(����)��ʱ��.

    time_t start;                       // ����ʱ��.
	char cliaddr[CLI_STRLEN+1];         // �ͻ���IP��ַ.

	struct xyz_ssl_t *ossl;             // SSL��Ϣ.
};

struct client_t g_client;               // ����ͻ��������Ϣ.

////////////////////////////////////////////////

// client process
int our_auth(void);
int client_init(void);
int client_check(void);
int client_read(int fd, void *arg);
int client_write(int fd, void *arg);

void client_echoready(void);
void client_destroy(void);

int smart_cliread(struct xyz_buf_t *buf, int fd);
int smart_cliwrite(struct xyz_buf_t *buf, int fd);

////////////////////////////////////////////////

#endif // __CLIENT_H__
