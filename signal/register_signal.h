typedef void     (*SIGNAL_HANDLER)(int);
void register_signal_use_signal(int signal_num, SIGNAL_HANDLER h);
void register_signal_use_sigaction(int signal_num, SIGNAL_HANDLER h);
void register_signal_with_attr(int signal_num, SIGNAL_HANDLER h, int attr);
