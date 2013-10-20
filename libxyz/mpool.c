
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "mpool.h"

#define BLOCK_DEEP 32

struct mpool_t {
	char label[32];
	int data_size;

	char *free_list;
	int free_count;

	char *block_list;
	int block_count;
};

struct mpool_t *mpool_create(char *label, int size)
{
	if(size <= 2) {
		return NULL;
	}

	struct mpool_t *mp = malloc(sizeof(struct mpool_t));
	if(mp==NULL) {
		return NULL;
	}

	if(label != NULL && strlen(label) > 0) {
		strncpy(mp->label, label, sizeof(mp->label)-1);
	}

	mp->data_size = size;
	if(size & 0x7) {
		mp->data_size = ((size>>3)<<3)+8;
	}

	mp->free_list = NULL;
	mp->free_count = 0;
	mp->block_list = NULL;
	mp->block_count = 0;

	return mp;
}

void mpool_destroy(struct mpool_t *mp)
{
	char *p;

	if(mp == NULL) {
		return;
	}

	while(mp->block_list) {
		p = mp->block_list;
		mp->block_list = *(char **)p;
		//printf("free block %p\n", p);
		free(p);
	}

	free(mp);

	return;
}

int mpool_grow(struct mpool_t *mp)
{
	int i;

	if(mp == NULL) {
		return -1;
	}

	char *b = malloc(sizeof(char *) + mp->data_size * BLOCK_DEEP);
	if(b == NULL) {
		return -1;
	}

	//printf("alloc block %p\n", b);

	char *p = b + sizeof(char *);
	for(i=0; i<BLOCK_DEEP; i++) {
		*(char **)p = mp->free_list;
		mp->free_list = p;
		p += mp->data_size;
	}
	mp->free_count += BLOCK_DEEP;

	*(void **)b = mp->block_list;
	mp->block_list = b;

	mp->block_count++;

	return 0;
}

void *mpool_malloc(struct mpool_t *mp)
{	char *p;

	if(mp == NULL) {
		return NULL;
	}

	if(mp->free_list == NULL) {
		mpool_grow(mp);
	}

	if(mp->free_list == NULL) {
		return NULL;
	}

	p = mp->free_list;
	mp->free_list = *(void **)p;
	mp->free_count--;

	bzero(p, mp->data_size);

	return p;
}

void mpool_free(struct mpool_t *mp, void *p)
{
	if(mp == NULL || p == NULL) {
		return;
	}

	*(char **)p = mp->free_list;
	mp->free_list = p;
	mp->free_count++;

	return;
}

void mpool_stat(struct mpool_t *mp, int dx)
{
	char *p;

	printf("------ mpool stat ------\n");
	printf("*** %s ***\n", mp->label);
	printf("data_size:%d\n", mp->data_size);
	printf("free_count:%d\n", mp->free_count);
	printf("block_size:%d\n", mp->data_size*BLOCK_DEEP+sizeof(char *));
	printf("block_count:%d\n", mp->block_count);

	if(dx) {
		printf("free_list display:\n");
		p = mp->free_list;
		while(p) {
			printf(" %p ", p);
			p = *(char **)p;
		}
		printf("\n");

		printf("block_list display:\n");
		p = mp->block_list;
		while(p) {
			printf(" %p ", p);
			p = *(char **)p;
		}
		printf("\n");
	}

	return;
}

/////////////////////////////////////////////////
/*
int main(void)
{
	struct mpool_t *mp;
	char *a[8000];
	int i;

	mp = mpool_create("test", 30);

	printf("test1----------------\n");
	mpool_grow(mp);
	mpool_stat(mp, 1);

	printf("test2----------------\n");
	for(i=0; i<7890; i++) {
		a[i] = mpool_malloc(mp);
	}
	mpool_stat(mp, 0);

	printf("test3----------------\n");
	for(i=0; i<7800; i++) {
		mpool_free(mp, a[i]);
	}
	mpool_stat(mp, 0);

	printf("test4----------------\n");
	for(i=0; i<300; i++){
		mpool_grow(mp);
	}
	mpool_stat(mp, 0);

	mpool_destroy(mp);

	return 0;
}
*/
