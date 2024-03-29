

#include <test_bank.h>

#if RTC_TEST == 1
#include <interrupts.h>
#include <kernel_output.h>
#include <cpu.h>
#include <rt_clock.h>
#include <rtc.h>
#include <panic.h>
#include <interrupt_settings.h>

/** @brief Initial RTC tick rate. */
#define RTC_INIT_RATE 10
/** @brief RTC minimal frequency. */
#define RTC_MIN_FREQ 2
/** @brief RTC maximal frequency. */
#define RTC_MAX_FREQ 8192

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
        kill_qemu();
    }
    else
    {
        kernel_printf("[TESTMODE] TEST_RTC 0\n");
    }

    /* REGISTER OUR HANNDLER */
    if((err = rtc_set_handler(rtc_handler)) != OS_NO_ERR)
    {
        kernel_error("TEST_RTC 1\n");
        kill_qemu();
    }
    else
    {
        kernel_printf("[TESTMODE] TEST_RTC 1\n");
    }


    /* REMOVE */
    if((err = rtc_remove_handler()) != OS_NO_ERR)
    {
        kernel_error("TEST_RTC 2\n");
        kill_qemu();
    }
    else
    {
        kernel_printf("[TESTMODE] TEST_RTC 2\n");
    }


    /* REGISTER OUR HANNDLER */
    if((err = rtc_set_handler(rtc_handler)) != OS_NO_ERR)
    {
        kernel_error("TEST_RTC 3\n");
        kill_qemu();
    }
    else
    {
        kernel_printf("[TESTMODE] TEST_RTC 3\n");
    }


    /* CHECK ENABLE / DISABLE*/
    counter = 0;
    cnt_val = 0;

    rtc_enable();
    for(i = 0; i < 100000000; ++i);
    rtc_disable();
    cnt_val = counter;
    if(counter == 0)
    {
        kernel_error("TEST_RTC 4\n");
        kill_qemu();
    }
    else
    {
        kernel_printf("[TESTMODE] TEST_RTC 4\n");
    }


    for(i = 0; i < 10000000; ++i);
    if(counter != cnt_val)
    {
        kernel_error("TEST_RTC 5\n");
        kill_qemu();
    }
    else
    {
        kernel_printf("[TESTMODE] TEST_RTC 5\n");
    }


    counter = 0;
    rtc_enable();

    for(i = 0; i < 100000000; ++i);

    rtc_disable();
    cnt_val = counter;
    if(counter == 0)
    {
        kernel_error("TEST_RTC 6\n");
        kill_qemu();
    }
    else
    {
        kernel_printf("[TESTMODE] TEST_RTC 6\n");
    }


    for(i = 0; i < 100000000; ++i);
    if(counter != cnt_val)
    {
        kernel_error("TEST_RTC 7\n");
        kill_qemu();
    }
    else
    {
        kernel_printf("[TESTMODE] TEST_RTC 7\n");
    }


    rtc_set_frequency(RTC_INIT_RATE);
    kernel_printf("[TESTMODE] TEST_RTC 8\n");
    kernel_printf("[TESTMODE] TEST_RTC 9\n");
    kernel_printf("[TESTMODE] TEST_RTC 10\n");

    /* Check if the RTC did not erenabled itself between */
    for(i = 0; i < 100000000; ++i);
    if(counter != cnt_val)
    {
        kernel_error("TEST_RTC 11\n");
        kill_qemu();
    }
    else
    {
        kernel_printf("[TESTMODE] TEST_RTC 11\n");
    }


    /* REMOVE */
    if(rtc_remove_handler() != OS_NO_ERR)
    {
        kernel_error("TEST_RTC 12\n");
        kill_qemu();
    }
    else
    {
        kernel_printf("[TESTMODE] TEST_RTC 12\n");
    }


    kernel_printf("[TESTMODE] RTC tests passed\n");

    rtc_enable();
    kernel_interrupt_disable();

    /* Kill QEMU */
    kill_qemu();
}
#else
void rtc_test(void)
{
}
#endif