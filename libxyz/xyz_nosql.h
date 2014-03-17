
#ifndef __XYZ_NOSQL_H__
#define __XYZ_NOSQL_H__

struct xyz_mc_t;

struct xyz_mc_t *xyz_mc_connect(char *ip, int port);
void xyz_mc_destroy(struct xyz_mc_t *mc);
int xyz_mc_get(struct xyz_mc_t *mc, char *key, char *value, size_t vlen);
int xyz_mc_set(struct xyz_mc_t *mc, int timeout, char *key, char *value, size_t vlen);

#endif // __XYZ_NOSQL_H__

