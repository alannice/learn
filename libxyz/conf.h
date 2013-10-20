
#ifndef __CONF_H__
#define __CONF_H__

struct conf_t
{
	char *key;
	char *value;
	struct conf_t *next;
};

struct conf_t *conf_load(char *file);
void conf_destroy(struct conf_t *conf);
const char *conf_string(struct conf_t *conf, char *key);
int conf_number(struct conf_t *conf, char *key);
void conf_stat(struct conf_t *conf);

#endif // __CONF_H__
