
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>

#include <sys/select.h>

#include "event.h"

//typedef int (*ev_func)(int fd, void *arg);
//
//#define EVTYPE_RD 1
//#define EVTYPE_WT 2

struct event_node_t
{
	int fd;
	int type;

	ev_func func;
	void *arg;

	struct event_node_t *next;
};

struct event_t
{
	int maxfd;
	int stop;
	int usec;

	fd_set rdset;
	fd_set wtset;

	struct event_node_t *list;
	ev_call call;
};

struct event_t *event_create()
{
	struct event_t *ev = malloc(sizeof(struct event_t));
	if(ev == NULL) {
		return NULL;
	}

	ev->maxfd = 0;
	ev->stop = 0;
	ev->usec = 200*1000;
	FD_ZERO(&ev->rdset);
	FD_ZERO(&ev->wtset);

	ev->list = NULL;
	ev->call = NULL;

	return ev;
}

int event_call(struct event_t *ev, ev_call call)
{
	ev->call = call;

	return 0;
}

int event_add(struct event_t *ev, int fd, int type, ev_func func, void *arg)
{
	struct event_node_t *en = ev->list;
	while(en) {
		if(en->fd == fd && en->type == type) {
			return 0;
		}
		en = en->next;
	}

	en = malloc(sizeof(struct event_node_t));
	if(en == NULL) {
		return -1;
	}

	en->fd = fd;
	en->type = type;
	en->func = func;
	en->arg = arg;
	en->next = ev->list;

	ev->list = en;

	return 0;
}

int event_del(struct event_t *ev, int fd, int type)
{
	struct event_node_t *en = ev->list;
	struct event_node_t *prov = NULL;

	while(en) {
		if(en->fd == fd && en->type == type) {
			if(prov) {
				prov->next = en->next;
			} else {
				ev->list = en->next;
			}
			free(en);

			return 0;
		}

		prov = en;
		en = en->next;
	}

	return -1;
}

int event_run(struct event_t *ev)
{
	int num;
	struct event_node_t *en;
	struct timeval tv;

	tv.tv_sec = 0;
	tv.tv_usec = ev->usec;

	// fill rdset/wtset
	FD_ZERO(&ev->rdset);
	FD_ZERO(&ev->wtset);
	en = ev->list;
	while(en) {
		if(en->type == EVTYPE_RD) {
			FD_SET(en->fd, &ev->rdset);
			if(en->fd > ev->maxfd) {
				ev->maxfd = en->fd;
			}
		} else if(en->type == EVTYPE_WT) {
			FD_SET(en->fd, &ev->wtset);
			if(en->fd > ev->maxfd) {
				ev->maxfd = en->fd;
			}
		}

		en = en->next;
	}

	num = select(ev->maxfd+1, &ev->rdset, &ev->wtset, NULL, &tv);
	if(num == 0) {
		return 0;
	} else if(num == -1) {
		if(errno == EINTR || errno == EAGAIN) {
			return 0;
		} else {
			return -1;
		}
	}

	// read/write
	en = ev->list;
	while(en) {
		if(en->type == EVTYPE_RD && FD_ISSET(en->fd, &ev->rdset)) {
			if(en->func) {
				en->func(en->fd, en->arg);
			}
		} else if(en->type == EVTYPE_WT && FD_ISSET(en->fd, &ev->wtset)) {
			if(en->func) {
				en->func(en->fd, en->arg);
			}
		}

		en = en->next;
	}

	return num;
}

void event_stop(struct event_t *ev)
{
	ev->stop = 1;

	return;
}

void event_loop(struct event_t *ev)
{
	ev->stop = 0;
/*
	while(! ev->stop) {
		event_run();
	}
*/
	do {
		event_run(ev);

		if(ev->call) {
			ev->call();
		}
	} while(! ev->stop);

	return;
}

void event_stat(struct event_t *ev)
{
	struct event_node_t *en;

	printf("------ event stat ------\n");

	en = ev->list;
	while(en) {
		if(en->type == EVTYPE_RD) {
			printf("read : %d\n", en->fd);
			printf("func : %p\n", en->func);
		} else if(en->type == EVTYPE_WT) {
			printf("write : %d\n", en->fd);
			printf("func : %p\n", en->func);
		}

		en = en->next;
	}
}


/////////////////////////////////////////////////
/*
int event_test(int fd, void *arg)
{
	struct event_t *ev = arg;

	char msg[128];
	bzero(msg, 128);
	read(fd, msg, 120);
	printf("%s\n", msg);
	if(strcasecmp(msg, "quit") >= 0) {
		event_stop(ev);
	}
	return 0;
}

int main(void)
{
	struct event_t *ev;
	ev = event_create();
	event_add(ev, 0, EVTYPE_RD, event_test, ev);
	event_loop(ev);
	return 0;
}
*/
