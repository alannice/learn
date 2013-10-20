
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "conf.h"

FILE *conf_open(char *file)
{
	FILE *fp;
	char filepath[256];

	if(file == NULL) {
		return NULL;
	}

	if(file[0] == '/' || file[0] == '.') {
		printf("1 config : %s\n", file);
		fp = fopen(file, "r");
		if(fp == NULL) {
			return NULL;
		}
		return fp;
	}

	bzero(filepath, sizeof(filepath));
	snprintf(filepath, sizeof(filepath)-1, "/etc/%s", file);
	if(access(filepath, R_OK) == 0) {
		printf("2 config : %s\n", filepath);
		fp = fopen(filepath, "r");
		if(fp == NULL) {
			return NULL;
		}
		return fp;
	}

	bzero(filepath, sizeof(filepath));
	snprintf(filepath, sizeof(filepath)-1, "/usr/local/etc/%s", file);
	if(access(filepath, R_OK) == 0) {
		printf("3 config : %s\n", filepath);
		fp = fopen(filepath, "r");
		if(fp == NULL) {
			return NULL;
		}
		return fp;
	}

	bzero(filepath, sizeof(filepath));
	getcwd(filepath, sizeof(filepath)-1);
	strncat(filepath, "/", sizeof(filepath)-strlen(filepath)-1);
	strncat(filepath, file, sizeof(filepath)-strlen(filepath)-1);
	if(access(filepath, R_OK) == 0) {
		printf("4 config : %s\n", filepath);
		fp = fopen(filepath, "r");
		if(fp == NULL) {
			return NULL;
		}
		return fp;
	}

	printf("5 not find config : %s\n", filepath);

	return NULL;
}

char *conf_parse(char *line)
{
	char *p;

	if(line == NULL || strlen(line) == 0) {
		return NULL;
	}

	p = strchr(line, '#');
	if(p) {
		*p = '\0';
	}

	p = strchr(line, '=');
	if(p == NULL) {
		return NULL;
	}

	*p = '\0';

	if(strlen(line) < 3 && strlen(p+1) < 3) {
		return NULL;
	}

	return p+1;
}

int conf_trim(char *line)
{
	char *d,*s;

	if(line == NULL || strlen(line) == 0) {
		return -1;
	}

	d = s = line;
	while(*s) {
		if(*s == ' ' || *s == '\t' || *s == '\r' || *s == '\n') {
			s++;
			continue;
		}
		*d = *s;
		s++;
		d++;
	}
	*(d++) = '\0';

	if(strlen(line) == 0) {
		return -1;
	}

	return 0;
}

struct conf_t *conf_append(struct conf_t *conf, char *key, char *value)
{
	if(key == NULL || strlen(key) == 0 || value == NULL || strlen(value) == 0) {
		return conf;
	}

	struct conf_t *tmpconf = malloc(sizeof(struct conf_t));
	if(tmpconf == NULL) {
		return conf;
	}

	tmpconf->key = strdup(key);
	if(tmpconf->key == NULL) {
		free(tmpconf);
		return conf;
	}
	tmpconf->value = strdup(value);
	if(tmpconf->value == NULL) {
		free(tmpconf->key);
		free(tmpconf);
		return conf;
	}

	tmpconf->next = conf;

	return tmpconf;
}

struct conf_t *conf_read(FILE *fp)
{
	char line[512];
	char *value;

	struct conf_t *conf = NULL;

	while(! feof(fp)) {
		bzero(line, sizeof(line));
		if(fgets(line, sizeof(line)-1, fp) == NULL) {
			continue;
		}

		if(conf_trim(line) == -1) {
			continue;
		}
		value = conf_parse(line);
		if(value == NULL) {
			continue;
		}
		conf = conf_append(conf, line, value);
	}

	return conf;
}

struct conf_t *conf_load(char *file)
{
	FILE *fp;

	fp = conf_open(file);
	if(fp == NULL) {
		return NULL;
	}

	return conf_read(fp);
}

void conf_destroy(struct conf_t *conf)
{
	struct conf_t *tmpconf;

	while(conf) {
		tmpconf = conf;
		conf = conf->next;

		free(tmpconf->key);
		free(tmpconf->value);
		free(tmpconf);
	}

	return;
}

const char *conf_string(struct conf_t *conf, char *key)
{
	while(conf) {
		if(strcmp(conf->key, key) == 0) {
			return conf->value;
		}
		conf = conf->next;
	}

	return NULL;
}

int conf_number(struct conf_t *conf, char *key)
{
	while(conf) {
		if(strcmp(conf->key, key) == 0) {
			return atoi(conf->value);
		}
		conf = conf->next;
	}

	return -999;
}

void conf_stat(struct conf_t *conf)
{
	printf("------ conf stat ------\n");

	while(conf) {
		printf("key: %-16s , value: %-16s\n", conf->key, conf->value);
		conf = conf->next;
	}

	return;
}

/////////////////////////////////////////////////
/*
int main(int argc, char *argv[])
{
    struct conf_t *conf=NULL;
	if((conf=conf_load(argv[1])) == NULL) {
		printf("conf load error\n");
		return 0;
	}
	conf_stat(conf);

	conf=conf_append(conf, "hello", "world");
	conf=conf_append(conf, "smail", "333");
	conf_stat(conf);

	conf_destroy(conf);

	return 0;
}
*/
