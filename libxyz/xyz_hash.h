
#ifndef __XYZ_HASH_H__
#define __XYZ_HASH_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "xyz_mpool.h"
#include "xyz_list.h"

struct xyz_hash_t {
	struct xyz_mpool_t *mp;
	int count;
	int size;
	struct xyz_list_t *list[];
};

struct xyz_hash_t *xyz_hash_create(int size);
void xyz_hash_destroy(struct xyz_hash_t *hash);
void *xyz_hash_get(struct xyz_hash_t *hash, char *key);
int xyz_hash_add(struct xyz_hash_t *hash, char *key, void *data);
int xyz_hash_del(struct xyz_hash_t *hash, char *key);
int xyz_hash_state(struct xyz_hash_t *hash);

#endif //__XYZ_HASH_H__

