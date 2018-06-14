#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <sys/types.h>
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
    //触发SIGTERM
    trigger_signal(SIGTERM, 1);
    psignal(signal_num, "catch a signal");
}

/*
 * 全局阻塞信号
 * 输出结果如下:
 * register a signal(2) use sigaction
 * block SIGINT: Interrupt
 * send signal: Interrupt
 * send signal: Interrupt
 * send signal: Interrupt
 * this signal is pending: Interrupt
 * unblock signal: Interrupt
 * catch a signal: Interrupt
 * no signal pending
 *
 * 发送了三次信号，结果只执行一次，说明SIGINT是不可靠信号，不支持信号排队
 */

void block_signal_always()
{
    sigset_t block, pending;
    int sig, flag;
    int signal_num = SIGINT;
    register_signal_use_sigaction(signal_num, signal_handler);

    sigemptyset(&block);
    sigaddset(&block, SIGINT);
    psignal(signal_num, "block SIGINT");
    sigprocmask(SIG_BLOCK, &block, NULL);

    trigger_signal(signal_num, 3);
    sleep(1);

    flag = 0;
    sigpending(&pending);
    for (sig = 1; sig < NSIG; sig++) {
        if (sigismember(&pending, sig)) {
            flag = 1;
            psignal(sig, "this signal is pending");
        }
    }

    if (flag == 0) {
        printf("no signal pending\n");
    }

    psignal(signal_num, "unblock signal");
    sigprocmask(SIG_UNBLOCK, &block, NULL);

    flag = 0;
    sigpending(&pending);
    for (sig = 1; sig < NSIG; sig++) {
        if (sigismember(&pending, sig)) {
            flag = 1;
            psignal(sig, "this signal is pending");
        }
    }

    if (flag == 0) {
        printf("no signal pending\n");
    }
}

/*
 * 只在信号执行期间阻塞信号
 * 输出结果如下:
 * register a signal(2) use sigaction
 * register a signal(15) use sigaction
 * send signal: Interrupt
 * send signal: Terminated
 * catch a signal: Interrupt
 * catch a signal: Terminated
 *
 * 当SIGINT的handler执行结束后才执行SIGTERM的handler
 */

void block_signal_in_signal_handler()
{
    int signal_num = SIGINT;
    printf("register a signal(%d) use sigaction\n", signal_num);
    struct sigaction action, old_action;
    action.sa_handler = sigint_handler;
    sigemptyset(&action.sa_mask);
    //安装handler的时候, 设置在handler执行期间, 屏蔽掉SIGTERM信号
    sigaddset(&action.sa_mask, SIGTERM);
    action.sa_flags = 0;

    sigaction(signal_num, NULL, &old_action);
    if (old_action.sa_handler != SIG_IGN) {
        sigaction(signal_num, &action, NULL);
    }

    register_signal_use_sigaction(SIGTERM, signal_handler);

    trigger_signal(signal_num, 1);
}

/*
 * gcc block_signal.c register_signal.c -I.
 * ./a.out
 */

int main(int argc, char *argv[])
{
    block_signal_always();
    block_signal_in_signal_handler();
    return 0;
}
