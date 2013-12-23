
// gcc mail_test.c -o mail_test -lssl

/************关于本文档********************************************
 *  * *filename: ssl-client.c
 *   * *purpose: 演示利用OpenSSL 库进行基于IP 层的SSL 加密通讯的方法，这是客户端例子
 *    * *wrote by: zhoulifa(zhoulifa@163.com) 周立发(http://zhoulifa.bokee.com)
 *     * Linux 爱好者Linux 知识传播者SOHO 族开发者最擅长C 语言
 *      * *date time:2007-02-02 20:10
 *       * *Note: 任何人可以任意复制代码并运用这些文档，当然包括你的商业用途
 *        * * 但请遵循GPL
 *         * *Thanks to:Google
 *          * *Hope:希望越来越多的人贡献自己的力量，为科学技术发展出力
 *           * * 科技站在巨人的肩膀上进步更快！感谢有开源前辈的贡献！
 *            * *********************************************************************/

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/select.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <sys/wait.h>
#include <unistd.h>
#include <signal.h>
#include <syslog.h>
#include <openssl/ssl.h>
#include <openssl/err.h>

#define MAXBUF 1024
int g_isdaemon;

struct cmd_t {
    char *label;
    char *cmd;
};

struct cmd_t imap_cmds[] =
{
    {"login", "id1 login zhangcheng_todo@sina.com yuanzc\r\n\0"},
    {"list", "id2 list \"\" *\r\n\0"},
    {"select", "id3 select INBOX\r\n\0"},
    {"fetch1", "id4 fetch 1:3 rfc822\r\n\0"},
    {"fetch2", "id5 fetch 3:6 rfc822\r\n\0"},
    {"fetch3", "id6 fetch 6:9 rfc822\r\n\0"},
    {"fetch4", "id7 fetch 9:12 rfc822\r\n\0"},
    {"logout", "id8 logout\r\n\0"},
    {NULL, NULL},
};

struct cmd_t pop_cmds[] =
{
    {"user", "user zhangcheng_todo@sina.com\r\n\0"},
    {"pass", "pass yuanzc\r\n\0"},
    {"list", "list\r\n\0"},
    {"uidl", "uidl\r\n\0"},
    {"stat", "stat\r\n\0"},
    {"noop", "noop\r\n\0"},
    {"retr1", "retr 1\r\nretr 2\r\nretr 3\r\n\0"},
    {"retr", "retr 4\r\nretr 5\r\nretr 6\r\n\0"},
    {"quit", "quit\r\n\0"},
    {NULL, NULL},
};

struct cmd_t smtp_cmds[] =
{
    {"helo", "helo www.sina.com\r\n\0"},
    {"from", "mail from:<zhangcheng_todo@sina.com>\r\n\0"},
    {"to", "rcpt to:<zhangcheng_todo@sina.cn>\r\n\0"},
    {"data", "data\r\n\0"},
    {"content", "From:<zhangcheng_todo@sina.com>\r\n"
        "To:<zhangcheng_todo@sina.cn>\r\n"
            "Subject: hello moto\r\n"
            "\r\n"
            "hello world hello world hello world\r\n"
            "hello world hello world hello world\r\n"
            "hello world hello world hello world\r\n"
            ".\r\n\0"},
    {"quit", "quit\r\n\0"},
    {NULL, NULL},
};

void ShowCerts(SSL * ssl)
{
    X509 *cert;
    char *line;
    cert = SSL_get_peer_certificate(ssl);
    if (cert != NULL) {
        line = X509_NAME_oneline(X509_get_subject_name(cert), 0, 0);
        //printf("subject_name: %s\n", line);
        free(line);
        line = X509_NAME_oneline(X509_get_issuer_name(cert), 0, 0);
        //printf("issuer_name: %s\n", line);
        free(line);
        X509_free(cert);
    } else {
        //printf("no info!\n");
    }
}

int test_read(int fd, SSL *ssl)
{
    fd_set rfds;
    struct timeval tv;
    int retval;

    char data[4096];
    int num;

    while(1) {
        /* Watch stdin (fd 0) to see when it has input. */
        FD_ZERO(&rfds);
        FD_SET(fd, &rfds);

        /* Wait up to two seconds. */
        tv.tv_sec = 1;
        tv.tv_usec = 0;

        retval = select(fd+1, &rfds, NULL, NULL, &tv);
        /* Don’t rely on the value of tv now! */
        if (retval == -1) {
            if(!g_isdaemon) {
                perror("select()");
            }
            return -1;
        } else if (retval) {
            //printf("Data is available now.\n");
            bzero(data, sizeof(data));
            if(ssl) {
                num = SSL_read(ssl, data, sizeof(data)-1);
            } else {
                num = read(fd, data, sizeof(data)-1);
            }

            if(num > 0) {
                if(!g_isdaemon) {
                    printf(data);
                    fflush(stdout);
                }
            } else {
                return num;
            }

            /* FD_ISSET(0, &rfds) will be true. */
        } else {
            //printf("No data within two seconds.\n");
            return 0;
        }
    }

    return 0;
}

void test_cmd(int fd, SSL *ssl, struct cmd_t *cmds)
{
    int num=0;

    num = test_read(fd, ssl);
    if(num == -1) {
        if(!g_isdaemon) {
            printf("recv ready error");
        }
        return;
    }

    int i=0;
    for(i=0; cmds[i].label; i++) {
        if(!g_isdaemon) {
            printf("\r\n%s", cmds[i].cmd);
        }
        if(ssl) {
            num = SSL_write(ssl, cmds[i].cmd, strlen(cmds[i].cmd));
        } else {
            num = write(fd, cmds[i].cmd, strlen(cmds[i].cmd));
        }
        if(num != strlen(cmds[i].cmd)) {
            if(!g_isdaemon) {
                printf("send [%s] len error: [%d]\n", cmds[i].label, num);
            }
        }
        num = test_read(fd, ssl);
        if(num == -1) {
            if(!g_isdaemon) {
                printf("recv [%s] error\n", cmds[i].label);
                fflush(stdout);
            }
            return;
        }
    }

    return;
}

int tcp_connect(char *ip, int port)
{
    int sockfd, len;
    struct sockaddr_in dest;

    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        if(!g_isdaemon) {
            perror("Socket");
        }
        exit(errno);
    }
    //printf("socket created\n");

    bzero(&dest, sizeof(dest));
    dest.sin_family = AF_INET;
    dest.sin_port = htons(port);
    if (inet_aton(ip, (struct in_addr *) &dest.sin_addr.s_addr) == 0) {
        if(!g_isdaemon) {
            perror(ip);
        }
        exit(errno);
    }
    //printf("address created\n");

    /* 连接服务器*/
    if (connect(sockfd, (struct sockaddr *) &dest, sizeof(dest)) != 0) {
        if(!g_isdaemon) {
            perror("Connect ");
        }
        exit(errno);
    }
    //printf("server connected\n");

    return sockfd;
}

SSL_CTX *g_ctx;
SSL *g_ssl;

int ssl_connect(int sockfd)
{
    SSL_library_init();
    OpenSSL_add_all_algorithms();
    SSL_load_error_strings();
    g_ctx = SSL_CTX_new(SSLv23_client_method());
    if (g_ctx == NULL) {
        if(!g_isdaemon) {
            ERR_print_errors_fp(stdout);
        }
        exit(1);
    }

    g_ssl = SSL_new(g_ctx);
    if(g_ssl == NULL) {
        if(!g_isdaemon) {
            ERR_print_errors_fp(stdout);
        }
        exit(1);
    }

    SSL_set_fd(g_ssl, sockfd);

    if (SSL_connect(g_ssl) == -1) {
        if(!g_isdaemon) {
            ERR_print_errors_fp(stderr);
        }
        exit(1);
    } else {
        //printf("Connected with %s encryption\n", SSL_get_cipher(g_ssl));
        //ShowCerts(g_ssl);
    }
    //printf("server ssl connected\n");

    return 0;
}

int ssl_close()
{
    SSL_shutdown(g_ssl);
    SSL_free(g_ssl);
    SSL_CTX_free(g_ctx);

    return 0;
}

int g_sum = 0;
void sig_child(int sig)
{
    pid_t pid;
    int status;
    while((pid = waitpid(-1, &status, WNOHANG)) > 0) {
        g_sum++;
        //printf("child pid exit :%d\n", pid);
    }

    signal(SIGCHLD, sig_child);

    return;
}

int fork_child(int i)
{
    pid_t pid;

    if(i<2) {
        return 0;
    }

    g_isdaemon = 1;
    if(daemon(0,0) == -1) {
        perror("daemon() error");
        exit(0);
    }

    signal(SIGCHLD, sig_child);

    g_sum = i;
    while(1) {
        if(g_sum == 0) {
            sleep(1);
            continue;
        }

        pid = fork();
        if(pid < 0) {
            //perror("fork() error");
            sleep(1);
        } else if(pid == 0) {
            //printf("newo child pid :%d\n", getpid());
            return 0;
        } else {
            g_sum--;
        }
        usleep(33000);
    }
}

int main(int argc, char **argv)
{
    int sockfd;
    int retval;

    printf("Usage: %s <server_ip> <server_port> <imap | pop | smtp> <ssl | nossl> <childs>\n\n", argv[0]);

    if (argc != 6) {
        printf("args error.\n");
        exit(0);
    }

    g_isdaemon = 0;
    fork_child(atoi(argv[5]));

    sockfd = tcp_connect(argv[1], atoi(argv[2]));

    if(strcmp(argv[4], "ssl") == 0) {
        if(ssl_connect(sockfd) == -1) {
            if(!g_isdaemon) {
                printf("ssl_connect() error");
            }
            exit(0);
        }
    } else {
        g_ssl = NULL;
    }

    if(strcmp(argv[3], "imap") == 0) {
        test_cmd(sockfd, g_ssl, imap_cmds);
    } else if(strcmp(argv[3], "pop") == 0) {
        test_cmd(sockfd, g_ssl, pop_cmds);
    } else if(strcmp(argv[3], "smtp") == 0) {
        test_cmd(sockfd, g_ssl, smtp_cmds);
    } else {
        if(!g_isdaemon) {
            printf(" test type [%s] error.\n", argv[3]);
        }
    }

    if(g_ssl) {
        ssl_close();
    }

    close(sockfd);

    return 0;
}


