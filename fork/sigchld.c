#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>

void signal_handler(int signal_num)
{
    pid_t spid;
    int status;
    spid = wait(&status);
    printf("son(%d) exit with %d status code\n", spid, status);
    psignal(signal_num, "catch a signal");
}

int main(int argc, char *argv[])
{
    pid_t spid;

    spid = fork();
    if (spid == 0) {
        printf("this is a son(%d), after 10s, it's will be exit\n", getpid());
        sleep(10);
        exit(1);
    }

    signal(SIGCHLD, signal_handler);
    printf("this is a father, wait sons\n");

    while(1);

    return 0;
}
