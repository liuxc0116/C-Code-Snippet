#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <sys/types.h>
#include <register_signal.h>

void register_signal_use_signal(int signal_num, SIGNAL_HANDLER handler)
{
    printf("register a signal(%d) use signal\n", signal_num);
    if (signal(signal_num, SIG_IGN) != SIG_IGN) {
        signal(signal_num, handler);
    }
}

void register_signal_use_sigaction(int signal_num, SIGNAL_HANDLER handler)
{
    printf("register a signal(%d) use sigaction\n", signal_num);
    struct sigaction action, old_action;
    action.sa_handler = handler;
    sigemptyset(&action.sa_mask);
    action.sa_flags = 0;

    sigaction(signal_num, NULL, &old_action);
    if (old_action.sa_handler != SIG_IGN) {
        sigaction(signal_num, &action, NULL);
    }
}

void register_signal_with_attr(int signal_num, SIGNAL_HANDLER handler, int attr)
{
    printf("register a signal(%d) use sigaction\n", signal_num);
    struct sigaction action, old_action;
    action.sa_handler = handler;
    sigemptyset(&action.sa_mask);
    action.sa_flags |= attr;

    sigaction(signal_num, NULL, &old_action);
    if (old_action.sa_handler != SIG_IGN) {
        sigaction(signal_num, &action, NULL);
    }
}
