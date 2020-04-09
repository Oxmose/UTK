#include <io/kernel_output.h>
#include <core/scheduler.h>
#include <interrupt/interrupts.h>
#include <Tests/test_bank.h>
#include <cpu.h>
#include <time/time_management.h>
#include <sync/critical.h> 
#if SCHEDULER_SLEEP_MC_TEST == 1

static volatile uint32_t lock = 0;

static void* print_th(void*args)
{
    (void) args;
    uint32_t i = time_get_current_uptime();
    sched_sleep(400);
    uint32_t diff = time_get_current_uptime() - i;

    if(diff < 400)
    {
        __pause_spinlock(&lock);
        kernel_error("Scheduler thread sleep tests failed %d\n", diff);
        lock = 0;
    }
    else
    {
        __pause_spinlock(&lock);
        kernel_printf("[TESTMODE] Scheduler thread sleep tests passed %d\n", diff);
        lock = 0;
    }
    return NULL;
}

void scheduler_sleep_mc_test(void)
{
    thread_t threads[MAX_CPU_COUNT];
    OS_RETURN_E err;
    uint32_t i;

    kernel_interrupt_restore(1);

    kernel_printf("[TESTMODE] Scheduler tests starts\n");

    for(i = 0; i < MAX_CPU_COUNT; ++i)
    {
        err = sched_create_kernel_thread(&threads[i], 0, "test",
                                    1024, i, print_th, NULL);
        if(err != OS_NO_ERR)
        {
            kernel_error("Cannot create threads %d\n", err);
        }
    }


    for(i = 0; i < MAX_CPU_COUNT; ++i)
    {
        sched_wait_thread(threads[i], NULL, NULL);
    }

    kernel_printf("[TESTMODE] Scheduler test passed\n");

    kernel_interrupt_disable();

    /* Kill QEMU */
    cpu_outw(0x2000, 0x604);    
    while(1)
    {
        __asm__ ("hlt");
    }
}
#else
void scheduler_sleep_mc_test(void)
{

}
#endif