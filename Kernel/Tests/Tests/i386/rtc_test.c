#include <interrupt/interrupts.h>
#include <io/kernel_output.h>
#include <cpu.h>
#include <rtc.h>
#include <Tests/test_bank.h>
#include <core/panic.h>
#include <interrupt_settings.h>

#if RTC_TEST == 1
static volatile uint32_t counter;
static void rtc_handler(cpu_state_t* cpu, uint32_t id, stack_state_t* stack)
{
    (void)cpu;
    (void)id;
    (void)stack;

    counter++;
    rtc_update_time();
    kernel_interrupt_set_irq_eoi(RTC_IRQ_LINE);
}

void rtc_test(void)
{
    volatile uint32_t i;
    volatile uint32_t cnt_val;
    OS_RETURN_E err;

    kernel_interrupt_restore(1);

    /* REGISTER NULL */
    if((err = rtc_set_handler(NULL)) != OS_ERR_NULL_POINTER)
    {
        kernel_error("TEST_RTC 0\n");
        kernel_panic(err);
    }
    else 
    {
        kernel_printf("[TESTMODE] TEST_RTC 0\n");
    }

    /* REGISTER OUR HANNDLER */
    if((err = rtc_set_handler(rtc_handler)) != OS_NO_ERR)
    {
        kernel_error("TEST_RTC 1\n");
        kernel_panic(err);
    }
    else 
    {
        kernel_printf("[TESTMODE] TEST_RTC 1\n");
    }


    /* REMOVE */
    if((err = rtc_remove_handler()) != OS_NO_ERR)
    {
        kernel_error("TEST_RTC 2\n");
        kernel_panic(err);
    }
    else 
    {
        kernel_printf("[TESTMODE] TEST_RTC 2\n");
    }


    /* REGISTER OUR HANNDLER */
    if((err = rtc_set_handler(rtc_handler)) != OS_NO_ERR)
    {
        kernel_error("TEST_RTC 3\n");
        kernel_panic(err);
    }
    else 
    {
        kernel_printf("[TESTMODE] TEST_RTC 3\n");
    }


    /* CHECK ENABLE / DISABLE*/
    counter = 0;
    cnt_val = 0;

    rtc_enable();
    for(i = 0; i < 10000000; ++i);
    rtc_disable();
    cnt_val = counter;
    if(counter == 0)
    {
        kernel_error("TEST_RTC 4\n");
        kernel_panic(OS_ERR_UNAUTHORIZED_ACTION);
    }
    else 
    {
        kernel_printf("[TESTMODE] TEST_RTC 4\n");
    }


    for(i = 0; i < 10000000; ++i);
    if(counter != cnt_val)
    {
        kernel_error("TEST_RTC 5\n");
        kernel_panic(OS_ERR_UNAUTHORIZED_ACTION);
    }
    else 
    {
        kernel_printf("[TESTMODE] TEST_RTC 5\n");
    }


    counter = 0;
    rtc_enable();

    for(i = 0; i < 10000000; ++i);

    rtc_disable();
    cnt_val = counter;
    if(counter == 0)
    {
        kernel_error("TEST_RTC 6\n");
        kernel_panic(OS_ERR_UNAUTHORIZED_ACTION);
    }
    else 
    {
        kernel_printf("[TESTMODE] TEST_RTC 6\n");
    }


    for(i = 0; i < 10000000; ++i);
    if(counter != cnt_val)
    {
        kernel_error("TEST_RTC 7\n");
        kernel_panic(OS_ERR_UNAUTHORIZED_ACTION);
    }
    else 
    {
        kernel_printf("[TESTMODE] TEST_RTC 7\n");
    }


    if((err = rtc_set_frequency(RTC_INIT_RATE)) != OS_NO_ERR)
    {
        kernel_error("TEST_RTC 8\n");
        kernel_panic(err);
    }
    else 
    {
        kernel_printf("[TESTMODE] TEST_RTC 8\n");
    }


    if((err = rtc_set_frequency(RTC_MIN_FREQ - 1)) != OS_ERR_OUT_OF_BOUND)
    {
        kernel_error("TEST_RTC 9\n");
        kernel_panic(err);
    }
    else 
    {
        kernel_printf("[TESTMODE] TEST_RTC 9\n");
    }


    if((err = rtc_set_frequency(RTC_MAX_FREQ + 1)) != OS_ERR_OUT_OF_BOUND)
    {
        kernel_error("TEST_RTC 10\n");
        kernel_panic(err);
    }
    else 
    {
        kernel_printf("[TESTMODE] TEST_RTC 10\n");
    }


    /* Check if the RTC did not erenabled itself between */
    for(i = 0; i < 10000000; ++i);
    if(counter != cnt_val)
    {
        kernel_error("TEST_RTC 11\n");
        kernel_panic(OS_ERR_UNAUTHORIZED_ACTION);
    }
    else 
    {
        kernel_printf("[TESTMODE] TEST_RTC 11\n");
    }


    /* REMOVE */
    if(rtc_remove_handler() != OS_NO_ERR)
    {
        kernel_error("TEST_RTC 12\n");
        kernel_panic(OS_ERR_UNAUTHORIZED_ACTION);
    }
    else 
    {
        kernel_printf("[TESTMODE] TEST_RTC 12\n");
    }


    kernel_printf("[TESTMODE] RTC tests passed\n");

    rtc_enable();
    kernel_interrupt_disable();

    /* Kill QEMU */
    cpu_outw(0x2000, 0x604);    
    while(1)
    {
        __asm__ ("hlt");
    }
}
#else 
void rtc_test(void)
{
}
#endif