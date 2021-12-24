
#include <test_bank.h>


#if EXCEPTION_TEST == 1
#include <exceptions.h>
#include <interrupt_settings.h>
#include <panic.h>
#include <kernel_output.h>
#include <cpu.h>
#include <cpu_settings.h>
#include <kernel_error.h>

void end(void);

void end(void)
{
    kernel_printf("[TESTMODE] Software exception tests passed\n");

    /* Kill QEMU */
    kill_qemu();
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
        kill_qemu();
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
        kill_qemu();
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
        kill_qemu();
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
        kill_qemu();
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
        kill_qemu();
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
        kill_qemu();
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
        kill_qemu();
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
        kill_qemu();
    }
    else
    {
        kernel_printf("[TESTMODE] TEST_SW_EXC 8\n");
    }

    if((err = kernel_exception_register_handler(DIV_BY_ZERO_LINE, dummy))
     != OS_ERR_INTERRUPT_ALREADY_REGISTERED)
    {
        kernel_error("TEST_SW_EXC 9\n");
        kill_qemu();
    }
    else
    {
        kernel_printf("[TESTMODE] TEST_SW_EXC 9\n");
    }


    /* Test exception */

    volatile int i = 0;
    volatile int m = 5 / i;
    (void)m;

    kernel_error("Should have killed QEMU on exception catch");
    /* Kill QEMU */
    kill_qemu();
}
#else
void exception_test(void)
{
}
#endif