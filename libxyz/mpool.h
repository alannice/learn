
#ifndef __MPOOL_H__
#define __MPOOL_H__

struct mpool_t;

struct mpool_t *mpool_create(char *label, int size);
void mpool_destroy(struct mpool_t *mp);
int mpool_grow(struct mpool_t *mp);
void *mpool_malloc(struct mpool_t *mp);
void mpool_free(struct mpool_t *mp, void *p);
void mpool_stat(struct mpool_t *mp, int dx);

#endif // __MPOOL_H__
