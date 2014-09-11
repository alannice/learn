
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
    if (SSL_accept(ossl->ssl) == -1) {
        SSL_shutdown(ossl->ssl);
        SSL_free(ossl->ssl);
        ossl->ssl = NULL;

        return -1;
    }

    return 0;
}

int xyz_ssl_connect(struct xyz_ssl_t *ossl, int sockfd)
{
    if(ossl == NULL || sockfd < 0) {
        return -1;
    }

    SSL_CTX_set_verify(ossl->ctx, SSL_VERIFY_PEER, NULL);

    ossl->ssl = SSL_new(ossl->ctx);
    if(ossl->ssl == NULL) {
        return -1;
    }

    SSL_set_fd(ossl->ssl, sockfd);
    if (SSL_connect(ossl->ssl) == -1) {
        SSL_shutdown(ossl->ssl);
        SSL_free(ossl->ssl);
        ossl->ssl = NULL;

        return -1;
    }

    if(SSL_get_verify_result(ossl->ssl) == X509_V_OK) {
        ERR_print_errors_fp(stdout);
    }

    return 0;
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

    if (ossl == NULL || data == NULL) {
        return -1;
    }
    if (len == 0) {
        return 0;
    }

    if(SSL_pending(ossl->ssl) <= 0) {
        return 0;
    }

    n = SSL_read(ossl->ssl, data, len);
    if (n < 0) {
        int err = SSL_get_error(ossl->ssl, n);
        if(err == SSL_ERROR_WANT_WRITE || err == SSL_ERROR_WANT_READ) {
            return 0;
        }
        return -1;
    } else if (n == 0) {
        return -1;
    }

    return n;
}

int xyz_ssl_write(struct xyz_ssl_t *ossl, char *data, int len)
{
    int n;

    if (ossl == NULL || data == NULL) {
        return -1;
    }
    if(len == 0) {
        return 0;
    }

    n = SSL_write(ossl->ssl, data, len);
    if (n < 0) {
        int err = SSL_get_error(ossl->ssl, n);
        if(err == SSL_ERROR_WANT_WRITE || err == SSL_ERROR_WANT_READ) {
            return 0;
        }
        return -1;
    } else if (n == 0) {
        return -1;
    }

    return n;
}

