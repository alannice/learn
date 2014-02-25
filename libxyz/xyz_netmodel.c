
/*
 * 1, 迭代服务器: 接收到数据, 处理, 再收数据,再处理.
 * 2, 并发服务器: 接收到数据, fork新进程处理.
 * 3, 预先创建子进程: 每个子进程调用ACCEPT, 并使用文件锁保护ACCEPT.
 * 4, 预先创建子进程: 由父进程ACCEPT后,传给子进程处理.
 * 5, 并发服务器: 由父进程为每个请求创建一个线程.
 * 6, 预先创建线程: 每个线程ACCEPT, 以互斥锁保护ACCEPT.
 * 7, 预先创建线程: 主线程调用ACCEPT, 子线程接收并处理数据.
 */

#include "xyz_netmodel.h"

///-------------------------------------------

static int g_xyz_netmodel_num;

void xyz_netmodel_sigchld(int no) 
{
    pid_t pid;                                                                                                   
    int status;                                                                                                  

    while((pid = waitpid(-1, &status, WNOHANG)) != 0) 
        g_xyz_netmodel_num--;                                                           

    return;
}

///--------------------------------------------

/// 一个连接一个进程, 进程处理完这个连接后退出.
int xyz_netmodel_perfork(int listenfd, int count, xyz_netmodel_func func) 
{
    int connfd;
    struct sockaddr addr;
    socklen_t addrlen;

    signal(SIGCHLD, xyz_netmodel_sigchld);

    while(1) {
        addrlen = sizeof(struct sockaddr);
        connfd = accept(listenfd, &addr, &addrlen);  
        if(connfd < 0) {
            if(errno == EINTR) {
                continue;
            } else {
                return -1;
            }
        }

        if(g_xyz_netmodel_num > count) {
            close(connfd);
            continue;
        }

        switch(fork()) {
            case 0:
                close(listenfd);
                func(connfd);
                close(connfd);
                exit(0);
            case -1:
                return -1;
            default:
                close(connfd);
                g_xyz_netmodel_num++;
                break;
        }
    }

    return 0;
}

////-------------------------------------------------

int xyz_netmodel_tempfd()
{
    int lock_fd;
    char lock_file[1024];

    strcpy(lock_file, "/tmp/xyz_netmode_forkpool.XXXXXX");
    lock_fd = mkstemp(lock_file);
    if(lock_fd == -1) {
        return -1;
    }
    unlink(lock_file);
    
    return lock_fd;
}

int xyz_netmodel_lockfd(int fd)
{
    struct flock lock_it;
    lock_it.l_type = F_WRLCK;
    lock_it.l_whence = SEEK_SET;
    lock_it.l_start = 0;
    lock_it.l_len = 0;

    int rc;
    while((rc = fcntl(fd, F_SETLKW, &lock_it)) < 0) {
        if(errno == EINTR) {
            continue;
        } else {
            return -1;
        }
    }

    return 0;
}

int xyz_netmodel_unlockfd(int fd)
{
    struct flock unlock_it;
    unlock_it.l_type = F_UNLCK;
    unlock_it.l_whence = SEEK_SET;
    unlock_it.l_start = 0;
    unlock_it.l_len = 0;

    fcntl(fd, F_SETLKW, &unlock_it);
    
    return 0;
}

// 子进程通过文件锁实现对ACCEPT的互斥.
int xyz_netmodel_forkpool_child(int lock_fd, int listenfd, xyz_netmodel_func func)
{
    int ret;
    int connfd;
    struct sockaddr addr;
    socklen_t addrlen;

    while(1) {
        addrlen = sizeof(addr);

        ret = xyz_netmodel_lockfd(lock_fd);
        if(ret == -1) {
            continue;
        }

        connfd = accept(listenfd, &addr, &addrlen);

        xyz_netmodel_unlockfd(lock_fd);

        if(connfd < 0) {
            if(errno == EINTR) {
                continue;
            } else {
                return -1;
            }
        }

        func(connfd);
        close(connfd);
    }

    return 0;
}

// 进程池,子进程处理完连接后, 不退出, 继续处理下一个连接.
int xyz_netmodel_forkpool(int listenfd, int count, xyz_netmodel_func func)  
{
    signal(SIGCHLD, xyz_netmodel_sigchld);

    int lock_fd = xyz_netmodel_tempfd();
    if(lock_fd == -1) {
        return -1;
    }

    g_xyz_netmodel_num = 0;
    while(1) {
        if(g_xyz_netmodel_num <= count) {
            switch(fork()) {
                case 0:
                    xyz_netmodel_forkpool_child(lock_fd, listenfd, func);
                    exit(0);
                case -1:
                    return -1;
                default:
                    g_xyz_netmodel_num++;
                    break;
            }
        } else {
            sleep(1);
        }
    }

    close(lock_fd);

    return 0;
}

////-------------------------------------------------

struct xyz_netmodel_thread_args_t
{
    int fd;
    xyz_netmodel_func func;
};

void *xyz_netmodel_perthread_func(void *args)
{
    struct xyz_netmodel_thread_args_t *thread_args = args;
    
    pthread_detach(pthread_self());
    thread_args->func(thread_args->fd);
    close(thread_args->fd);
    
    return NULL;
}

/// 一个连接一个线程, 处理完一个连接后, 线程关闭.
int xyz_netmodel_perthread(int listenfd, int count, xyz_netmodel_func func)  
{
    struct sockaddr cliaddr;
    socklen_t addrlen;
    struct xyz_netmodel_thread_args_t thread_args;
    int connfd;
    pthread_t tid;

    while(1) {
        addrlen = sizeof(cliaddr);
        connfd = accept(listenfd, &cliaddr, &addrlen);
        if(connfd >= 0) {
            thread_args.fd = connfd;
            thread_args.func = func;
            pthread_create(&tid, NULL, &xyz_netmodel_perthread_func, (void*)&thread_args);
        } 
        if(connfd < 0) {
            if(errno == EINTR) {
                continue;
            } else {
                return -1;
            }
        }
    }

    return 0;
}

////-------------------------------------------------

struct xyz_netmodel_threadpool_t 
{
    pthread_t tid;
    xyz_netmodel_func func;
}; 

pthread_mutex_t xyz_netmodel_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t xyz_netmodel_cond = PTHREAD_COND_INITIALIZER;
int xyz_netmodel_connfd;

void *xyz_netmodel_threadpool_func(void *args) 
{
    int connfd;
    struct xyz_netmodel_threadpool_t *threadpool = args;

    pthread_detach(pthread_self());

    while(1) {
        pthread_mutex_lock(&xyz_netmodel_mutex);
        pthread_cond_wait(&xyz_netmodel_cond, &xyz_netmodel_mutex);
        connfd = xyz_netmodel_connfd;
        pthread_mutex_unlock(&xyz_netmodel_mutex);

        threadpool->func(connfd);
        close(connfd);
    }

    return NULL;
}

/// 线程池, 通过互斥锁,信号量唤醒一线程处理,处理完不关闭.
int xyz_netmodel_threadpool(int listenfd, int count, xyz_netmodel_func func)       
{
    int i=0;
    int connfd;
    struct sockaddr addr;
    socklen_t addrlen;

    struct xyz_netmodel_threadpool_t *threadpool = malloc(count*sizeof(struct xyz_netmodel_threadpool_t));

    for(i=0; i<count; i++) {
        threadpool[i].func = func;
        pthread_create(&(threadpool[i].tid), NULL, xyz_netmodel_threadpool_func, (void *)&threadpool[i]);
    }
    
    while(1) {
        addrlen = sizeof(addr);
        connfd = accept(listenfd, &addr, &addrlen);
        if(connfd < 0) {
            if(errno == EINTR) {
                continue;
            } else {
                return -1;
            }
        }

        pthread_mutex_lock(&xyz_netmodel_mutex);
        xyz_netmodel_connfd = connfd;
        pthread_cond_signal(&xyz_netmodel_cond);
        pthread_mutex_unlock(&xyz_netmodel_mutex);
    }

    return 0;
}

///----------------------------------------------------------

#if 0

// cc xyz_netmodel.c xyz_sock.c -lpthread

#include "xyz_sock.h"

int func(int fd)
{
    char buf[1024];

    while(1) {
        memset(buf, '\0', 1024);
        int n = read(fd, buf, 1000);
        if(n > 0) {
            printf("[%u] %s\n", getpid(), buf);
            if(buf[0] == 'Q') {
                printf("client quit\n");
                return 0;
            }
        } else {
            printf("read() return %d\n", n);
            return -1;
        }
    }

    return 0;
}

int main(int argc, char *argv[])
{
    printf("Usage:\n\t%s [1 2 3 4]\n\t1:perfork; 2:forkpool; 3:perthread; 4:threadpool\n", argv[0]); 

    int fd = xyz_sock_listen(NULL, 12345, XYZ_SOCKET_TCP); 

    if(argc != 2) {
        printf("argc mast 2 : %d\n", argc);
        exit(0);
    }
    
    switch(atoi(argv[1])) {
        case 1:
            xyz_netmodel_perfork(fd, 10, func);
            break;
        case 2:
            xyz_netmodel_forkpool(fd, 10, func);
            break;
        case 3:
            xyz_netmodel_perthread(fd, 10, func);
            break;
        case 4:
            xyz_netmodel_threadpool(fd, 10, func);
            break;
        default:
            printf("args error\n");
            break;
    }

    close(fd);

    return 0;
}

#endif 

