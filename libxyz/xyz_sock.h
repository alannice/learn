
#ifndef __XYZ_SOCKET_H__
#define __XYZ_SOCKET_H__

#define XYZ_SOCKET_TCP  1
#define XYZ_SOCKET_UDP  2
#define XYZ_SOCKET_SCTP  3

int xyz_sock_noblock(int sockfd);
int xyz_sock_listen(char *addr, int port, int type);
int xyz_sock_accept(int sockfd);
int xyz_sock_connect(char *addr, int port, int type);
int xyz_sock_peeraddr(int sockfd, char *addr, int len);
void xyz_sock_setopt(int sockfd);

#endif // __XYZ_SOCKET_H__
