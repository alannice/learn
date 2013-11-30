
#ifndef __XYZ_SOCKET_H__
#define __XYZ_SOCKET_H__

int xyz_sock_noblock(int sockfd);
int xyz_sock_listen(char *addr, int port);
int xyz_sock_accept(int sockfd);
int xyz_sock_connect(char *addr, int port);
void xyz_sock_setopt(int sockfd);

#endif // __XYZ_SOCKET_H__
