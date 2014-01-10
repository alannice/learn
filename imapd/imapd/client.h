
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

#define CLI_STRLEN 255                  // 一般长度定义,如用户名,密码等.
#define BUFSIZE_MAX 8192                // 用户数据接收,发送BUFFER的长度.
#define LINE_MAX   998                  // RFC中定义的IMAP指令一长最大长度.

/// 保存客户端信息.
struct client_t {
	char user[CLI_STRLEN+1];            // 电子邮件.
	char passwd[CLI_STRLEN+1];          // 邮件密码.
	struct xyz_buf_t *bufin;            // 接收客户端数据并发送到服务端.
	struct xyz_buf_t *bufout;           // 接收服务端数据并发送到客户端.
	char line[LINE_MAX+1];              // IMAP指令行.
	char tag[CLI_STRLEN+1];             // IMAP指令的TAG.
	char cmd[CLI_STRLEN+1];             // IMAP指令.
	char args[LINE_MAX+1];              // IMAP指令的参数.
	int status;                         // 客户端状态,未使用.
	int errcmd;                         // 连续错误的次数累计.
	time_t lastcmd;                     // 客户端最后一条指令(数据)的时间.

    time_t start;                       // 启动时间.
	char cliaddr[CLI_STRLEN+1];         // 客户端IP地址.

	struct xyz_ssl_t *ossl;             // SSL信息.
};

struct client_t g_client;               // 保存客户端相关信息.

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
