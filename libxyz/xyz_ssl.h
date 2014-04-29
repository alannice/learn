
#ifndef __XYZ_SSL_H__
#define __XYZ_SSL_H__

#include <openssl/ssl.h>
#include <openssl/err.h>

#define XYZ_SSLv23  1
#define XYZ_TLSv1   2

struct xyz_ssl_t {
    SSL_CTX *ctx;    
    SSL *ssl;
    int method;
    int rdfd;
    int wtfd;
    int sockfd;
};

struct xyz_ssl_t *xyz_ssl_create(int method, char *cafile, char *pemfile); 
int xyz_ssl_accept(struct xyz_ssl_t *ssl, int rfd, int wfd);  
int xyz_ssl_connect(struct xyz_ssl_t *ssl, int sockfd);  
void xyz_ssl_destroy(struct xyz_ssl_t *ssl);  

#endif // __XYZ_SSL_H__
