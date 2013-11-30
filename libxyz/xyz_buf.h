
#ifndef __XYZ_BUF_H__
#define __XYZ_BUF_H__

struct xyz_buf_t
{
	char label[32];
	int size;
	char *data;
	int len;
};

struct xyz_buf_t *xyz_buf_create(char *label, int size);
void xyz_buf_clear(struct xyz_buf_t *buf);
void xyz_buf_destroy(struct xyz_buf_t *buf);
int xyz_buf_add(struct xyz_buf_t *buf, char *data, int len);
int xyz_buf_get(struct xyz_buf_t *buf, char *data, int len);
int xyz_buf_peek(struct xyz_buf_t *buf, char *data, int len);
int xyz_buf_sprintf(struct xyz_buf_t *buf, char *fmt, ...);
int xyz_buf_read(struct xyz_buf_t *buf, int fd);
int xyz_buf_write(struct xyz_buf_t *buf, int fd);	
char *xyz_buf_data(struct xyz_buf_t *buf);
int xyz_buf_length(struct xyz_buf_t *buf);
void xyz_buf_stat(struct xyz_buf_t *buf);

int xyz_buf_getline(struct xyz_buf_t *buf, char *data, int len);
int xyz_buf_getword(struct xyz_buf_t *buf, char *data, int len);

#endif // __XYZ_BUF_H__

