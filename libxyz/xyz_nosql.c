
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "xyz_nosql.h"

//////////////////////////////////////////////////////////////////////////////

// memcached
// cc xyz_nosql.c -lmemcached -I/usr/local/include -L/usr/local/lib

#include <libmemcached/memcached.h>

struct xyz_mc_t {
    memcached_st *memc;
    memcached_server_st *serversa;
};

struct xyz_mc_t *xyz_mc_connect(char *ip, int port)
{
    struct xyz_mc_t *mc;
    memcached_return rc;
    int retval = 0;

    if(mc == NULL || ip == NULL || port <= 0) {
        return NULL;
    }

    mc = malloc(sizeof(struct xyz_mc_t));
    if(mc == NULL) {
        return NULL;
    }

    mc->memc = memcached_create(NULL);
    if(mc->memc == NULL) {
        free(mc);
        return  NULL;
    }

    rc = memcached_behavior_set(mc->memc, MEMCACHED_BEHAVIOR_BINARY_PROTOCOL, 1);
    if(rc != MEMCACHED_SUCCESS) {
        memcached_free(mc->memc);
        free(mc);
        return NULL;
    }

    mc->serversa = memcached_server_list_append(NULL, ip, port, &rc);
    if(rc != MEMCACHED_SUCCESS) {
        memcached_free(mc->memc);
        if(mc->serversa != NULL) {
            memcached_server_list_free(mc->serversa);
        }
        free(mc);
        return NULL;
    }

    rc = memcached_server_push(mc->memc, mc->serversa);
    if(rc != MEMCACHED_SUCCESS) {
        memcached_free(mc->memc);
        memcached_server_list_free(mc->serversa);
        free(mc);
        return NULL;
    }

    return mc;
}

void xyz_mc_destroy(struct xyz_mc_t *mc)
{
    if(mc != NULL) {
        if(mc->memc) {
            memcached_free(mc->memc);
        }
        if(mc->serversa) {
            memcached_server_list_free(mc->serversa);
        }

        free(mc);
    }

    return;
}

int xyz_mc_get(struct xyz_mc_t *mc, char *key, char *value, size_t vlen)
{
    memcached_return rc;
    void *tmpvalue = NULL;
    size_t tmplen = 0;
    uint32_t flag = 0;

    if(mc == NULL || key == NULL || value == NULL || vlen == 0) {
        return -1;
    }

    tmpvalue = memcached_get(mc->memc, key, strlen(key), &tmplen, &flag, &rc);
    if(rc != MEMCACHED_SUCCESS) {
        if(tmpvalue != NULL) {
            free(tmpvalue);
        }
        return rc;
    }

    if(tmpvalue == NULL || tmplen > vlen) {
        if(tmpvalue != NULL) {
            free(tmpvalue);
        }
        return -1;
    }

    memcpy(value, tmpvalue, tmplen);
    if(tmpvalue) {
        free(tmpvalue);
    }

    return 0;
}

int xyz_mc_set(struct xyz_mc_t *mc, int timeout, char *key, char *value, size_t vlen)
{
    memcached_return rc;
    int retval = 0;
    uint32_t flag = 0;

    if(mc == NULL || key == NULL || value == NULL || vlen == 0) {
        return -1;
    }

    rc = memcached_set(mc->memc, key, strlen(key), value, vlen, timeout, flag);
    if(rc != MEMCACHED_SUCCESS) {
        return rc;
    }

    return 0;
}

//////////////////////////////////////////////////////////////////////////////

#if 0

int main(void)
{
    int retval;

    struct xyz_mc_t *mc = xyz_mc_connect("127.0.0.1", 11211);
    if(mc == NULL) {
        printf("xyz_mc_connect() error\n");
        return 1;
    }

    retval = xyz_mc_set(mc, 100, "test", "hello", 5);
    if(retval != 0) {
        printf("xyz_mc_set() error : %d\n", retval);
        return 1;
    }

    char value[64] = {0};
    retval = xyz_mc_get(mc, "test", value, 60);
    if(retval != 0) {
        printf("xyz_mc_get() error : %d\n", retval);
        return 1;
    }
    printf("get value is %s\n", value);

    xyz_mc_destroy(mc);

    return 0;
}

#endif 

