
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <fcntl.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>

#include "xyz_sock.h"

#define LISTEN_MAX 128

int xyz_sock_noblock(int sockfd)
{
	int flag;

	flag = fcntl(sockfd,F_GETFL);

	return fcntl(sockfd, F_SETFL, flag|O_NONBLOCK);
}

int xyz_sock_closeonexec(int sockfd)
{
	return fcntl(sockfd, F_SETFD, FD_CLOEXEC); 
}

int xyz_sock_listen(char *addr, int port)
{
	struct sockaddr_in sin;
	int sockfd;
	struct  hostent *ht;

	memset (&sin, 0, sizeof (sin));

	if(addr != NULL) {
		memset (&sin, 0, sizeof (sin));
		ht = gethostbyname(addr);
		if(ht == NULL) {
			perror ("gethostbyname() error");
			return -1;
		}
		//sin.sin_addr = *((struct in_addr *)ht->h_addr);
		sin.sin_addr.s_addr = *((in_addr_t *)(ht->h_addr));
	} else {
		sin.sin_addr.s_addr = INADDR_ANY;
	}

    sin.sin_port = htons (port);
    sin.sin_family = AF_INET;

    sockfd = socket (PF_INET, SOCK_STREAM, 0);
    if (sockfd == -1)
    {
		perror ("socket() error");
		return -1;
    }

	int i=1;
	setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &i, sizeof(i));

    if (bind (sockfd, (struct sockaddr *) &sin, sizeof (sin)) == -1)
    {
		perror ("bind() error");
		return -1;
    }
    if (listen (sockfd, LISTEN_MAX) == -1)
    {
		perror ("listen() error");
		return -1;
    }

	return sockfd;
}

int xyz_sock_accept(int sockfd)
{
	struct sockaddr sa;
	socklen_t len;
	int conn;

	len = sizeof(sa);
	conn = accept(sockfd, &sa, &len);
	if(conn == -1) {
		perror("accept() error");
	}

	return conn;
}

int xyz_sock_connect(char *addr, int port)
{
	struct sockaddr_in sin;
	int sockfd;
	struct  hostent *ht;

	if(addr == NULL) {
		return -1;
	}

	ht = gethostbyname(addr);
	if(ht == NULL) {
		perror ("gethostbyname() error");
		return -1;
	}

	sockfd = socket(AF_INET, SOCK_STREAM, 0);

	memset (&sin, 0, sizeof (sin));
	sin.sin_addr.s_addr = *((in_addr_t *)(ht->h_addr));
	//sin.sin_addr = *((struct in_addr *)ht->h_addr);
    sin.sin_port = htons(port);
    sin.sin_family = AF_INET;

	if(connect(sockfd, (struct sockaddr *)&sin, sizeof(struct sockaddr)) == -1) {
		perror ("connect() error");
		return -1;
	}

	return sockfd;
}

int xyz_sock_peeraddr(int sockfd, char *addr, int len)
{
    struct sockaddr_in sa_in;
    socklen_t sa_len;  
    char *paddr;
    int pport;

    sa_len = sizeof(struct sockaddr);
    bzero(&sa_in, sizeof(struct sockaddr_in));
    if(getpeername(sockfd, (struct sockaddr *)&sa_in, &sa_len ) < 0)
    {
        return -1;
    }

    paddr = inet_ntoa(sa_in.sin_addr);
    pport = ntohs(sa_in.sin_port);

    if(paddr == NULL) {
        return -1;
    }

    strncpy(addr, paddr, len);

    return 0;
}

void xyz_sock_setopt(int sockfd)
{
	int keepalive = 1;
	setsockopt(sockfd,SOL_SOCKET,SO_KEEPALIVE,(void*)(&keepalive), sizeof(int));

#ifdef __linux__
	int keepalive_time = 30;
	setsockopt(sockfd, IPPROTO_TCP, TCP_KEEPIDLE,(void*)(&keepalive_time), sizeof(int));
	int keepalive_intvl = 3;
	setsockopt(sockfd, IPPROTO_TCP, TCP_KEEPINTVL,(void*)(&keepalive_intvl),sizeof(int));
	int keepalive_probes= 3;
	setsockopt(sockfd, IPPROTO_TCP, TCP_KEEPCNT,(void*)(&keepalive_probes),sizeof(int));
#endif // __linux__

	return;
}

//////////////////////////////////////////////////////////////////////////////

#if 0
int main(int argc, char *argv[])
{
	int fd;
	
	//sock_listen(NULL, 12345);
	fd = xyz_sock_connect("imap.sina.com", 143);
	printf("connect on %d\n", fd);

	char ip[32];
	xyz_sock_peeraddr(fd, ip, 30);
	printf("ip is :%s\n", ip);

	return 0;
}
#endif 

