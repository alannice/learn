
#ifndef __XYZ_EVENT2_H__
#define __XYZ_EVENT2_H__

#define XYZ_EVENT2_RD 1
#define XYZ_EVENT2_WT 2

typedef int (*xyz_ev2_func)(int fd, void *arg);
typedef int (*xyz_ev2_call)(void);

struct xyz_event2_t *xyz_event2_create();
int xyz_event2_add(struct xyz_event2_t *ev, int fd, int type, xyz_ev2_func func, void *arg);
int xyz_event2_del(struct xyz_event2_t *ev, int fd, int type);
int xyz_event2_run(struct xyz_event2_t *ev);
void xyz_event2_stop(struct xyz_event2_t *ev);
void xyz_event2_destroy(struct xyz_event2_t *ev);
void xyz_event2_loop(struct xyz_event2_t *ev);
void xyz_event2_stat(struct xyz_event2_t *ev);
int xyz_event2_call(struct xyz_event2_t *ev, xyz_ev2_call call);

#endif // __XYZ_EVENT2_H__

