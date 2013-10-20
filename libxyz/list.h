
#ifndef __LIST_H__
#define __LIST_H__

struct list_t;

typedef int (*list_foreach_t)(void *data, void *arg);

struct list_t *list_create(char *label);
void list_clear(struct list_t *list);
void list_destroy(struct list_t *list);
int list_push(struct list_t *list, void *data);
void *list_pop(struct list_t *list);
void *list_pushback(struct list_t *list, void *data);
void *list_popback(struct list_t *list);
int list_foreach(struct list_t *list, list_foreach_t func, void *arg, void **data);
int list_find(struct list_t *list, void *data);
int list_insert(struct list_t *list, void *data, void *newdata);
int list_delete(struct list_t *list, void *data);
void list_stat(struct list_t *list, int ex);

#endif // __LIST_H__

