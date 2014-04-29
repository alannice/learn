
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
#include <sys/select.h>
#include <sys/un.h> 
#include <sys/stat.h>

#include "xyz_sock.h"

#define XYZ_LISTEN_MAX 128

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

int xyz_sock_listen(char *addr, int port, int type)
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
		sin.sin_addr.s_addr = *((in_addr_t *)(ht->h_addr));
	} else {
		sin.sin_addr.s_addr = INADDR_ANY;
	}

    sin.sin_port = htons (port);
    sin.sin_family = AF_INET;

    switch(type) {
        case XYZ_SOCKET_TCP:
            sockfd = socket (PF_INET, SOCK_STREAM, IPPROTO_TCP);
            break;
        case XYZ_SOCKET_UDP:
            sockfd = socket (PF_INET, SOCK_DGRAM, IPPROTO_UDP);
            break;
        case XYZ_SOCKET_SCTP:
            sockfd = socket (PF_INET, SOCK_SEQPACKET, IPPROTO_SCTP);
            break;
        default:
            return -1;
    }
    if(sockfd == -1) {
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
    if (listen (sockfd, XYZ_LISTEN_MAX) == -1)
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

int xyz_sock_connect(char *addr, int port, int type)
{
	struct sockaddr_in sin;
	int sockfd;
	struct  hostent *ht;

	if(addr == NULL) {
		return -1;
	}

    switch(type) {
        case XYZ_SOCKET_TCP:
            sockfd = socket (PF_INET, SOCK_STREAM, IPPROTO_TCP);
            break;
        case XYZ_SOCKET_UDP:
            sockfd = socket (PF_INET, SOCK_DGRAM, IPPROTO_UDP);
            break;
        case XYZ_SOCKET_SCTP:
            sockfd = socket (PF_INET, SOCK_SEQPACKET, IPPROTO_SCTP);
            break;
        default:
            return -1;
    }
    if(sockfd == -1) {
		perror ("socket() error");
		return -1;
    }

	ht = gethostbyname(addr);
	if(ht == NULL) {
		perror ("gethostbyname() error");
		return -1;
	}
	memset (&sin, 0, sizeof (sin));
	sin.sin_addr.s_addr = *((in_addr_t *)(ht->h_addr));
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

    if(addr == NULL || len < 15) {
        return -1;
    }

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

int xyz_sock_read_to(int sockfd, char *data, int len, int usec)
{
    fd_set rdset;
    FD_ZERO(&rdset);
    FD_SET(sockfd, &rdset);
    struct timeval tv;
    tv.tv_sec = usec/1000000;
    tv.tv_usec = usec%1000000;

    int n = select(sockfd+1, &rdset, NULL, NULL, &tv);
    if(n == 0) {
        return 0;
    }
    if(n == 1) {
        return read(sockfd, data, len);
    }
    if(n == -1) {
        if(errno == EINTR) {
            return 0;
        } else {
            return -1;
        }
    }
    
    return 0;
}

int xyz_sock_write_to(int sockfd, char *data, int len, int usec)
{
    fd_set wtset;
    FD_ZERO(&wtset);
    FD_SET(sockfd, &wtset);
    struct timeval tv;
    tv.tv_sec = usec/1000000;
    tv.tv_usec = usec%1000000;

    int n = select(sockfd+1, NULL, &wtset, NULL, &tv);
    if(n == 0) {
        return 0;
    }
    if(n == 1) {
        return write(sockfd, data, len);
    }
    if(n == -1) {
        if(errno == EINTR) {
            return 0;
        } else {
            return -1;
        }
    }

    return 0;
}

int xyz_domain_listen(char *addr)
{
    int listen_fd;
    int ret;
    struct sockaddr_un srv_addr;

    listen_fd=socket(AF_UNIX, SOCK_STREAM, IPPROTO_TCP); 
    if(listen_fd < 0) {
        perror("socket() error");
        return -1;
    }

    strncpy(srv_addr.sun_path, addr, sizeof(srv_addr.sun_path)-1); 
    ret=bind(listen_fd, (struct sockaddr*)&srv_addr, sizeof(srv_addr)); 
    if(ret == -1)
    { 
        perror("bind() error"); 
        close(listen_fd); 
        unlink(addr); 
        return -1; 
    } 

    ret=listen(listen_fd,1); 
    if(ret == -1) {
        perror("listen() error");
        close(listen_fd);
        unlink(addr);
        return -1;
    }

    unlink(addr); 

    return listen_fd;
}

int xyz_domain_connect(char *addr)
{
    int connect_fd;
    struct sockaddr_un srv_addr; 
    int ret;

    connect_fd = socket(AF_UNIX, SOCK_STREAM, IPPROTO_TCP); 
    if(connect_fd < 0)
    { 
        perror("socket() error");
        return -1;
    } 

    srv_addr.sun_family=AF_UNIX; 
    strncpy(srv_addr.sun_path, addr, sizeof(srv_addr.sun_path)-1);

    ret=connect(connect_fd, (struct sockaddr*)&srv_addr, sizeof(srv_addr)); 
    if(ret == -1)
    { 
        printf("connect() error");
        close(connect_fd); 
        return -1;
    }

    return connect_fd;
}

#ifdef __linux__

#include <sys/sendfile.h>

int xyz_sock_sendfile(int sockfd, char *file)
{
    int fd;
    struct stat statbuf;

    if(stat(file, &statbuf) == -1) {
        return -1;
    }
    
    fd = open(file, O_RDONLY);
    if(fd == -1) {
        return -1;
    }

    if(statbuf.st_size != sendfile(sockfd, fd, 0, statbuf.st_size)) {
        close(fd);
        return -1;
    }

    close(fd);

    return 0;
}
#endif // __linux__

#ifdef __FreeBSD__

#include <sys/uio.h>

int xyz_sock_sendfile(int sockfd, char *file)
{
    int fd;
    off_t sndlen;
    struct stat statbuf;

    if(stat(file, &statbuf) == -1) {
        return -1;
    }
    
    fd = open(file, O_RDONLY);
    if(fd == -1) {
        return -1;
    }

    if(sendfile(fd, sockfd, 0, statbuf.st_size, NULL, &sndlen, 0) != 0) {
        return -1;
    }

    if(sndlen != statbuf.st_size) {
        return -1;
    }

    return 0;
}
#endif  // __FreeBSD__


//////////////////////////////////////////////////////////////////////////////

#if 0
int main(int argc, char *argv[])
{
	int fd;
	
	//sock_listen(NULL, 12345, XYZ_SOCKET_TCP);
	fd = xyz_sock_connect("imap.sina.com", 143, XYZ_SOCKET_TCP);
	printf("connect on %d\n", fd);

	char ip[32];
	xyz_sock_peeraddr(fd, ip, 30);
	printf("ip is :%s\n", ip);

	return 0;
}
#endif 

