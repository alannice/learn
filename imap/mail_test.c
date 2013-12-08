
// gcc mail_test.c -o mail_test -lssl

/************���ڱ��ĵ�********************************************
 *  * *filename: ssl-client.c
 *   * *purpose: ��ʾ����OpenSSL ����л���IP ���SSL ����ͨѶ�ķ��������ǿͻ�������
 *    * *wrote by: zhoulifa(zhoulifa@163.com) ������(http://zhoulifa.bokee.com)
 *     * Linux ������Linux ֪ʶ������SOHO �忪�������ó�C ����
 *      * *date time:2007-02-02 20:10
 *       * *Note: �κ��˿������⸴�ƴ��벢������Щ�ĵ�����Ȼ���������ҵ��;
 *        * * ������ѭGPL
 *         * *Thanks to:Google
 *          * *Hope:ϣ��Խ��Խ����˹����Լ���������Ϊ��ѧ������չ����
 *           * * �Ƽ�վ�ھ��˵ļ���Ͻ������죡��л�п�Դǰ���Ĺ��ף�
 *            * *********************************************************************/

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <sys/socket.h>
//#include <resolv.h>
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
        {NULL, NULL},
};

void ShowCerts(SSL * ssl)
{
        X509 *cert;
        char *line;
        cert = SSL_get_peer_certificate(ssl);
        if (cert != NULL) {
                //printf("����֤����Ϣ:\n");
                line = X509_NAME_oneline(X509_get_subject_name(cert), 0, 0);
                //printf("֤��: %s\n", line);
                free(line);
                line = X509_NAME_oneline(X509_get_issuer_name(cert), 0, 0);
                //printf("�䷢��: %s\n", line);
                free(line);
                X509_free(cert);
        } else {
                //printf("��֤����Ϣ��\n");
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
                /* Don��t rely on the value of tv now! */

                if (retval == -1) {
                        //perror("select()");
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
                                //printf(data);
                                fflush(stdout);
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
                //printf("recv ready error");
                return;
        }

        int i=0;
        for(i=0; cmds[i].label; i++) {
                //printf("\r\n%s", cmds[i].cmd);
                if(ssl) {
                        num = SSL_write(ssl, cmds[i].cmd, strlen(cmds[i].cmd));
                } else {
                        num = write(fd, cmds[i].cmd, strlen(cmds[i].cmd));
                }
                if(num != strlen(cmds[i].cmd)) {
                        //printf("send [%s] len error: [%d]\n", cmds[i].label, num);
                        syslog(LOG_DEBUG, "send [%s] len error: [%d]\n", cmds[i].label, num);
                }
                num = test_read(fd, ssl);
                if(num == -1) {
                        //printf("recv [%s] error\n", cmds[i].label);
                        syslog(LOG_DEBUG, "recv [%s] error\n", cmds[i].label);
                        return;
                }
                fflush(stdout);
        }

        return;
}

int tcp_connect(char *ip, int port)
{
        int sockfd, len;
        struct sockaddr_in dest;

        /* ����һ��socket ����tcp ͨ��*/
        if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
                //perror("Socket");
                syslog(LOG_DEBUG, "Socket() error");
                exit(errno);
        }
        //printf("socket created\n");

        /* ��ʼ���������ˣ��Է����ĵ�ַ�Ͷ˿���Ϣ*/
        bzero(&dest, sizeof(dest));
        dest.sin_family = AF_INET;
        dest.sin_port = htons(port);
        if (inet_aton(ip, (struct in_addr *) &dest.sin_addr.s_addr) == 0) {
                //perror(ip);
                syslog(LOG_DEBUG, "inet_aton() error");
                exit(errno);
        }
        //printf("address created\n");

        /* ���ӷ�����*/
        if (connect(sockfd, (struct sockaddr *) &dest, sizeof(dest)) != 0) {
                //perror("Connect ");
                syslog(LOG_DEBUG, "Connect() error");
                exit(errno);
        }
        //printf("server connected\n");

        return sockfd;
}

SSL_CTX *g_ctx;
SSL *g_ssl;

int ssl_connect(int sockfd)
{
        /* SSL ���ʼ�����ο�ssl-server.c ����*/
        SSL_library_init();
        OpenSSL_add_all_algorithms();
        SSL_load_error_strings();
        g_ctx = SSL_CTX_new(SSLv23_client_method());
        if (g_ctx == NULL) {
                ERR_print_errors_fp(stdout);
                exit(1);
        }

        /* ����ctx ����һ���µ�SSL */
        g_ssl = SSL_new(g_ctx);
        if(g_ssl == NULL) {
                ERR_print_errors_fp(stdout);
                exit(1);
        }

        SSL_set_fd(g_ssl, sockfd);

        /* ����SSL ����*/
        if (SSL_connect(g_ssl) == -1) {
                ERR_print_errors_fp(stderr);
                exit(1);
        } else {
                //printf("Connected with %s encryption\n", SSL_get_cipher(g_ssl));
                ShowCerts(g_ssl);
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
                syslog(LOG_DEBUG, "child pid exit :%d\n", pid);
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

        //signal(SIGCHLD, sig_child);

        if(daemon(0,0) == -1) {
                //perror("daemon() error");
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
                        syslog(LOG_DEBUG, "fork() error");
                        sleep(1);
                } else if(pid == 0) {
                        //printf("newo child pid :%d\n", getpid());
                        signal(SIGCHLD, SIG_DFL);
                        syslog(LOG_DEBUG, "new child pid :%d\n", getpid());
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

        printf("Usage: %s <server_ip> <server_port> <imap | pop> <ssl | nossl> <childs>\n\n", argv[0]);

        if (argc != 6) {
                printf("args error.\n");
                exit(0);
        }

        openlog("mailtest", LOG_CONS|LOG_PID, LOG_MAIL);

        fork_child(atoi(argv[5]));

        sockfd = tcp_connect(argv[1], atoi(argv[2]));

        if(strcmp(argv[4], "ssl") == 0) {
                if(ssl_connect(sockfd) == -1) {
                        syslog(LOG_DEBUG, "ssl_connect() error");
                        exit(0);
                }
        } else {
                g_ssl = NULL;
        }

        if(strcmp(argv[3], "imap") == 0) {
                test_cmd(sockfd, g_ssl, imap_cmds);
        } else if(strcmp(argv[3], "pop") == 0) {
                test_cmd(sockfd, g_ssl, pop_cmds);
        } else {
                //printf(" test type [%s] error.\n", argv[3]);
                syslog(LOG_DEBUG, " test type [%s] error.\n", argv[3]);
        }

        if(g_ssl) {
                ssl_close();
        }

        close(sockfd);

        return 0;
}


