
#ifndef __EVENT_H__
#define __EVENT_H__

typedef int (*ev_func)(int fd, void *arg);
typedef int (*ev_call)(void);

#define EVTYPE_RD 1
#define EVTYPE_WT 2

struct event_t *event_create();
int event_add(struct event_t *ev, int fd, int type, ev_func func, void *arg);
int event_del(struct event_t *ev, int fd, int type);
int event_run(struct event_t *ev);
void event_stop(struct event_t *ev);
void event_loop(struct event_t *ev);
void event_stat(struct event_t *ev);
int event_call(struct event_t *ev, ev_call call);

#endif  // __EVENT_H__

