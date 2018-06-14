#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <sys/types.h>
#include <errno.h>
#include <register_signal.h>

void trigger_signal(int signal_num, int count)
{
    int i;
    for(i = 0; i < count; i++) {
        sleep(1);
        psignal(signal_num, "send signal");
        kill(getpid(), signal_num);
    }
}

void signal_handler(int signal_num)
{
    //在早期的linux系统中，信号被交付后, 系统自动的重置handler为默认动作
    //为了使信号在handler处理期间, 仍能对后继信号做出反应, 往往在handler的第一条语句再次调用 signal
    //signal(signum, signal_handler);
    psignal(signal_num, "catch a signal");
}

void sigint_handler(int signal_num)
{
    //必须再次注册， 否则当二次^C,程序会退出，WHY？
    register_signal_with_attr(signal_num, sigint_handler, SA_RESTART);
    psignal(signal_num, "catch a signal");
}

/*
 * 输出结果如下：
 * register a signal(2) use sigaction
 * ^Ccatch a signal: Interrupt
 * read error: Interrupted system call
 * read -1 bytes
 * buf
 */

void read_problem()
{
    char buf[100];
    int signal_num = SIGINT, ret;
    register_signal_use_sigaction(signal_num, signal_handler);

    bzero(buf, 100);
    ret = read(0, buf, 100);
    if (ret == -1) {
        perror("read error");
    }

    printf("read %d bytes\n", ret);
    printf("buf %s\n", buf);
}

/*
 *register a signal(2) use sigaction
 * ^Ccatch a signal: Interrupt
 * hello
 * read 6 bytes
 * buf hello
 */

void read_problem_resolve1()
{
    char buf[100];
    int signal_num = SIGINT, ret;
    register_signal_with_attr(signal_num, sigint_handler, SA_RESTART);

    bzero(buf, 100);
    while (1) {
        ret = read(0, buf, 100);
        if (ret == -1) {
            perror("read error");
        }
        break;
    }

    printf("read %d bytes\n", ret);
    printf("buf %s\n", buf);
}

/*
 *register a signal(2) use sigaction
 * ^Ccatch a signal: Interrupt
 * catch a signal
 * hello
 * read 6 bytes
 * buf hello
 */
void read_problem_resolve2()
{
    char buf[100];
    int signal_num = SIGINT, ret;
    register_signal_use_sigaction(signal_num, signal_handler);
    bzero(buf, 100);
    for(;;) {
        ret = read(0, buf, 100);
        if (ret == -1) {
            if (errno == EINTR) {
                printf("catch a signal\n");
                continue;
            }
            perror("read error");
        }
        break;
    }

    printf("read %d bytes\n", ret);
    printf("buf %s\n", buf);
}

/*
 * gcc problem.c register_signal.c -I.
 * ./a.out
 */

int main(int argc, char *argv[])
{
    /*
     * 管道破裂。
     * 这个信号通常在进程间通信产生，比如采用FIFO(管道)通信的两个进程，读管道没打开或者意外终止就往管道写，写进程会收到SIGPIPE信号。
     * 此外用Socket通信的两个进程，写进程在写Socket的时候，读进程已经终止。也会收到SIGPIPE信号
     */
    //忽略SIGPIPE
    register_signal_use_sigaction(SIGPIPE, SIG_IGN);

    read_problem();
    read_problem_resolve1();
    read_problem_resolve2();
    return 0;
}
