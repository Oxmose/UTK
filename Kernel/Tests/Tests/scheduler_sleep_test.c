#include <io/kernel_output.h>
#include <core/scheduler.h>
#include <interrupt/interrupts.h>
#include <Tests/test_bank.h>
#include <cpu.h>
#include <time/time_management.h>

#if SCHEDULER_SLEEP_TEST == 1

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
    thread_t thread;
    OS_RETURN_E err;

    kernel_interrupt_restore(1);

    kernel_printf("[TESTMODE] Scheduler tests sarts\n");

    err = sched_create_kernel_thread(&thread, 0, "test",
                                  1024, 0, print_th, NULL);
    if(err != OS_NO_ERR)
    {
        kernel_error("Cannot create threads %d\n", err);
        /* Kill QEMU */
        cpu_outw(0x2000, 0x604);    
        while(1)
        {
            __asm__ ("hlt");
        }
    
    }

    sched_wait_thread(thread, NULL, NULL);

    kernel_interrupt_disable();

    /* Kill QEMU */
    cpu_outw(0x2000, 0x604);    
    while(1)
    {
        __asm__ ("hlt");
    }
}
#else
void scheduler_sleep_test(void)
{

}
#endif