#include <interrupt/exceptions.h>
#include <interrupt_settings.h>
#include <core/panic.h>
#include <io/kernel_output.h>
#include <cpu.h>
#include <Tests/test_bank.h>
#include <cpu_settings.h>

#if EXCEPTION_TEST == 1

void end(void)
{
    kernel_printf("[TESTMODE] Software exception tests passed\n");

    /* Kill QEMU */
    cpu_outw(0x2000, 0x604);    
    while(1)
    {
        __asm__ ("hlt");
    }
}

static void dummy(cpu_state_t* cpu,  uint32_t int_id,
                                stack_state_t* stack)
{
    (void)cpu;
    (void)int_id;
    stack->eip = (uintptr_t)end;
    kernel_printf("[TESTMODE] EXCEPTION CATCHED\n");
}

void exception_test(void)
{
    OS_RETURN_E err;


    /* TEST REGISTER < MIN */
    if((err = kernel_exception_register_handler(MIN_EXCEPTION_LINE - 1, dummy))
     != OR_ERR_UNAUTHORIZED_INTERRUPT_LINE)
    {
        kernel_error("TEST_SW_EXC 0\n");
        kernel_panic(err);
    }
    else 
    {
        kernel_printf("[TESTMODE] TEST_SW_EXC 0\n");
    }

    /* TEST REGISTER > MAX */
    if((err = kernel_exception_register_handler(MAX_EXCEPTION_LINE + 1, dummy))
     != OR_ERR_UNAUTHORIZED_INTERRUPT_LINE)
    {
        kernel_error("TEST_SW_EXC 1\n");
        kernel_panic(err);
    }
    else 
    {
        kernel_printf("[TESTMODE] TEST_SW_EXC 1\n");
    }

    /* TEST REMOVE < MIN */
    if((err = kernel_exception_remove_handler(MIN_EXCEPTION_LINE - 1))
     != OR_ERR_UNAUTHORIZED_INTERRUPT_LINE)
    {
        kernel_error("TEST_SW_EXC 2\n");
        kernel_panic(err);
    }
    else 
    {
        kernel_printf("[TESTMODE] TEST_SW_EXC 2\n");
    }

    /* TEST REMOVE > MAX */
    if((err = kernel_exception_remove_handler(MAX_EXCEPTION_LINE + 1))
     != OR_ERR_UNAUTHORIZED_INTERRUPT_LINE)
    {
        kernel_error("TEST_SW_EXC 3\n");
        kernel_panic(err);
    }
    else 
    {
        kernel_printf("[TESTMODE] TEST_SW_EXC 3\n");
    }

    /* TEST NULL HANDLER */
    if((err = kernel_exception_register_handler(MIN_EXCEPTION_LINE, NULL))
     != OS_ERR_NULL_POINTER)
    {
        kernel_error("TEST_SW_EXC 4\n");
        kernel_panic(err);
    }
    else 
    {
        kernel_printf("[TESTMODE] TEST_SW_EXC 4\n");
    }

    /* TEST REMOVE WHEN NOT REGISTERED */
    if((err = kernel_exception_remove_handler(MIN_EXCEPTION_LINE))
     != OS_NO_ERR)
    {
        kernel_error("TEST_SW_EXC 5\n");
        kernel_panic(err);
    }
    else 
    {
        kernel_printf("[TESTMODE] TEST_SW_EXC 5\n");
    }
     /* TEST REMOVE WHEN NOT REGISTERED */
    if((err = kernel_exception_remove_handler(MIN_EXCEPTION_LINE))
     != OS_ERR_INTERRUPT_NOT_REGISTERED)
    {
        kernel_error("TEST_SW_EXC 7\n");
        kernel_panic(err);
    }
    else 
    {
        kernel_printf("[TESTMODE] TEST_SW_EXC 7\n");
    }

    /* TEST REGISTER WHEN ALREADY REGISTERED */
    if((err = kernel_exception_register_handler(MIN_EXCEPTION_LINE, dummy))
     != OS_NO_ERR)
    {
        kernel_error("TEST_SW_EXC 8\n");
        kernel_panic(err);
    }
    else 
    {
        kernel_printf("[TESTMODE] TEST_SW_EXC 8\n");
    }

    if((err = kernel_exception_register_handler(DIV_BY_ZERO_LINE, dummy))
     != OS_ERR_INTERRUPT_ALREADY_REGISTERED)
    {
        kernel_error("TEST_SW_EXC 9\n");
        kernel_panic(err);
    }
    else 
    {
        kernel_printf("[TESTMODE] TEST_SW_EXC 9\n");
    }

    
    /* Test exception */

    volatile int i = 0;
    volatile int m = 5 / i;
    (void)m;

    /* Kill QEMU */
    cpu_outw(0x2000, 0x604);    
    while(1)
    {
        __asm__ ("hlt");
    }
}
#else 
void exception_test(void)
{
}
#endif