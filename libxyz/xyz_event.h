
#ifndef __XYZ_EVENT_H__
#define __XYZ_EVENT_H__

typedef int (*xyz_ev_func)(int fd, void *arg);
typedef int (*xyz_ev_call)(void);

#define XYZ_EVTYPE_RD 1
#define XYZ_EVTYPE_WT 2

#define XYZ_EVENT_FDMAX 8 

struct xyz_event_node_t                                                                                          
{                                                                                                                
    int fd;                                                                                                      
    xyz_ev_func rdfunc;                                                                                          
    xyz_ev_func wtfunc;                                                                                          
    void *arg;                                                                                                   
};                                                                                                               
                                                                                                                 
struct xyz_event_t                                                                                               
{                                                                                                                
    int stop;                                                                                                    
    int usec;                                                                                                    

    struct xyz_event_node_t array[XYZ_EVENT_FDMAX];                                                              
    xyz_ev_call call;                                                                                            
};  

struct xyz_event_t *xyz_event_create();
int xyz_event_add(struct xyz_event_t *ev, int fd, int type, xyz_ev_func func, void *arg);
int xyz_event_del(struct xyz_event_t *ev, int fd, int type);
int xyz_event_run(struct xyz_event_t *ev);
void xyz_event_stop(struct xyz_event_t *ev);
void xyz_event_destroy(struct xyz_event_t *ev);
void xyz_event_loop(struct xyz_event_t *ev);
void xyz_event_stat(struct xyz_event_t *ev);
int xyz_event_call(struct xyz_event_t *ev, xyz_ev_call call);

#endif  // __XYZ_EVENT_H__

