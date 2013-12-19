
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "xyz_mpool.h"

#define XYZ_BLOCK_DEEP 32

struct xyz_mpool_t *xyz_mpool_create(char *label, int size)
{
	if(size <= 2) {
		return NULL;
	}

	struct xyz_mpool_t *mp = malloc(sizeof(struct xyz_mpool_t));
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

void xyz_mpool_destroy(struct xyz_mpool_t *mp)
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

int xyz_mpool_grow(struct xyz_mpool_t *mp)
{
	int i;

	if(mp == NULL) {
		return -1;
	}

	char *b = malloc(sizeof(char *) + mp->data_size * XYZ_BLOCK_DEEP);
	if(b == NULL) {
		return -1;
	}

	//printf("alloc block %p\n", b);

	char *p = b + sizeof(char *);
	for(i=0; i<XYZ_BLOCK_DEEP; i++) {
		*(char **)p = mp->free_list;
		mp->free_list = p;
		p += mp->data_size;
	}
	mp->free_count += XYZ_BLOCK_DEEP;

	*(void **)b = mp->block_list;
	mp->block_list = b;

	mp->block_count++;

	return 0;
}

void *xyz_mpool_malloc(struct xyz_mpool_t *mp)
{	char *p;

	if(mp == NULL) {
		return NULL;
	}

	if(mp->free_list == NULL) {
		xyz_mpool_grow(mp);
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

void xyz_mpool_free(struct xyz_mpool_t *mp, void *p)
{
	if(mp == NULL || p == NULL) {
		return;
	}

	*(char **)p = mp->free_list;
	mp->free_list = p;
	mp->free_count++;

	return;
}

void xyz_mpool_stat(struct xyz_mpool_t *mp, int v)
{
	char *p;

	printf("------ mpool stat ------\n");
	printf("*** %s ***\n", mp->label);
	printf("data_size:%d\n", mp->data_size);
	printf("free_count:%d\n", mp->free_count);
	printf("block_size:%d\n", mp->data_size*XYZ_BLOCK_DEEP+sizeof(char *));
	printf("block_count:%d\n", mp->block_count);

	if(v) {
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

/////////////////////////////////////////////////////////////////////////////

#if 0
int main(void)
{
	struct xyz_mpool_t *mp;
	char *a[8000];
	int i;

	mp = xyz_mpool_create("test", 30);

	printf("test1----------------\n");
	xyz_mpool_grow(mp);
	xyz_mpool_stat(mp, 1);

	printf("test2----------------\n");
	for(i=0; i<7890; i++) {
		a[i] = xyz_mpool_malloc(mp);
	}
	xyz_mpool_stat(mp, 0);

	printf("test3----------------\n");
	for(i=0; i<7800; i++) {
		xyz_mpool_free(mp, a[i]);
	}
	xyz_mpool_stat(mp, 0);

	printf("test4----------------\n");
	for(i=0; i<300; i++){
		xyz_mpool_grow(mp);
	}
	xyz_mpool_stat(mp, 0);

	xyz_mpool_destroy(mp);

	return 0;
}
#endif 

