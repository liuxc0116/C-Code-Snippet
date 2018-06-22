#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <pthread.h>
#include "thread_pool.h"

int log_level = 0;

void *thread_handler(void *data)
{
    printf("thread 0x%x, running\n", pthread_self());
    sleep(10);
    printf("thread 0x%x, die\n", pthread_self());
}

int main(int argc, char *argv[])
{
    pool_init(10);
    int i;
    for (i = 0; i < 5; i++) {
        pool_add_worker(thread_handler, NULL);
        sleep(2);
    }

    while(1) {
        sleep(2);
    };
    //pool_destroy();
    return 0;
}
