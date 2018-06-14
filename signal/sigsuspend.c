#include <unistd.h>
#include <signal.h>
#include <stdio.h>
#include <register_signal.h>

/*
 * sigsuspend的原子操作是:
 * 1. 设置新的mask阻塞当前进程
 * 2. 收到SIGUSR1信号，阻塞，程序继续挂起；收到其他信号，恢复原先的mask
 * 3. 调用该进程设置的信号处理函数
 * 4. 待信号处理函数返回
 *
 * 更改进程的信号屏蔽字可以阻塞所选择的信号，或解除对它们的阻塞，使用这种技术可以保护不希望由信号中断的代码临界区。
 */

void signal_handler(int signal_num)
{
    psignal(signal_num, "catch a signal");
}

void signal_pause()
{
    sigset_t new_set, old_set;

    //register_signal_use_sigaction(SIGINT, signal_handler);

    sigemptyset(&new_set);
    sigaddset(&new_set, SIGINT);

    sigprocmask(SIG_BLOCK, &new_set, &old_set);
    printf("blocked SIGINT");

    sigprocmask(SIG_SETMASK, &old_set, NULL);
    sleep(10);
    pause();
}

void signal_suspend()
{
    sigset_t new_set, old_set, wait_set;
    register_signal_use_sigaction(SIGUSR1, signal_handler);
    register_signal_use_sigaction(SIGINT, signal_handler);
    register_signal_use_sigaction(SIGQUIT, signal_handler);

    sigemptyset(&new_set);
    sigaddset(&new_set, SIGINT);

    sigemptyset(&wait_set);
    sigaddset(&wait_set, SIGUSR1);

    //保存当前信号机到old_set
    sigprocmask(SIG_BLOCK, &new_set, &old_set);

    if (sigsuspend(&wait_set) != -1) {
        perror("call sigsuspend error");
    }

    printf("after sigsuspend\n");
    sigprocmask(SIG_SETMASK, &old_set, NULL);
}

int main(int argc, char *argv[])
{
    signal_pause();
    signal_suspend();
    return 0;
}
