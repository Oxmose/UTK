
#include <test_bank.h>


#if PIT_TEST == 1
#include <interrupts.h>
#include <kernel_output.h>
#include <cpu.h>
#include <pit.h>
#include <panic.h>
#include <interrupt_settings.h>

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
        KERNEL_PANIC(err);
    }
    else 
    {
        kernel_printf("[TESTMODE] TEST_PIT 0\n");
    }

    /* REGISTER OUR HANNDLER */
    if((err = pit_set_handler(pit_handler)) != OS_NO_ERR)
    {
        kernel_error("TEST_PIT 1\n");
        KERNEL_PANIC(err);
    }
    else 
    {
        kernel_printf("[TESTMODE] TEST_PIT 1\n");
    }


    /* REMOVE */
    if((err = pit_remove_handler()) != OS_NO_ERR)
    {
        kernel_error("TEST_PIT 2\n");
        KERNEL_PANIC(err);
    }
    else 
    {
        kernel_printf("[TESTMODE] TEST_PIT 2\n");
    }


    /* REGISTER OUR HANNDLER */
    if((err = pit_set_handler(pit_handler)) != OS_NO_ERR)
    {
        kernel_error("TEST_PIT 3\n");
        KERNEL_PANIC(err);
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
        KERNEL_PANIC(OS_ERR_UNAUTHORIZED_ACTION);
    }
    else 
    {
        kernel_printf("[TESTMODE] TEST_PIT 4\n");
    }


    for(i = 0; i < 10000000; ++i);
    if(counter != cnt_val)
    {
        kernel_error("TEST_PIT 5\n");
        KERNEL_PANIC(OS_ERR_UNAUTHORIZED_ACTION);
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
        KERNEL_PANIC(OS_ERR_UNAUTHORIZED_ACTION);
    }
    else 
    {
        kernel_printf("[TESTMODE] TEST_PIT 6\n");
    }


    for(i = 0; i < 10000000; ++i);
    if(counter != cnt_val)
    {
        kernel_error("TEST_PIT 7\n");
        KERNEL_PANIC(OS_ERR_UNAUTHORIZED_ACTION);
    }
    else 
    {
        kernel_printf("[TESTMODE] TEST_PIT 7\n");
    }


    if((err = pit_set_frequency(PIT_INIT_FREQ)) != OS_NO_ERR)
    {
        kernel_error("TEST_PIT 8\n");
        KERNEL_PANIC(err);
    }
    else 
    {
        kernel_printf("[TESTMODE] TEST_PIT 8\n");
    }


    if((err = pit_set_frequency(PIT_MIN_FREQ - 1)) != OS_ERR_OUT_OF_BOUND)
    {
        kernel_error("TEST_PIT 9\n");
        KERNEL_PANIC(err);
    }
    else 
    {
        kernel_printf("[TESTMODE] TEST_PIT 9\n");
    }


    if((err = pit_set_frequency(PIT_MAX_FREQ + 1)) != OS_ERR_OUT_OF_BOUND)
    {
        kernel_error("TEST_PIT 10\n");
        KERNEL_PANIC(err);
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
        KERNEL_PANIC(OS_ERR_UNAUTHORIZED_ACTION);
    }
    else 
    {
        kernel_printf("[TESTMODE] TEST_PIT 11\n");
    }


    /* REMOVE */
    if(pit_remove_handler() != OS_NO_ERR)
    {
        kernel_error("TEST_PIT 12\n");
        KERNEL_PANIC(OS_ERR_UNAUTHORIZED_ACTION);
    }
    else 
    {
        kernel_printf("[TESTMODE] TEST_PIT 12\n");
    }


    kernel_printf("[TESTMODE] PIT tests passed\n");

    pit_enable();
    kernel_interrupt_disable();

    /* Kill QEMU */
    kill_qemu();
}
#else 
void pit_test(void)
{
}
#endif