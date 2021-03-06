
#include "xyz_ssl.h"

struct xyz_ssl_t *xyz_ssl_create(int method, char *cafile, char *pemfile)
{
    if(method != XYZ_SSLv23 && method != XYZ_TLSv1) {
        return NULL;
    }

    struct xyz_ssl_t *ossl = malloc(sizeof(struct xyz_ssl_t));
    if(ossl == NULL) {
        return NULL;
    }
    bzero(ossl, sizeof(struct xyz_ssl_t));

    ossl->method = method;

    SSL_library_init();
    OpenSSL_add_all_algorithms();
    SSL_load_error_strings();

    if(method == XYZ_SSLv23) {
        ossl->ctx = SSL_CTX_new(SSLv23_server_method());
    } else if(method == XYZ_TLSv1) {
        ossl->ctx = SSL_CTX_new(TLSv1_server_method());
    } else {
        free(ossl);
        return NULL;
    }

    if (ossl->ctx == NULL) {
        ERR_print_errors_fp(stdout);
        SSL_CTX_free(ossl->ctx);
        free(ossl);
        return NULL;
    }

    if(cafile) {
        if(SSL_CTX_load_verify_locations(ossl->ctx, cafile, NULL) != 1) {
            ERR_print_errors_fp(stdout);
        }
    }

    if(pemfile) {
        if(SSL_CTX_use_certificate_file(ossl->ctx, pemfile, SSL_FILETYPE_PEM) != 1) {
            ERR_print_errors_fp(stdout);
            SSL_CTX_free(ossl->ctx);
            free(ossl);
            return NULL;
        }

        if(SSL_CTX_use_PrivateKey_file(ossl->ctx, pemfile, SSL_FILETYPE_PEM) != 1) {
            ERR_print_errors_fp(stdout);
            SSL_CTX_free(ossl->ctx);
            free(ossl);
            return NULL;
        }

        if (SSL_CTX_check_private_key(ossl->ctx) != 1) {
            ERR_print_errors_fp(stdout);
            SSL_CTX_free(ossl->ctx);
            free(ossl);
            return NULL;
        }
    }
    
    return ossl;
}

int xyz_ssl_accept(struct xyz_ssl_t *ossl, int rfd, int wfd)
{
    if(ossl == NULL || rfd < 0 || wfd < 0) {
        return -1;
    }

    ossl->ssl = SSL_new(ossl->ctx);
    if(ossl->ssl == NULL) {
        return -1;
    }

    ossl->rdfd = rfd;
    ossl->wtfd = wfd;

    SSL_set_rfd(ossl->ssl, rfd);
    SSL_set_wfd(ossl->ssl, wfd);

    int n=0;
    int err = 0;
    time_t start=time(NULL);
    while(1) {
        if(time(NULL) - start > 10) {
            SSL_shutdown(ossl->ssl);
            SSL_free(ossl->ssl);
            ossl->ssl = NULL;
            return -1;
        }

        n = SSL_accept(ossl->ssl);
        if(n == 1) {
            return 0;
        }

        err = SSL_get_error(ossl->ssl, n);
        if(err == SSL_ERROR_WANT_WRITE || err == SSL_ERROR_WANT_READ) {
            usleep(1000*30);
            continue;
        }

        SSL_shutdown(ossl->ssl);
        SSL_free(ossl->ssl);
        ossl->ssl = NULL;

        return -1;
    }

    return 0;
}

struct xyz_ssl_t *xyz_ssl_connect(int method, int sockfd)
{
    if(sockfd < 0) {
        return NULL;
    }

    if(method != XYZ_SSLv23 && method != XYZ_TLSv1) {
        return NULL;
    }

    struct xyz_ssl_t *ossl = malloc(sizeof(struct xyz_ssl_t));
    if(ossl == NULL) {
        return NULL;
    }
    bzero(ossl, sizeof(struct xyz_ssl_t));

    ossl->method = method;

    SSL_library_init();
    OpenSSL_add_all_algorithms();
    SSL_load_error_strings();

    if(method == XYZ_SSLv23) {
        ossl->ctx = SSL_CTX_new(SSLv23_client_method());
    } else if(method == XYZ_TLSv1) {
        ossl->ctx = SSL_CTX_new(TLSv1_client_method());
    } else {
        free(ossl);
        return NULL;
    }

    if (ossl->ctx == NULL) {
        ERR_print_errors_fp(stdout);
        SSL_CTX_free(ossl->ctx);
        free(ossl);
        return NULL;
    }

    ossl->ssl = SSL_new(ossl->ctx);
    if(ossl->ssl == NULL) {
        SSL_CTX_free(ossl->ctx);
        free(ossl);
        return NULL;
    }

    SSL_set_fd(ossl->ssl, sockfd);

    int n = 0;
    int err = 0;
    time_t start = time(NULL);
    while(1) {
        if(time(NULL) - start > 10) {
            SSL_shutdown(ossl->ssl);
            SSL_free(ossl->ssl);
            SSL_CTX_free(ossl->ctx);
            free(ossl);

            return NULL;
        }

        n = SSL_connect(ossl->ssl);
        if(n == 1) {
            return ossl;
        }

        err = SSL_get_error(ossl->ssl, n);
        if(err == SSL_ERROR_WANT_WRITE || err == SSL_ERROR_WANT_READ) {
            usleep(1000*30);
            continue;
        }

        SSL_shutdown(ossl->ssl);
        SSL_free(ossl->ssl);
        SSL_CTX_free(ossl->ctx);
        free(ossl);

        return NULL;
    }

    return ossl;
}

void xyz_ssl_destroy(struct xyz_ssl_t *ossl)
{
    if(ossl == NULL) {
        return;
    }
    
    if(ossl->ssl) {
        SSL_shutdown(ossl->ssl);
        SSL_free(ossl->ssl);
    }
    if(ossl->ctx) {
        SSL_CTX_free(ossl->ctx);
    }

    free(ossl);

    return;
}

int xyz_ssl_read(struct xyz_ssl_t *ossl, char *data, int len)
{
    int n;

    if (ossl == NULL || data == NULL || len == 0) {
        return -1;
    }

    n = SSL_read(ossl->ssl, data, len);
    if (n <= 0) {
        int err = SSL_get_error(ossl->ssl, n);
        if(err == SSL_ERROR_WANT_WRITE || err == SSL_ERROR_WANT_READ) {
            return 0;
        }
        return -1;
    } 

    return n;
}

int xyz_ssl_write(struct xyz_ssl_t *ossl, char *data, int len)
{
    int n;

    if (ossl == NULL || data == NULL || len == 0) {
        return -1;
    }

    n = SSL_write(ossl->ssl, data, len);
    if (n <= 0) {
        int err = SSL_get_error(ossl->ssl, n);
        if(err == SSL_ERROR_WANT_WRITE || err == SSL_ERROR_WANT_READ) {
            return 0;
        }
        return -1;
    } 

    return n;
}

