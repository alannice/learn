
#ifndef __SOCKET_H__
#define __SOCKET_H__

int sock_noblock(int sockfd);
int sock_listen(char *addr, int port);
int sock_accept(int sockfd);
int sock_connect(char *addr, int port);
void sock_setopt(int sockfd);

#endif // __SOCKET_H__
