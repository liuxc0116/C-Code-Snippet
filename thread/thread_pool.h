#ifndef THREAD_POOL_H
#define THREAD_POOL_H

#include <pthread.h>
/*
*线程池里所有运行和等待的任务都是一个CThread_worker
*由于所有任务都在链表里，所以是一个链表结构
*/

typedef void *(*run_process)(void *arg);
typedef struct worker
{
    /*回调函数，任务运行时会调用此函数，注意也可声明成其它形式*/
    run_process process;
    /*回调函数的参数*/
    void *arg;
    struct worker *next;

} CThread_worker;

/*线程池结构*/
typedef struct
{
    pthread_mutex_t queue_lock;
    pthread_cond_t queue_ready;

    /*链表结构，线程池中所有等待任务*/
    CThread_worker *queue_head;

    /*是否销毁线程池*/
    int shutdown;
    pthread_t *threadid;
    /*线程池中允许的活动线程数目*/
    int max_thread_num;
    /*当前等待队列的任务数目*/
    int cur_queue_size;

} CThread_pool;

void pool_init(int max_thread_num);
int pool_destroy();
int pool_add_worker(run_process process, void *arg);
#endif

