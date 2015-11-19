
#ifndef __XYZ_SSL_H__
#define __XYZ_SSL_H__

#include <unistd.h>
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
struct xyz_ssl_t *xyz_ssl_connect(int method, int sockfd);  
void xyz_ssl_destroy(struct xyz_ssl_t *ssl);  
int xyz_ssl_read(struct xyz_ssl_t *ossl, char *data, int len);
int xyz_ssl_write(struct xyz_ssl_t *ossl, char *data, int len);

#endif // __XYZ_SSL_H__
