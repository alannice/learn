
#ifndef __SETTING_H__
#define __SETTING_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <time.h>
#include <strings.h>
#include <unistd.h>
#include <signal.h>

#include <xyz_conf.h>
#include <xyz_log.h>

/// 保存配置文件信息.
struct setting_t {
	char defdomain[256];       // 邮件默认域,用于邮件地址不完整时补齐.
	int timeout;               // 客户端无数据的超时时间, 秒.
	int errcmd;                // 客户端连接错误的上限.
	int loglevel;              // 日志级别, 1~7, 一般为6或7.
	int usessl;                // 是否是用SSL连接.
	char pemfile[256];         // SSL的PEM证书文件.
};

struct setting_t g_setting;

// setting
int setting_init(void);
int setting_load(char *file);

#endif // __SETTING_H__

