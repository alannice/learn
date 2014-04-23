
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/select.h>

#include "xyz_event.h"

struct xyz_event_t *xyz_event_create()
{
	int i;

	struct xyz_event_t *ev = malloc(sizeof(struct xyz_event_t));
	if(ev == NULL) {
		return NULL;
	}

	ev->maxfd = 0;
	ev->stop = 0;
	ev->usec = 200*1000;
	FD_ZERO(&(ev->rdset));
	FD_ZERO(&(ev->wtset));

	for(i=0; i<XYZ_EVENT_FDMAX; i++) {
		ev->array[i].fd = -1;
		ev->array[i].rdtype = 0;
		ev->array[i].wttype = 0;
	}
	ev->call = NULL;

	return ev;
}

int xyz_event_call(struct xyz_event_t *ev, xyz_ev_call call)
{
    if(ev == NULL) {
        return -1;
    }

	ev->call = call;

	return 0;
}

int xyz_event_add(struct xyz_event_t *ev, int fd, int type, xyz_ev_func func, void *arg)
{
	int i;

	if(ev == NULL || fd < 0 || func == NULL) {
		return -1;
	}
    if(type != XYZ_EVTYPE_RD && type != XYZ_EVTYPE_WT) {
        return -1;
    }

	for(i=0; i<XYZ_EVENT_FDMAX; i++) {
		if(ev->array[i].fd == fd) {
			if(type == XYZ_EVTYPE_RD) {
				ev->array[i].rdtype = 1;
				ev->array[i].rdfunc = func;
			} else if(type == XYZ_EVTYPE_WT) {
				ev->array[i].wttype = 1;
				ev->array[i].wtfunc = func;
			} else {
				return -1;
			}
			ev->array[i].fd = fd;
			ev->array[i].arg = arg;

			return 0;
		}
	}

	for(i=0; i<XYZ_EVENT_FDMAX; i++) {
		if(ev->array[i].fd < 0) {
			if(type == XYZ_EVTYPE_RD) {
				ev->array[i].rdtype = 1;
				ev->array[i].rdfunc = func;
			} else if(type == XYZ_EVTYPE_WT) {
				ev->array[i].wttype = 1;
				ev->array[i].wtfunc = func;
			} else {
				return -1;
			}
			ev->array[i].fd = fd;
			ev->array[i].arg = arg;

			return 0;
		}
	}

	return -1;
}

int xyz_event_del(struct xyz_event_t *ev, int fd, int type)
{
	int i;

    if(ev == NULL || fd < 0) {
        return -1;
    }
    if(type != XYZ_EVTYPE_RD && type != XYZ_EVTYPE_WT) {
        return -1;
    }

	for(i=0; i<XYZ_EVENT_FDMAX; i++) {
		if(ev->array[i].fd == fd) {
			if(type == XYZ_EVTYPE_RD) {
				ev->array[i].rdtype = 0;
			} else if(type == XYZ_EVTYPE_WT) {
				ev->array[i].wttype = 0;
			} else {
				return -1;
			}
			if(ev->array[i].rdtype == 0 && ev->array[i].wttype == 0) {
				ev->array[i].fd = -1;
			}
			return 0;
		}
	}

	return -1;
}

int xyz_event_run(struct xyz_event_t *ev)
{
	int num, i;
	struct timeval tv;

    if(ev == NULL) {
        return -1;
    }

	tv.tv_sec = 0;
	tv.tv_usec = ev->usec;

	// fill rdset/wtset
	FD_ZERO(&(ev->rdset));
	FD_ZERO(&(ev->wtset));
	for(i=0; i<XYZ_EVENT_FDMAX; i++) {
		if(ev->array[i].fd < 0) {
			continue;
		}
		if(ev->array[i].rdtype) {
			FD_SET(ev->array[i].fd, &(ev->rdset));
		} 
		if(ev->array[i].wttype) {
			FD_SET(ev->array[i].fd, &(ev->wtset));
		}
		if(ev->array[i].fd > ev->maxfd) {
			ev->maxfd = ev->array[i].fd;
		}
	}

	num = select(ev->maxfd+1, &ev->rdset, &ev->wtset, NULL, &tv);
	if(num == 0) {
		return 0;
	} else if(num == -1) {
		if(errno == EINTR) {
			return 0;
		} else {
			return -1;
		}
	}

	// read/write
	for(i=0; i<XYZ_EVENT_FDMAX; i++) {
		if(ev->array[i].fd < 0) {
			continue;
		}
		if(ev->array[i].rdtype && FD_ISSET(ev->array[i].fd, &(ev->rdset))) {
			if(ev->array[i].rdfunc) {
				ev->array[i].rdfunc(ev->array[i].fd, ev->array[i].arg);
			}
		} 
		if(ev->array[i].wttype && FD_ISSET(ev->array[i].fd, &(ev->wtset))) {
			if(ev->array[i].wtfunc) {
				ev->array[i].wtfunc(ev->array[i].fd, ev->array[i].arg);
			}
		}
	}

	return num;
}

void xyz_event_stop(struct xyz_event_t *ev)
{
    if(ev) {
        ev->stop = 1;
    }

	return;
}

void xyz_event_destroy(struct xyz_event_t *ev)
{
	if(ev) {
		free(ev);
	}

	return;
}

void xyz_event_loop(struct xyz_event_t *ev)
{
    if(ev == NULL) {
        return;
    }

	ev->stop = 0;
	do {
		xyz_event_run(ev);

		if(ev->call) {
			ev->call();
		}
	} while(! ev->stop);

	return;
}

void xyz_event_stat(struct xyz_event_t *ev)
{
	int i;

    if(ev == NULL) {
        return;
    }

	printf("------ event stat ------\n");

	for(i=0; i<XYZ_EVENT_FDMAX; i++) {
		if(ev->array[i].fd < 0) {
			continue;
		}
		printf("fd:%d    ", ev->array[i].fd);
		if(ev->array[i].rdtype) {
			printf("read func : %p    ", ev->array[i].rdfunc);
		} 
		if(ev->array[i].wttype) {
			printf("write func : %p", ev->array[i].wtfunc);
		}
		printf("\n");
	}
}


//////////////////////////////////////////////////////////////////////////////

#if 0
int xyz_event_test(int fd, void *arg)
{
	struct xyz_event_t *ev = arg;

	char msg[128];
	bzero(msg, 128);
	read(fd, msg, 120);
	printf("%s\n", msg);
	if(strncasecmp(msg, "quit", 4) == 0) {
		xyz_event_stop(ev);
		printf("bye ......\r\n");
	}
	return 0;
}

int main(void)
{
	struct xyz_event_t *ev;
	ev = xyz_event_create();
	printf("echo server, input \"quit\" to exit\n");
	xyz_event_add(ev, 0, XYZ_EVTYPE_RD, xyz_event_test, ev);
	xyz_event_loop(ev);
    xyz_event_destroy(ev);
	return 0;
}
#endif 

