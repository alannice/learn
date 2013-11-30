
#ifndef __XYZ_LIST_H__
#define __XYZ_LIST_H__

struct xyz_list_t;

typedef int (*xyz_list_foreach_t)(void *data, void *arg);

struct xyz_list_t *xyz_list_create(char *label);
void xyz_list_clear(struct xyz_list_t *list);
void xyz_list_destroy(struct xyz_list_t *list);
int xyz_list_push(struct xyz_list_t *list, void *data);
void *xyz_list_pop(struct xyz_list_t *list);
void *xy_list_pushback(struct xyz_list_t *list, void *data);
void *xyz_list_popback(struct xyz_list_t *list);
int xyz_list_foreach(struct xyz_list_t *list, xyz_list_foreach_t func, void *arg, void **data);
int xyz_list_find(struct xyz_list_t *list, void *data);
int xyz_list_insert(struct xyz_list_t *list, void *data, void *newdata);
int xyz_list_delete(struct xyz_list_t *list, void *data);
void xyz_list_stat(struct xyz_list_t *list, int ex);

#endif // __XYZ_LIST_H__

