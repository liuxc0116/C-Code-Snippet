#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <signal.h>
#include <time.h>
#include <syslog.h>

/*
1. 让程序在后台执行。方法是调用fork（）产生一个子进程，然后使父进程退出。
2. 调用setsid() 创建一个新对话期。
   控制终端、登录会话和进程组通常是从父进程继承下来的，守护进程要摆脱它们，不受它们的影响
   方法是调用setsid() 使进程成为一个会话组长。
   setsid() 调用成功后，进程成为新的会话组长和进程组长，并与原来的登录会话、进程组和控制终端脱离。
3. 禁止进程重新打开控制终端。经过以上步骤，进程已经成为一个无终端的会话组长，但是它可以重新申请打开一个终端。
   为了避免这种情况发生，可以通过使进程不再是会话组长来实现。再一次通过fork() 创建新的子进程，使调用fork的进程退出。
4. 关闭不再需要的文件描述符。
   子进程从父进程继承打开的文件描述符。
   如不关闭，将会浪费系统资源，造成进程所在的文件系统无法卸下以及引起无法预料的错误。
   首先获得最高文件描述符值，然后用一个循环程序，关闭0到最高文件描述符值的所有文件描述符。
5. 将当前目录更改为根目录。
6. 子进程从父进程继承的文件创建屏蔽字可能会拒绝某些许可权。为防止这一点，使用unmask(0) 将屏蔽字清零。
7. 处理SIGCHLD信号。对于服务器进程，在请求到来时往往生成子进程处理请求。
   如果父进程不等待子进程结束，子进程将成为僵尸进程（zombie），从而占用系统资源。
   如果父进程等待子进程结束，将增加父进程的负担，影响服务器进程的并发性能。
   在Linux下可以简单地将SIGCHLD信号的操作设为SIG_IGN。
   这样，子进程结束时不会产生僵尸进程。
*/

int init_daemon()
{
    signal(SIGTTOU,SIG_IGN);
    signal(SIGTTIN,SIG_IGN);
    signal(SIGTSTP,SIG_IGN);
    signal(SIGHUP,SIG_IGN);

    pid_t spid;
    spid = fork();
    if (spid > 0) {
        printf("father exit\n");
        //结束父进程，使得子进程成为后台进程
        exit(0);
    } else if (spid < 0){
        perror("fork error");
        exit(1);
    }

    //建立一个新的进程组,在这个新的进程组中,子进程成为这个进程组的首进程,以使该进程脱离所有终端
    setsid();

    //再次新建一个子进程，退出父进程，保证该进程不是进程组长，同时让该进程无法再打开一个新的终端
    spid = fork();
    if (spid > 0) {
        printf("son exit\n");
        exit(0);
    } else if (spid < 0){
        perror("fork error");
        exit(1);
    }

    //关闭所有从父进程继承的不再需要的文件描述符
    //for(i = 0;i < NOFILE; close(i++));

    //改变工作目录，使得进程不与任何文件系统联系
    chdir("/");

    //将文件当时创建屏蔽字设置为0
    umask(0);

    //忽略SIGCHLD信号
    signal(SIGCHLD, SIG_IGN);

    return 0;
}


int main(int argc, char *argv[])
{
    time_t now;
    init_daemon();
    while(1) {
        sleep(5);
        time(&now);
        syslog(LOG_USER|LOG_INFO, "SystemTime: \t%s\t\t\n", ctime(&now));
    }
    return 0;
}
