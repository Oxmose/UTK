


/* UTK configuration file */

void sched_set_thread_termination_cause(int value);
void sched_terminate_thread(void);
void sched_sleep(int val);
kernel_thread_t* sched_get_self();
OS_RETURN_E sched_init_ap(void);