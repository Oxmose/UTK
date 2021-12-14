#include <test_bank.h>


#if SCHEDULER_SLEEP_TEST == 1

#include <kernel_output.h>
#include <scheduler.h>
#include <interrupts.h>
#include <time_management.h>


static void* print_th(void*args)
{
    (void) args;
    uint64_t i = time_get_current_uptime();
    sched_sleep(400);
    if(time_get_current_uptime() < i + 400)
    {
        kernel_error("Scheduler thread sleep tests failed\n");
    }
    else
    {
        kernel_printf("[TESTMODE] Scheduler thread sleep tests passed\n");
    }
    return NULL;
}

void scheduler_sleep_test(void)
{
    kernel_thread_t* thread;
    OS_RETURN_E err;

    kernel_interrupt_restore(1);

    kernel_printf("[TESTMODE] Scheduler tests sarts\n");

    err = sched_create_kernel_thread(&thread, 0, "test",
                                  THREAD_TYPE_KERNEL, 0x1000, print_th, NULL);
    if(err != OS_NO_ERR)
    {
        kernel_error("Cannot create threads %d\n", err);
        /* Kill QEMU */
        kill_qemu();    
    }

    sched_join_thread(thread, NULL, NULL);

    kernel_interrupt_disable();

    kill_qemu();  
}
#else
void scheduler_sleep_test(void)
{

}
#endif