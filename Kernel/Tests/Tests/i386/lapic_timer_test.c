#include <interrupt/interrupts.h>
#include <io/kernel_output.h>
#include <cpu.h>
#include <lapic.h>
#include <Tests/test_bank.h>
#include <core/panic.h>
#include <interrupt_settings.h>

#if LAPIC_TIMER_TEST == 1
static volatile uint32_t counter;

#define LAPIC_TIMER_INIT_FREQ 100

static void lapic_timer_handler(cpu_state_t* cpu, uint32_t id, stack_state_t* stack)
{
    (void)cpu;
    (void)id;
    (void)stack;

    counter++;
    kernel_interrupt_set_irq_eoi(LAPIC_TIMER_INTERRUPT_LINE);
}

void lapic_timer_test(void)
{
    volatile uint32_t i;
    volatile uint32_t cnt_val;
    OS_RETURN_E err;

    kernel_interrupt_restore(1);

    /* REGISTER NULL */
    if((err = lapic_timer_set_handler(NULL)) != OS_ERR_NULL_POINTER)
    {
        kernel_error("TEST_LAPIC_TIMER 0\n");
        kernel_panic(err);
    }
    else 
    {
        kernel_printf("[TESTMODE] TEST_LAPIC_TIMER 0\n");
    }

    /* REGISTER OUR HANNDLER */
    if((err = lapic_timer_set_handler(lapic_timer_handler)) != OS_NO_ERR)
    {
        kernel_error("TEST_LAPIC_TIMER 1\n");
        kernel_panic(err);
    }
    else 
    {
        kernel_printf("[TESTMODE] TEST_LAPIC_TIMER 1\n");
    }


    /* REMOVE */
    if((err = lapic_timer_remove_handler()) != OS_NO_ERR)
    {
        kernel_error("TEST_LAPIC_TIMER 2\n");
        kernel_panic(err);
    }
    else 
    {
        kernel_printf("[TESTMODE] TEST_LAPIC_TIMER 2\n");
    }


    /* REGISTER OUR HANNDLER */
    if((err = lapic_timer_set_handler(lapic_timer_handler)) != OS_NO_ERR)
    {
        kernel_error("TEST_LAPIC_TIMER 3\n");
        kernel_panic(err);
    }
    else 
    {
        kernel_printf("[TESTMODE] TEST_LAPIC_TIMER 3\n");
    }


    /* CHECK ENABLE / DISABLE*/
    counter = 0;
    cnt_val = 0;

    lapic_timer_enable();
    for(i = 0; i < 10000000; ++i);
    lapic_timer_disable();
    cnt_val = counter;
    if(counter == 0)
    {
        kernel_error("TEST_LAPIC_TIMER 4\n");
        kernel_panic(OS_ERR_UNAUTHORIZED_ACTION);
    }
    else 
    {
        kernel_printf("[TESTMODE] TEST_LAPIC_TIMER 4\n");
    }


    for(i = 0; i < 10000000; ++i);
    if(counter != cnt_val)
    {
        kernel_error("TEST_LAPIC_TIMER 5\n");
        kernel_panic(OS_ERR_UNAUTHORIZED_ACTION);
    }
    else 
    {
        kernel_printf("[TESTMODE] TEST_LAPIC_TIMER 5\n");
    }


    counter = 0;
    lapic_timer_enable();

    for(i = 0; i < 10000000; ++i);

    lapic_timer_disable();
    cnt_val = counter;
    if(counter == 0)
    {
        kernel_error("TEST_LAPIC_TIMER 6\n");
        kernel_panic(OS_ERR_UNAUTHORIZED_ACTION);
    }
    else 
    {
        kernel_printf("[TESTMODE] TEST_LAPIC_TIMER 6\n");
    }


    for(i = 0; i < 10000000; ++i);
    if(counter != cnt_val)
    {
        kernel_error("TEST_LAPIC_TIMER 7\n");
        kernel_panic(OS_ERR_UNAUTHORIZED_ACTION);
    }
    else 
    {
        kernel_printf("[TESTMODE] TEST_LAPIC_TIMER 7\n");
    }


    if((err = lapic_timer_set_frequency(LAPIC_TIMER_INIT_FREQ)) != OS_NO_ERR)
    {
        kernel_error("TEST_LAPIC_TIMER 8\n");
        kernel_panic(err);
    }
    else 
    {
        kernel_printf("[TESTMODE] TEST_LAPIC_TIMER 8\n");
    }


    /* Check if the LAPIC_TIMER did not erenabled itself between */
    for(i = 0; i < 10000000; ++i);
    if(counter != cnt_val)
    {
        kernel_error("TEST_LAPIC_TIMER 9\n");
        kernel_panic(OS_ERR_UNAUTHORIZED_ACTION);
    }
    else 
    {
        kernel_printf("[TESTMODE] TEST_LAPIC_TIMER 9\n");
    }


    /* REMOVE */
    if(lapic_timer_remove_handler() != OS_NO_ERR)
    {
        kernel_error("TEST_LAPIC_TIMER 10\n");
        kernel_panic(OS_ERR_UNAUTHORIZED_ACTION);
    }
    else 
    {
        kernel_printf("[TESTMODE] TEST_LAPIC_TIMER 10\n");
    }


    kernel_printf("[TESTMODE] LAPIC_TIMER tests passed\n");

    lapic_timer_enable();
    kernel_interrupt_disable();

    /* Kill QEMU */
    cpu_outw(0x2000, 0x604);    
    while(1)
    {
        __asm__ ("hlt");
    }
}
#else 
void lapic_timer_test(void)
{
}
#endif