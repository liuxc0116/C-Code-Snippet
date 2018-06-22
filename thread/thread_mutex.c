#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <pthread.h>
#include <assert.h>

//所有线程中init_routine只执行一次
//pthread_once_t once = PTHREAD_ONCE_INIT
//int pthread_once(pthread_once_t *once_control, void (*init_routine)(void));

//线程私有存储区
//int pthread_key_create(pthread_key_t *key, void (*destructor)(void*));
//int pthread_key_delete(pthread_key_t key);
//int pthread_setspecific(pthread_key_t key, const void *value);
//void *pthread_getspecific(pthread_key_t key);

/*
锁时输出
179642368 count is 2
180178944 count is 2
179642368 count is 3
180178944 count is 4
179642368 count is 5
180178944 count is 6
179642368 count is 7
180178944 count is 8
179642368 count is 9
180178944 count is 10
179642368 count is 11
180178944 count is 12
179642368 count is 13
180178944 count is 14
179642368 count is 15
180178944 count is 16
179642368 count is 17
180178944 count is 18
179642368 count is 19
180178944 count is 20
179642368 count is 20
180178944 count is 20
thread exit status 20
*/

/*
加锁输出
246468608 count is 1
247005184 count is 2
246468608 count is 3
247005184 count is 4
246468608 count is 5
247005184 count is 6
246468608 count is 7
247005184 count is 8
246468608 count is 9
247005184 count is 10
246468608 count is 11
247005184 count is 12
246468608 count is 13
247005184 count is 14
246468608 count is 15
247005184 count is 16
246468608 count is 17
247005184 count is 18
246468608 count is 19
246468608 count is 19
247005184 count is 20
247005184 count is 20
thread exit status 20
*/

/*
int pthread_mutex_init(pthread_mutex_t *mutex, const pthread_mutexattr_t *attr);
int pthread_mutex_destroy(pthread_mutex_t *mutex);
int pthread_mutex_trylock(pthread_mutex_t *mutex);
int pthread_mutex_lock(pthread_mutex_t *mutex);
int pthread_mutex_unlock(pthread_mutex_t *mutex);

int pthread_mutexattr_init(pthread_mutexattr_t *attr);
int pthread_mutexattr_destroy(pthread_mutexattr_t *attr);
int pthread_mutexattr_setprioceiling(pthread_mutexattr_t *attr, int prioceiling);
int pthread_mutexattr_getprioceiling(pthread_mutexattr_t *attr, int *prioceiling);
int pthread_mutexattr_setprotocol(pthread_mutexattr_t *attr, int protocol);
int pthread_mutexattr_getprotocol(pthread_mutexattr_t *attr, int *protocol);
int pthread_mutexattr_settype(pthread_mutexattr_t *attr, int type);
int pthread_mutexattr_gettype(pthread_mutexattr_t *attr, int *type);
*/

int count = 0;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

void *doit(void *data)
{
    int i;
    for(i = 0; i < 10; i++) {
        //pthread_mutex_lock(&mutex);
        count++;
        printf("%u count is %d\n", (unsigned int)pthread_self(), count);
        //pthread_mutex_unlock(&mutex);
    }

    printf("%u count is %d\n", (unsigned int)pthread_self(), count);
    pthread_exit(&count);
}

int main(int argc, char *argv[])
{
    pthread_t thread1, thread2;

    //pthread_attr_t attr;
    if (pthread_create(&thread1, NULL, &doit, NULL) != 0) {
        perror("create thread error");
    }

    //pthread_detach(&thread1);

    if (pthread_create(&thread1, NULL, &doit, NULL) != 0) {
        perror("create thread error");
    }

    void *status;
    //pthread_join(&thread1, NULL);
    pthread_join(thread1, &status);
    pthread_join(thread1, &status);
    printf("thread exit status %d\n", *((int *)status));
    return 0;
}
