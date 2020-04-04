#include <interrupt/interrupts.h>
#include <io/kernel_output.h>
#include <cpu.h>
#include <pit.h>
#include <Tests/test_bank.h>
#include <core/panic.h>
#include <interrupt_settings.h>

#if PIT_TEST == 1
static volatile uint32_t counter;

static void pit_handler(cpu_state_t* cpu, uint32_t id, stack_state_t* stack)
{
    (void)cpu;
    (void)id;
    (void)stack;

    counter++;
    kernel_interrupt_set_irq_eoi(PIT_IRQ_LINE);
}

void pit_test(void)
{
    volatile uint32_t i;
    volatile uint32_t cnt_val;
    OS_RETURN_E err;

    kernel_interrupt_restore(1);

    /* REGISTER NULL */
    if((err = pit_set_handler(NULL)) != OS_ERR_NULL_POINTER)
    {
        kernel_error("TEST_PIT 0\n");
        kernel_panic(err);
    }
    else 
    {
        kernel_printf("[TESTMODE] TEST_PIT 0\n");
    }

    /* REGISTER OUR HANNDLER */
    if((err = pit_set_handler(pit_handler)) != OS_NO_ERR)
    {
        kernel_error("TEST_PIT 1\n");
        kernel_panic(err);
    }
    else 
    {
        kernel_printf("[TESTMODE] TEST_PIT 1\n");
    }


    /* REMOVE */
    if((err = pit_remove_handler()) != OS_NO_ERR)
    {
        kernel_error("TEST_PIT 2\n");
        kernel_panic(err);
    }
    else 
    {
        kernel_printf("[TESTMODE] TEST_PIT 2\n");
    }


    /* REGISTER OUR HANNDLER */
    if((err = pit_set_handler(pit_handler)) != OS_NO_ERR)
    {
        kernel_error("TEST_PIT 3\n");
        kernel_panic(err);
    }
    else 
    {
        kernel_printf("[TESTMODE] TEST_PIT 3\n");
    }


    /* CHECK ENABLE / DISABLE*/
    counter = 0;
    cnt_val = 0;

    pit_enable();
    for(i = 0; i < 10000000; ++i);
    pit_disable();
    cnt_val = counter;
    if(counter == 0)
    {
        kernel_error("TEST_PIT 4\n");
        kernel_panic(OS_ERR_UNAUTHORIZED_ACTION);
    }
    else 
    {
        kernel_printf("[TESTMODE] TEST_PIT 4\n");
    }


    for(i = 0; i < 10000000; ++i);
    if(counter != cnt_val)
    {
        kernel_error("TEST_PIT 5\n");
        kernel_panic(OS_ERR_UNAUTHORIZED_ACTION);
    }
    else 
    {
        kernel_printf("[TESTMODE] TEST_PIT 5\n");
    }


    counter = 0;
    pit_enable();

    for(i = 0; i < 10000000; ++i);

    pit_disable();
    cnt_val = counter;
    if(counter == 0)
    {
        kernel_error("TEST_PIT 6\n");
        kernel_panic(OS_ERR_UNAUTHORIZED_ACTION);
    }
    else 
    {
        kernel_printf("[TESTMODE] TEST_PIT 6\n");
    }


    for(i = 0; i < 10000000; ++i);
    if(counter != cnt_val)
    {
        kernel_error("TEST_PIT 7\n");
        kernel_panic(OS_ERR_UNAUTHORIZED_ACTION);
    }
    else 
    {
        kernel_printf("[TESTMODE] TEST_PIT 7\n");
    }


    if((err = pit_set_frequency(PIT_INIT_FREQ)) != OS_NO_ERR)
    {
        kernel_error("TEST_PIT 8\n");
        kernel_panic(err);
    }
    else 
    {
        kernel_printf("[TESTMODE] TEST_PIT 8\n");
    }


    if((err = pit_set_frequency(PIT_MIN_FREQ - 1)) != OS_ERR_OUT_OF_BOUND)
    {
        kernel_error("TEST_PIT 9\n");
        kernel_panic(err);
    }
    else 
    {
        kernel_printf("[TESTMODE] TEST_PIT 9\n");
    }


    if((err = pit_set_frequency(PIT_MAX_FREQ + 1)) != OS_ERR_OUT_OF_BOUND)
    {
        kernel_error("TEST_PIT 10\n");
        kernel_panic(err);
    }
    else 
    {
        kernel_printf("[TESTMODE] TEST_PIT 10\n");
    }


    /* Check if the PIT did not erenabled itself between */
    for(i = 0; i < 10000000; ++i);
    if(counter != cnt_val)
    {
        kernel_error("TEST_PIT 11\n");
        kernel_panic(OS_ERR_UNAUTHORIZED_ACTION);
    }
    else 
    {
        kernel_printf("[TESTMODE] TEST_PIT 11\n");
    }


    /* REMOVE */
    if(pit_remove_handler() != OS_NO_ERR)
    {
        kernel_error("TEST_PIT 12\n");
        kernel_panic(OS_ERR_UNAUTHORIZED_ACTION);
    }
    else 
    {
        kernel_printf("[TESTMODE] TEST_PIT 12\n");
    }


    kernel_printf("[TESTMODE] PIT tests passed\n");

    pit_enable();
    kernel_interrupt_disable();

    /* Kill QEMU */
    cpu_outw(0x2000, 0x604);    
    while(1)
    {
        __asm__ ("hlt");
    }
}
#else 
void pit_test(void)
{
}
#endif