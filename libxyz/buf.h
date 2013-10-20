
#ifndef __BUF_H__
#define __BUF_H__

struct buf_t
{
	char label[32];
	int size;
	char *data;
	int len;
};

struct buf_t *buf_create(char *label, int size);
void buf_clear(struct buf_t *buf);
void buf_destroy(struct buf_t *buf);
int buf_add(struct buf_t *buf, char *data, int len);
int buf_get(struct buf_t *buf, char *data, int len);
int buf_peek(struct buf_t *buf, char *data, int len);
int buf_sprintf(struct buf_t *buf, char *fmt, ...);
int buf_read(struct buf_t *buf, int fd);
int buf_write(struct buf_t *buf, int fd);	
char *buf_data(struct buf_t *buf);
int buf_length(struct buf_t *buf);
void buf_stat(struct buf_t *buf);

int buf_getline(struct buf_t *buf, char *data, int len);
int buf_getword(struct buf_t *buf, char *data, int len);

#endif // __BUF_H__

