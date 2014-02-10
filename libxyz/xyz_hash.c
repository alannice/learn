
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

    hash = malloc(sizeof(struct xyz_hash_t));
    if(hash == NULL) {
        return NULL;
    }

    hash->mp = xyz_mpool_create("hash_list", sizeof(struct xyz_hash_list_t));
    if(hash->mp == NULL) {
        free(hash);
        return NULL;
    }

    hash->size = size;
    hash->count = 0;

    hash->list = (void *)malloc(sizeof(struct xyz_hash_list_t *)*size);
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
            free(hash->list);
        }
        free(hash);
    }
    
    return;
}

void *xyz_hash_get(struct xyz_hash_t *hash, char *key)
{
    int nkey = bkdr_hash(key)%hash->size;
    struct xyz_hash_list_t *list = hash->list[nkey];

    if(list == NULL) {
        return NULL;
    }

    while(list) {
        if(strcmp(key, list->key) == 0) {
            return list->data;
        }
        list = list->next;
    }
    
    return NULL;
}

int xyz_hash_add(struct xyz_hash_t *hash, char *key, void *data)
{
    int nkey = bkdr_hash(key)%hash->size;
    struct xyz_hash_list_t *list = hash->list[nkey];
    struct xyz_hash_list_t *tmp;

    tmp = xyz_mpool_malloc(hash->mp);
    strncpy(tmp->key, key, sizeof(tmp->key)-1);
    tmp->data = data;
    tmp->next = NULL;

    if(list == NULL) {
        hash->list[nkey] = tmp;
        hash->count++;
        return 0;
    }
    while(list->next) {
        if(strcmp(list->key, key) == 0) {
            return -1;
        }
        list = list->next;
    }
    list->next = tmp;
    hash->count++;

	return 0;
}

int xyz_hash_del(struct xyz_hash_t *hash, char *key)
{
    int nkey = bkdr_hash(key)%hash->size;
    struct xyz_hash_list_t *list = hash->list[nkey];
    struct xyz_hash_list_t *prov = NULL;

    if(list == NULL) {
        return 0;
    }

    while(list) {
        if(strcmp(list->key, key) == 0) {
            if(prov) {
                prov->next = list->next;
            } else {
                hash->list[nkey] = NULL;
            }
            xyz_mpool_free(hash->mp, list);
            hash->count--;
            return 0;
        } 
        prov = list;
        list = list->next;
    }

	return 0;
}

int xyz_hash_state(struct xyz_hash_t *hash)
{
    struct xyz_hash_list_t *list;
    int i;

    if(hash == NULL) {
        return -1;
    }

    printf("---xyz_hash_state---\n");
    printf("hash size : %d\n", hash->size);
    printf("hash count : %d\n", hash->count);
    printf("hash node :");
    for(i=0; i<hash->size; i++) {
        list = hash->list[i];
        if(list) {
            printf("\n <%d>", i);
        }
        while(list) {
            printf(" [%s]", list->key);
            list = list->next;
        }
    }
    printf("\n---the end---\n");

	return 0;
}

////////////////////////////////////////////////////

#if 1

int main(void)
{
    struct xyz_hash_t *hash;
    hash = xyz_hash_create(128);
    if(hash == NULL) {
        printf("xyz_hash_create() error\n");
        return 1;
    }

    xyz_hash_add(hash, "hello", "hello");
    xyz_hash_add(hash, "world", "world");
    xyz_hash_add(hash, "hoho", "hoho");
    xyz_hash_add(hash, "gaga", "gaga");
    xyz_hash_state(hash);

    xyz_hash_del(hash, "hoho");
    xyz_hash_state(hash);

    char *str = xyz_hash_get(hash, "hello");
    if(str) {
        printf("get hash hello value : %s\n", str);
    }

    return 0;
}

#endif // 0

