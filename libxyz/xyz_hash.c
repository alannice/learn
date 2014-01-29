
#include "xyz_hash.h"

signed int bkdr_hash(char *str)
{
    unsigned int seed = 131; // 31 131 1313 13131 131313 etc..
    unsigned int hash = 0;

    while (*str)
    {
        hash = hash * seed + (*str++);
    }

    return (hash & 0x7FFFFFFF);
}

struct xyz_hash_t *xyz_hash_create(int size)
{
    struct xyz_hash_t *hash;

    if(size < 2) {
        return NULL;
    }

    hash = malloc(sizeof(struct xyz_list_t));
    if(hash == NULL) {
        return NULL;
    }

    hash->mp = xyz_mpool_create("hash", size);
    if(hash->mp == NULL) {
        free(hash);
        return NULL;
    }

    hash->size = size;
    hash->count = 0;

    hash->list = (void *)malloc(sizeof(struct xyz_list *)*size);
    if(hash->list == NULL) {
        xyz_mpool_destroy(hash->mp);
        free(hash);
        return NULL;
    }

    return hash;
}

void xyz_hash_destroy(struct xyz_hash_t *hash)
{
    if(hash) {
        if(hash->mp) {
            xyz_mpool_destroy(hash->mp);
        }
        if(hash->list) {
            int i=0;
            for(i=0; i<hash->size; i++) {
                xyz_list_destroy(hash->list[i]);
            }
            free(hash->list);
        }
        free(hash);
    }
    
    return;
}

void *xyz_hash_get(struct xyz_hash_t *hash, char *key)
{
    int nkey = bkdr_hash(key)/hash->size;
    struct xyz_list_t *list = hash->list[nkey];

   // xyz_list_get(list, key);
    
    return NULL;
}

int xyz_hash_add(struct xyz_hash_t *hash, char *key, void *data)
{
    int nkey = bkdr_hash(key)/hash->size;
    struct xyz_list_t *list = hash->list[nkey];

	return 0;
}

int xyz_hash_del(struct xyz_hash_t *hash, char *key)
{
    int nkey = bkdr_hash(key)/hash->size;
    struct xyz_list_t *list = hash->list[nkey];

	return 0;
}

int xyz_hash_state(struct xyz_hash_t *hash)
{
    int nkey = bkdr_hash(key)/hash->size;
    struct xyz_list_t *list = hash->list[nkey];

	return 0;
}

////////////////////////////////////////////////////

#if 0

int main(void)
{


    return 0;
}

#endif // 0

