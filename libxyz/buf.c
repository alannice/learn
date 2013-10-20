
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#include <unistd.h>
#include <errno.h>

#include "buf.h"

struct buf_t *buf_create(char *label, int size)
{
	if(size <= 2) {
		return NULL;
	}

	struct buf_t *p = malloc(sizeof(struct buf_t));
	if(p == NULL) {
		return NULL;
	}

	p->data = malloc(size+1);
	if(p->data == NULL) {
		free(p);
		return NULL;
	}

	if(label != NULL || strlen(label) > 0) {
		strncpy(p->label, label, sizeof(p->label));
	}

	p->size = size;
	p->len = 0;

	return p;
}

void buf_clear(struct buf_t *buf)
{
	if(buf == NULL) {
		return;
	}

	buf->len = 0;
	*(buf->data) = '\0';

	return;
}

void buf_destroy(struct buf_t *buf)
{
	if(buf == NULL) {
		return;
	}

	free(buf->data);
	free(buf);

	return;
}

int buf_add(struct buf_t *buf, char *data, int len)
{
	if(buf == NULL || data == NULL || len <= 0) {
		return -1;
	}

	if(buf->len + len > buf->size) {
		return -1;
	}

	memcpy(buf->data+buf->len, data, len);
	buf->len+=len;
	*(buf->data+buf->len) = '\0';

	return 0;
}

int buf_get(struct buf_t *buf, char *data, int len)
{
	if(buf == NULL || data == NULL || len <= 0) {
		return -1;
	}

	int l = buf->len >= len ? len : buf->len;

	memcpy(buf->data, data, l);

	buf->len -= l;
	if(buf->len) {
		memmove(buf->data+l, buf->data, buf->len);
	}
	*(buf->data+buf->len) = '\0';

	return l;
}

int buf_peek(struct buf_t *buf, char *data, int len)
{
	if(buf == NULL || data == NULL || len <= 0) {
		return -1;
	}

	int l = buf->len >= len ? len : buf->len;

	memcpy(buf->data, data, l);

	return l;
}

int buf_read(struct buf_t *buf, int fd)
{
	int n;

	if(buf->len == buf->size) {
		return 0;
	}

	n = read(fd, buf->data+buf->len, buf->size-buf->len);
	if(n < 0) {
		if(errno == EAGAIN || errno == EINTR || errno == EWOULDBLOCK) {
			return 0;
		} else {
			return -1;
		}
	} else if(n == 0) {
		return -1;
	}

	buf->len += n;
	*(buf->data+buf->len) = '\0';

	return n;
}

int buf_write(struct buf_t *buf, int fd)
{
	int i, n, m = 0;

	for(i=0; i<6 && buf->len > m; i++) {
		n = write(fd, buf->data+m, buf->len-m);
		if(n < 0) {
			if(errno == EAGAIN || errno == EINTR || errno == EWOULDBLOCK) {
				break;
			} 
			return -1;
		} else if(n == 0) {
			return -1;
		} 

		m+=n;
	} 

	memmove(buf->data+m, buf->data, buf->len-m);
	buf->len -= m;
	*(buf->data+buf->len) = '\0';

	return m;
}

int buf_sprintf(struct buf_t *buf, char *fmt, ...)
{
	va_list vl;
	int rv;

	if(buf == NULL || fmt == NULL) {
		return -1;
	}

	va_start(vl, fmt);
	rv = vsnprintf(buf->data+buf->len, buf->size-buf->len, fmt, vl);
	va_end(vl);
	if(rv > 0) {
		buf->len+=rv;
	}

	return 0;
}

char *buf_data(struct buf_t *buf)
{
	if(buf == NULL) {
		return NULL;
	}

	return buf->data;
}

int buf_length(struct buf_t *buf)
{
	if(buf == NULL) {
		return -1;
	}

	return buf->len;
}

void buf_stat(struct buf_t *buf)
{
	printf("------ buf stat ------\n");
	printf("*** %s ***\n", buf->label);
	printf("size : %d\n", buf->size);
	// printf("data : %s\n", buf->data);
	printf("len : %d\n", buf->len);

	return;
}

//-----------------------------------------------

int buf_getline(struct buf_t *buf, char *data, int len)
{
	char *pos;

	if(buf == NULL || data == NULL || len <= 0) {
		return -1;
	}

	pos = strchr(buf->data, '\n');
	if(pos == NULL) {
		return 0;
	}

	if(pos - buf->data > len) {
		return -1;
	}

	*pos = '\0';
	if(*(pos-1) == '\r') {
		*(pos-1) = '\0';
	}

	strncpy(data, buf->data, len);

	buf->len -= ((pos+1)-buf->data);
	memmove(buf->data, pos+1, buf->len);
	*(buf->data+buf->len) = '\0';

	return strlen(data);
}

int buf_getword(struct buf_t *buf, char *data, int len)
{
	int pos = 0;
	int skip = 0;

	if(buf == NULL || data == NULL || len <= 0) {
		return -1;
	}

	for(pos=0; pos<=len; pos++) {
		if(buf->data[pos] == '\r' || buf->data[pos] == '\n') {
			break;
		}
		if(buf->data[pos] == ' ' || buf->data[pos] == '\t') {
			skip++;
			break;
		}
	}

	if(pos > len) {
		return -1;
	}

	strncpy(data, buf->data, pos);

	buf->len -= (pos+skip);
	memmove(buf->data, buf->data+pos+skip, buf->len);
	*(buf->data+buf->len) = '\0';

	return pos;
}

/////////////////////////////////////////////////
/*
int main(void)
{
	struct buf_t *buf;

	buf = buf_create("test buf", 64);

	buf_add(buf, "hello\n", 6);
	buf_sprintf(buf, "world\n");
	buf_sprintf(buf, "a %s b\n", "cc");
	buf_write(buf, 1);

	printf("add hello, world\n");
	buf_add(buf, "hello", 5);
	buf_add(buf, "world", 5);

	buf_stat(buf);

	printf("add a string\n");
	buf_add(buf, "aa\nbb\ncc\r\ndd\r\n", 14);
	buf_stat(buf);

	printf("get a line\n");
	char arr[128];
	bzero(arr, 100);
	buf_getline(buf, arr, 120);
	printf("line:%s\n", arr);
	buf_stat(buf);

	printf("get a world\n");
	bzero(arr, 100);
	buf_getword(buf, arr, 120);
	printf("word:%s\n", arr);
	buf_stat(buf);

	printf("buf write\n");
	buf_write(buf, STDOUT_FILENO);
	buf_stat(buf);

	return 0;
}
*/
