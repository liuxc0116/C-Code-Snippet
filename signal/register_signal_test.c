#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <sys/types.h>
#include <register_signal.h>

void signal_handler(int signal_num)
{
    //在早期的linux系统中，信号被交付后, 系统自动的重置handler为默认动作
    //为了使信号在handler处理期间, 仍能对后继信号做出反应, 往往在handler的第一条语句再次调用 signal
    //signal(signum, signal_handler);
    psignal(signal_num, "catch a signal");
}

void trigger_signal(int signal_num)
{
    int i;
    for(i = 0; i < 3; i++) {
        sleep(1);
        psignal(signal_num, "send signal");
        kill(getpid(), signal_num);
    }
}

/*
 * gcc register_signal_test.c  register_signal.c -I.
 * ./a.out
 * 输出结果如下：
 * register a signal(2) use signal
 * send signal: Interrupt
 * catch a signal: Interrupt
 * send signal: Interrupt
 * catch a signal: Interrupt
 * send signal: Interrupt
 * catch a signal: Interrupt
 * register a signal(1) use sigaction
 * send signal: Hangup
 * catch a signal: Hangup
 * send signal: Hangup
 * catch a signal: Hangup
 * send signal: Hangup
 * catch a signal: Hangup
 * Killed: 9
 */

int main(int argc, char *argv[])
{
    int signal_num = SIGINT;
    register_signal_use_signal(signal_num, signal_handler);
    trigger_signal(signal_num);

    signal_num = SIGHUP;
    register_signal_use_sigaction(signal_num, signal_handler);
    trigger_signal(signal_num);

    //kill -9 pid
    kill(getpid(), SIGKILL);
    //kill pid
    //kill(getpid(), SIGTERM);
    return 0;
}
