
#ifndef __XYZ_NETMODEL_H__
#define __XYZ_NETMODEL_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <errno.h>
#include <fcntl.h>

#include <pthread.h>

typedef int (*xyz_netmodel_func)(int fd);

int xyz_netmodel_perfork(int sockfd, int count, xyz_netmodel_func func);
int xyz_netmodel_forkpool(int sockfd, int count, xyz_netmodel_func func);
int xyz_netmodel_perthread(int sockfd, int count, xyz_netmodel_func func);
int xyz_netmodel_threadpool(int sockfd, int count, xyz_netmodel_func func); 

#endif // __XYZ_NETMODEL_H__

