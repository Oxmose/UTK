#include <interrupt/interrupts.h>
#include <io/kernel_output.h>
#include <cpu.h>
#include <time/time_management.h>
#include <Tests/test_bank.h>
#include <core/panic.h>
#include <interrupt_settings.h>
#include <lapic.h>
#include <rtc.h> 

#if TIME_TEST == 1
void time_test(void)
{
    uint32_t tick_count = time_get_tick_count();
    uint32_t daytime = rtc_get_current_daytime();
    uint32_t new_tick_count;
    uint32_t new_daytime;


    kernel_interrupt_restore(1);

    for(volatile uint32_t i = 0; i < 5000000; ++i);

    new_tick_count = time_get_tick_count();
    new_daytime = rtc_get_current_daytime();

    if(tick_count != new_tick_count)
    {
        kernel_printf("[TESTMODE] TIME tests passed\n");
    }
    else
    {
        kernel_error("Time test failed (%d %d) (%d %d)\n",
                      tick_count, daytime, new_tick_count, new_daytime);
    }

    daytime = rtc_get_current_daytime();
    time_wait_no_sched(3000);
    new_daytime = rtc_get_current_daytime();
    if(new_daytime != daytime + 3)
    {
        kernel_error("Wait no sched failed %u, %u, %u\n",
                      daytime, new_daytime, new_daytime - daytime);
    }
    
    kernel_printf("[TESTMODE] TIME wait passed\n");

    /* Kill QEMU */
    cpu_outw(0x2000, 0x604);    
    while(1)
    {
        __asm__ ("hlt");
    }
}
#else 
void time_test(void)
{
}
#endif