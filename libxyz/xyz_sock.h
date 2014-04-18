
#ifndef __XYZ_SOCKET_H__
#define __XYZ_SOCKET_H__

#define XYZ_SOCKET_TCP  1
#define XYZ_SOCKET_UDP  2
#define XYZ_SOCKET_SCTP  3

int xyz_sock_noblock(int sockfd);
int xyz_sock_closeonexec(int sockfd);
int xyz_sock_listen(char *addr, int port, int type);
int xyz_sock_accept(int sockfd);
int xyz_sock_connect(char *addr, int port, int type);
int xyz_sock_peeraddr(int sockfd, char *addr, int len);
int xyz_sock_read_to(int sockfd, char *data, int len, int usec);
int xyz_sock_write_to(int sockfd, char *data, int len, int usec);
int xyz_domain_listen(char *addr);
int xyz_domain_connect(char *addr); 

#endif // __XYZ_SOCKET_H__
