
#ifndef __XYZ_MPOOL_H__
#define __XYZ_MPOOL_H__

struct xyz_mpool_t {                                                                                             
    char label[32];                                                                                              
    int data_size;                                                                                               

    char *free_list;                                                                                             
    int free_count;                                                                                              

    char *block_list;                                                                                            
    int block_count;                                                                                             
};     

struct xyz_mpool_t *xyz_mpool_create(char *label, int size);
void xyz_mpool_destroy(struct xyz_mpool_t *mp);
void *xyz_mpool_malloc(struct xyz_mpool_t *mp);
void xyz_mpool_free(struct xyz_mpool_t *mp, void *p);
void xyz_mpool_stat(struct xyz_mpool_t *mp, int v);

#endif // __XYZ_MPOOL_H__
