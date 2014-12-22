
#ifndef __XYZ_CONF_H__
#define __XYZ_CONF_H__

struct xyz_conf_t
{
	char *key;
	char *value;
	struct xyz_conf_t *next;
};

struct xyz_conf_t *xyz_conf_load(char *file);
void xyz_conf_destroy(struct xyz_conf_t *conf);
const char *xyz_conf_string(struct xyz_conf_t *conf, char *key);
void xyz_conf_stat(struct xyz_conf_t *conf);

#endif // __XYZ_CONF_H__
