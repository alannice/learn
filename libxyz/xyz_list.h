
#ifndef __XYZ_LIST_H__
#define __XYZ_LIST_H__

typedef int (*xyz_list_foreach_t)(void *data, void *arg);

struct xyz_list_node_t                                                                                           
{                                                                                                                
    void *data;                                                                                                  
    struct xyz_list_node_t *prev, *next;                                                                         
};                                                                                                               
                                                                                                                 
struct xyz_list_t                                                                                                
{                                                                                                                
    char label[32];                                                                                              
    struct xyz_mpool_t *mp;                                                                                      
    int count;                                                                                                   
    struct xyz_list_node_t *head, *tail;                                                                         
}; 

struct xyz_list_t *xyz_list_create(char *label);
void xyz_list_clear(struct xyz_list_t *list);
void xyz_list_destroy(struct xyz_list_t *list);
int xyz_list_puthead(struct xyz_list_t *list, void *data);
void *xyz_list_gethead(struct xyz_list_t *list);
int xy_list_puttail(struct xyz_list_t *list, void *data);
void *xyz_list_gettail(struct xyz_list_t *list);
int xyz_list_foreach(struct xyz_list_t *list, xyz_list_foreach_t func, void *arg, void **data);
int xyz_list_find(struct xyz_list_t *list, void *data);
int xyz_list_insert(struct xyz_list_t *list, void *data, void *newdata);
int xyz_list_delete(struct xyz_list_t *list, void *data);
void xyz_list_stat(struct xyz_list_t *list, int ex);

#endif // __XYZ_LIST_H__

