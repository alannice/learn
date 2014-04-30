
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>

#include "xyz_event2.h"

#define XYZ_EVENT2_FDMAX 65535 

struct xyz_event2_node_t {
    int rdtype;
    int wttype;
    xyz_ev2_func rdfunc;
    xyz_ev2_func wtfunc;
    void *rdarg;
    void *wtarg;
};

struct xyz_event2_t {
    int evfd;
    struct xyz_event2_node_t *nodes;
    int isstop;
    xyz_ev2_call call;
};

#ifdef __linux__

#include <sys/epoll.h>

int __xyz_event2_init(struct xyz_event2_t *ev2)
{
    if(ev2 == NULL) {
        return -1;
    }

    ev2->evfd = epoll_create(XYZ_EVENT2_FDMAX);

    if(ev2->evfd == -1) {
        return -1;
    }

    return 0;
}

int __xyz_event2_add(struct xyz_event2_t *ev2, int fd, int type)
{
    struct epoll_event ev;

    if(ev2 == NULL || fd < 0 || fd > XYZ_EVENT2_FDMAX) {
        return -1;
    }

    ev.data.fd = fd;
    if(type == XYZ_EVENT2_RD) {
        ev.events = EPOLLIN;
        if(ev2->nodes[fd].wttype == 1) {
            ev.events |= EPOLLOUT;
            return epoll_ctl(ev2->evfd, EPOLL_CTL_MOD, fd, &ev);
        } else {
            return epoll_ctl(ev2->evfd, EPOLL_CTL_ADD, fd, &ev);
        }
    } 
    if(type == XYZ_EVENT2_WT) {
        ev.events = EPOLLOUT;
        if(ev2->nodes[fd].rdtype == 1) {
            ev.events |= EPOLLIN;
            return epoll_ctl(ev2->evfd, EPOLL_CTL_MOD, fd, &ev);
        } else {
            return epoll_ctl(ev2->evfd, EPOLL_CTL_ADD, fd, &ev);
        }
    } 

    return -1;
}

int __xyz_event2_del(struct xyz_event2_t *ev2, int fd, int type)
{
    struct epoll_event ev;

    if(ev2 == NULL || fd < 0 || fd > XYZ_EVENT2_FDMAX) {
        return -1;
    }

    ev.data.fd = fd;
    if(type == XYZ_EVENT2_RD) {
        if(ev2->nodes[fd].wttype == 1) {
            ev.events=EPOLLOUT;
            return epoll_ctl(ev2->evfd, EPOLL_CTL_MOD, fd, &ev);
        } else {
            return epoll_ctl(ev2->evfd, EPOLL_CTL_DEL, fd, &ev);
        }
    }
    if(type == XYZ_EVENT2_WT) {
        if(ev2->nodes[fd].rdtype == 1) {
            ev.events=EPOLLIN;
            return epoll_ctl(ev2->evfd, EPOLL_CTL_MOD, fd, &ev);
        } else {
            return epoll_ctl(ev2->evfd, EPOLL_CTL_DEL, fd, &ev);
        }
    } 

    return -1;
}

int __xyz_event2_run(struct xyz_event2_t *ev2)
{
    int i;
    struct epoll_event events[XYZ_EVENT2_FDMAX];

    if(ev2 == NULL) {
        return -1;
    }

    int nfds=epoll_wait(ev2->evfd, events, XYZ_EVENT2_FDMAX, 500);

    for(i=0; i < nfds; ++i)
    {
        if(events[i].events & EPOLLIN)  {
            ev2->nodes[events[i].data.fd].rdfunc(events[i].data.fd, ev2->nodes[events[i].data.fd].rdarg);
        } else if(events[i].events & EPOLLOUT) {
            ev2->nodes[events[i].data.fd].wtfunc(events[i].data.fd, ev2->nodes[events[i].data.fd].wtarg);
        }
    }

    return 0;
}

#endif // __linux__

/////////////////////////////////////////////////////////////////////////////

#ifdef __FreeBSD__

#include <sys/types.h>
#include <sys/event.h>
#include <sys/time.h>

int __xyz_event2_init(struct xyz_event2_t *ev2)
{
    if(ev2 == NULL) {
        return -1;
    }

    ev2->evfd = kqueue();

    if(ev2->evfd == -1) {
        return -1;
    }

    return 0;
}

int __xyz_event2_add(struct xyz_event2_t *ev2, int fd, int type)
{
    struct kevent event[1];

    if(ev2 == NULL || fd < 0) {
        return -1;
    }

    if(type == XYZ_EVENT2_RD) {
        EV_SET(&event[0], fd, EVFILT_READ, EV_ADD, 0, 0, NULL);
    } else if(type == XYZ_EVENT2_WT) {
        EV_SET(&event[0], fd, EVFILT_WRITE, EV_ADD, 0, 0, NULL);
    } else {
        return -1;
    }

    return kevent(ev2->evfd, event, 1, NULL, 0, NULL);
}

int __xyz_event2_del(struct xyz_event2_t *ev2, int fd, int type)
{
    struct kevent event[1];

    if(ev2 == NULL || fd < 0) {
        return -1;
    }

    if(type == XYZ_EVENT2_RD) {
        EV_SET(&event[0], fd, EVFILT_READ, EV_DELETE, 0, 0, NULL);
    } else if(type == XYZ_EVENT2_WT) {
        EV_SET(&event[0], fd, EVFILT_WRITE, EV_DELETE, 0, 0, NULL);
    } else {
        return -1;
    }

    return kevent(ev2->evfd, event, 1, NULL, 0, NULL);
}

int __xyz_event2_run(struct xyz_event2_t *ev2)
{
    int i;
    struct kevent events[XYZ_EVENT2_FDMAX];

    if(ev2 == NULL) {
        return -1;
    }

    int nfds = kevent(ev2->evfd, NULL, 0, events, XYZ_EVENT2_FDMAX, NULL);

    for(i=0; i < nfds; ++i)
    {
        if(events[i].filter & EVFILT_READ)  {
            ev2->nodes[events[i].ident].rdfunc(events[i].ident, ev2->nodes[events[i].ident].rdarg);
        } else if(events[i].filter & EVFILT_WRITE) {
            ev2->nodes[events[i].ident].wtfunc(events[i].ident, ev2->nodes[events[i].ident].wtarg);
        }
    }

    return 0;
}

#endif // __FreeBSD__

struct xyz_event2_t *xyz_event2_create()
{
    struct xyz_event2_t *event2 = malloc(sizeof(struct xyz_event2_t));
    if(event2 == NULL) {
        return NULL;
    }

    event2->nodes = calloc(XYZ_EVENT2_FDMAX+1, sizeof(struct xyz_event2_node_t));
    if(event2->nodes == NULL) {
        free(event2);
        return NULL;
    }

    event2->call = NULL;

    __xyz_event2_init(event2);

    return event2;
}

void xyz_event2_destroy(struct xyz_event2_t *ev2)
{
    if(ev2) {
        close(ev2->evfd);

        if(ev2->nodes) {
            free(ev2->nodes);
        }

        free(ev2);
    }

    return;
}

int xyz_event2_add(struct xyz_event2_t *ev2, int fd, int type, xyz_ev2_func func, void *arg)
{
    if(ev2 == NULL || fd < 0) {
        return -1;
    }

    if(type == XYZ_EVENT2_RD) {
        ev2->nodes[fd].rdtype = 1;
        ev2->nodes[fd].rdfunc = func;
        ev2->nodes[fd].rdarg = arg;
    } else if(type == XYZ_EVENT2_WT) {
        ev2->nodes[fd].wttype = 1;
        ev2->nodes[fd].wtfunc = func;
        ev2->nodes[fd].wtarg = arg;
    } else {
        return -1;
    }

    __xyz_event2_add(ev2, fd, type);

    return 0;
}

int xyz_event2_del(struct xyz_event2_t *ev2, int fd, int type)
{
    if(ev2 == NULL || fd < 0) {
        return -1;
    }

    if(type == XYZ_EVENT2_RD) {
        ev2->nodes[fd].rdtype = 0;
        ev2->nodes[fd].rdfunc = NULL;
        ev2->nodes[fd].rdarg = NULL;
    } else if(type == XYZ_EVENT2_WT) {
        ev2->nodes[fd].wttype = 0;
        ev2->nodes[fd].wtfunc = NULL;
        ev2->nodes[fd].wtarg = NULL;
    } else {
        return -1;
    }

    __xyz_event2_del(ev2, fd, type);

    return 0;
}

int xyz_event2_call(struct xyz_event2_t *ev2, xyz_ev2_call call)
{
    if(ev2) {
        ev2->call = call;
    }

    return 0;
}

void xyz_event2_loop(struct xyz_event2_t *ev2)
{
    if(ev2 == NULL) {
        return;
    }

    ev2->isstop = 0;

    while(ev2->isstop == 0) {
        __xyz_event2_run(ev2);

        if(ev2->call) {
            ev2->call();
        }
    }

    return;
}

void xyz_event2_stop(struct xyz_event2_t *ev2)
{
    if(ev2) {
        ev2->isstop = 1;
    }

    return;
}

void xyz_event2_stat(struct xyz_event2_t *ev2)
{
    int idx;

    if(ev2) {
        printf("------ event2 stat ------\n");
        printf("isstop : %d\n", ev2->isstop);
        printf("call : %p\n", ev2->call);

        for(idx=0; idx<XYZ_EVENT2_FDMAX; idx++) {
            if(ev2->nodes[idx].rdtype) {
                printf("\t[%d] rdfunc : %p\n", idx, ev2->nodes[idx].rdfunc);
                printf("\t[%d] rdarg : %p\n", idx, ev2->nodes[idx].rdarg);
            }

            if(ev2->nodes[idx].wttype) {
                printf("\t[%d] wtfunc : %p\n", idx, ev2->nodes[idx].wtfunc);
                printf("\t[%d] wtarg : %p\n", idx, ev2->nodes[idx].wtarg);
            }
        }
    }

    return;
}

//////////////////////////////////////////////////////////////////////////////

#if 0

int xyz_event2_test(int fd, void *arg)
{
    struct xyz_event2_t *ev = arg;

    char msg[128];
    bzero(msg, 128);
    read(fd, msg, 120);
    printf("%s\n", msg);
    if(strncasecmp(msg, "quit", 4) == 0) {
        xyz_event2_stop(ev);
        printf("bye ......\r\n");
    }

    return 0;
}

int main(void)
{
    struct xyz_event2_t *ev;
    ev = xyz_event2_create();

    printf("echo server, input \"quit\" to exit\n");

    xyz_event2_add(ev, 0, XYZ_EVENT2_RD, xyz_event2_test, ev);
    //xyz_event2_stat(ev);
    xyz_event2_loop(ev);
    xyz_event2_destroy(ev);

    return 0;
}


#endif 

