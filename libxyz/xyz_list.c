/*
 * cc -o xyz_list xyz_list.c xyz_mpool.c -D__XYZ_LIST__
 */
 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "xyz_mpool.h"
#include "xyz_list.h"

struct xyz_list_t *xyz_list_create(char *label)
{
	struct xyz_list_t *l;

	l = malloc(sizeof(struct xyz_list_t));
	if(l == NULL) {
		return NULL;
	}

	if(label != NULL || strlen(label) > 0) {
		strncpy(l->label, label, sizeof(l->label)-1);
	}

	l->mp = xyz_mpool_create(l->label, sizeof(struct xyz_list_node_t));
	if(l->mp == NULL) {
		free(l);
		return NULL;
	}

	l->count = 0;
	l->head = l->tail = NULL;

	return l;
}

void xyz_list_clear(struct xyz_list_t *list)
{
	if(list == NULL) {
		return;
	}

	struct xyz_list_node_t *ln = list->head;

	while(list->head) {
		ln = list->head;
		list->head = list->head->next;
		xyz_mpool_free(list->mp, ln);
	}

	list->count = 0;
	list->head = list->tail = NULL;

	return;
}

void xyz_list_destroy(struct xyz_list_t *list)
{
	if(list == NULL) {
		return;
	}

	xyz_list_clear(list);

	xyz_mpool_destroy(list->mp);

	free(list);

	return;
}

int xyz_list_puthead(struct xyz_list_t *list, void *data)
{
	if(list == NULL || data == NULL) {
		return -1;
	}

	struct xyz_list_node_t *ln = xyz_mpool_malloc(list->mp);
	if(ln == NULL) {
		return -1;
	}
	ln->data = data;

	if(list->head == NULL && list->tail == NULL) {
		list->head = list->tail = ln;
		ln->prev = ln->next = NULL;
	} else {
		ln->next = list->head;
		ln->prev = NULL;
		list->head->prev = ln;
		list->head = ln;
	}

	list->count++;

	return 0;
}

void *xyz_list_gethead(struct xyz_list_t *list)
{
	if(list == NULL) {
		return NULL;
	}

	if(list->head == NULL && list->tail == NULL) {
		return NULL;
	}

	struct xyz_list_node_t *ln = list->head;

	if(list->head == list->tail) {
		list->head = list->tail = NULL;
	} else {
		list->head = list->head->next;
		list->head->prev = NULL;
	}

	list->count--;

	void *p = ln->data;
	xyz_mpool_free(list->mp, ln);

	return p;
}

int xyz_list_puttail(struct xyz_list_t *list, void *data)
{
	if(list == NULL || data == NULL) {
		return -1;
	}

	struct xyz_list_node_t *ln = xyz_mpool_malloc(list->mp);
	if(ln == NULL) {
		return -1;
	}
	ln->data = data;

	if(list->head == NULL && list->tail == NULL) {
		list->head = list->tail = ln;
		ln->prev = ln->next = NULL;
	} else {
		ln->prev = list->tail;
		ln->next = NULL;
		list->tail->next = ln;
		list->tail = ln;
	}

	list->count++;

	return 0;
}

void *xyz_list_gettail(struct xyz_list_t *list)
{
	if(list == NULL) {
		return NULL;
	}

	if(list->head == NULL && list->tail == NULL) {
		return NULL;
	}

	struct xyz_list_node_t *ln = list->tail;

	if(list->head == list->tail) {
		list->head = list->tail = NULL;
	} else {
		list->tail = list->tail->prev;
		list->tail->next = NULL;
	}

	list->count--;

	void *p = ln->data;
	xyz_mpool_free(list->mp, ln);

	return p;
}

// typedef int (*xyz_list_foreach_t)(void *data, void *arg);
int xyz_list_foreach(struct xyz_list_t *list, xyz_list_foreach_t func, void *arg, void **data)
{
	struct xyz_list_node_t *ln;
	int ret;

	if(list == NULL || func == NULL) {
		return -1;
	}

	ln = list->head;
	while(ln) {
		ret = func(ln->data, arg);
		if(ret != 0) {
			if(data != NULL) {
				*data = ln->data;
			}
			return ret;
		}

		ln = ln->next;
	}

	return 0;
}

int xyz_list_find(struct xyz_list_t *list, void *data)
{
	if(list == NULL || data == NULL) {
		return -1;
	}

	struct xyz_list_node_t *ln = list->head;

	while(ln) {
		if(ln->data == data)
			return 1;

		ln = ln->next;
	}

	return 0;
}

int xyz_list_insert(struct xyz_list_t *list, void *data, void *newdata)
{
	if(list == NULL || data == NULL || newdata == NULL) {
		return -1;
	}

	struct xyz_list_node_t *lntmp = list->head;
	while(lntmp) {
		if(lntmp->data == data) {
			struct xyz_list_node_t *ln = xyz_mpool_malloc(list->mp);
			if(ln == NULL) {
				return -1;
			}

			ln->data = newdata;
			ln->prev = lntmp;
			ln->next = lntmp->next;
			lntmp->next = ln;
			if(ln->next) {
				ln->next->prev = ln;
			} else {
				list->tail = ln;
			}

			list->count++;

			return 1;
		}

		lntmp = lntmp->next;
	}

	return 0;
}

int xyz_list_delete(struct xyz_list_t *list, void *data)
{
	if(list == NULL || data == NULL) {
		return -1;
	}

	struct xyz_list_node_t *ln = list->head;
	while(ln) {
		if(ln->data == data) {
			if(ln == list->head) {
				list->head = list->head->next;
				if(list->head) {
					list->head->prev = NULL;
				} else {
					list->tail = list->head = NULL;
				}
			} else if(ln == list->tail) {
				list->tail = list->tail->prev;
				if(list->tail) {
					list->tail->next = NULL;
				} else {
					list->head = list->tail = NULL;
				}
			} else {
				ln->prev->next = ln->next;
				ln->next->prev = ln->prev;
			}

			xyz_mpool_free(list->mp, ln);
			list->count--;

			return 1;
		}

		ln = ln->next;
	}

	return 0;
}

void xyz_list_stat(struct xyz_list_t *list, int ex)
{
	if(list == NULL) {
		return;
	}

	printf("------list stat------\n");
	printf("*** %s ***\n", list->label);
	printf("count : %d\n", list->count);
	printf("head : %p , tail : %p\n", list->head, list->tail);

	struct xyz_list_node_t *ln = list->head;
	while(ln) {
		printf("prev : %p , next : %p , data : %p:%s\n", ln->prev, ln->next, ln->data, (char *)ln->data);
		ln = ln->next;
	}

	if(ex) {
		xyz_mpool_stat(list->mp, 0);
	}

	return;
}

//////////////////////////////////////////////////////////////////////////////

#ifdef __XYZ_LIST__ 
int main(void)
{
	char *str[8] = {"aaa", "bbb", "ccc", "ddd", "eee", "fff", "ggg"};

	struct xyz_list_t *list = xyz_list_create("test list");

	printf("* append 123, 456\n");
	xyz_list_puttail(list,"123");
	xyz_list_puttail(list,"456");
	xyz_list_stat(list, 0);
	xyz_list_clear(list);

	printf(" * push 123, 456\n");
	xyz_list_puthead(list, "123");
	xyz_list_puthead(list, "456");
	xyz_list_stat(list, 0);
	//list_clear(list);

	printf(" * pop one\n");
	xyz_list_gethead(list);
	xyz_list_stat(list, 0);
	xyz_list_clear(list);

	int i;
	for(i=0; i<8; i++) {
		printf("* append %s\n", str[i]);
		xyz_list_puttail(list, str[i]);
		xyz_list_stat(list, 0);
	}

    printf("* back one\n");
	xyz_list_gettail(list);
	xyz_list_stat(list, 0);

	xyz_list_find(list, str[3]);
	printf("* insert 456789 after %s\n", str[3]);
	xyz_list_insert(list, str[3], "456789");
	xyz_list_stat(list, 0);

	printf("* delete %s\n", str[3]);
	xyz_list_delete(list,str[3]);
	xyz_list_stat(list, 0);

	xyz_list_destroy(list);

	return 0;
}

#endif  // __XYZ_LIST__


