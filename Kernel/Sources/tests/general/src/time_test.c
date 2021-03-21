
#include <test_bank.h>
#if TIME_TEST == 1

#include <interrupts.h>
#include <kernel_output.h>
#include <cpu_api.h>
#include <time_management.h>
#include <panic.h>
#include <interrupt_settings.h>

void time_test(void)
{
    uint32_t tick_count = time_get_tick_count();
    uint32_t new_tick_count;


    kernel_interrupt_restore(1);

    for(volatile uint32_t i = 0; i < 5000000; ++i);

    new_tick_count = time_get_tick_count();

    if(tick_count != new_tick_count)
    {
        kernel_printf("[TESTMODE] TIME tests passed\n");
    }
    else
    {
        kernel_error("Time test failed (%d) (%d)\n",
                      tick_count, new_tick_count);
    }

    tick_count = time_get_tick_count();
    time_wait_no_sched(3000);
    new_tick_count = time_get_tick_count();
    if(new_tick_count != tick_count + 3000 / (1000 / KERNEL_MAIN_TIMER_FREQ))
    {
        kernel_error("Wait no sched failed %u, %u\n",
                      tick_count, new_tick_count);
    }
    else 
    {
        kernel_printf("[TESTMODE] TIME wait passed\n");
    }

    /* Kill QEMU */
    kill_qemu();
}
#else 
void time_test(void)
{
}
#endif