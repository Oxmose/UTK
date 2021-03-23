/*******************************************************************************
 * @file syscall.c
 * 
 * @see syscall.h
 *
 * @author Alexy Torres Aurora Dugo
 *
 * @date 21/03/2021
 *
 * @version 1.0
 *
 * @brief System call management.
 * 
 * @details System call management. This modules defines the functions used to
 * perform system calls as well as their management.
 * 
 * @copyright Alexy Torres Aurora Dugo
 ******************************************************************************/

#include <stdint.h>             /* Generic int types */
#include <stddef.h>             /* Standard definitions */
#include <interrupt_settings.h> /* Interrupt settings */
#include <cpu_api.h>            /* CPU API */
#include <kernel_output.h>      /* Kernel output */

/* UTK configuration file */
#include <config.h>

/* Tests header file */
#ifdef TEST_MODE_ENABLED
#include <test_bank.h>
#endif

/* Header file */
#include <syscall.h>

/*******************************************************************************
 * GLOBAL VARIABLES
 ******************************************************************************/

static void dummy_syscall_handler(SYSCALL_FUNCTION_E func, void* param);

/** @brief Stores the handlers for each system call. */
static syscall_handler_t kernel_interrupt_handlers[SYSCALL_MAX_ID] = {
    {dummy_syscall_handler}
};

/*******************************************************************************
 * FUNCTIONS
 ******************************************************************************/

/**
 * @brief Dummy system handler, outputs the call id and parameters on the debug
 * output.
 */
static void dummy_syscall_handler(SYSCALL_FUNCTION_E func, void* param)
{
    KERNEL_DEBUG(SYSCALL_DEBUG_ENABLED, "DUMMY SYSCALL %d 0x%p", func, param);
    *(uint32_t*)param = 0xB1EB1E00;
}

/**
 * @brief Kernel's syscall interrupt handler.
 *
 * @details  Kernel's syscall interrupt handler. This function received the 
 * system call interrupt with the CPU and stack state. It then dispatch the 
 * system call to the right call.
 *
 * @param[in, out] cpu_state The cpu registers structure.
 * @param[in] int_id The interrupt number.
 * @param[in, out] stack_state The stack state before the interrupt.
 */
static void syscall_handler(cpu_state_t *cpu_state,
                            uintptr_t int_id,
                            stack_state_t *stack_state)
{
    uint32_t func;
    void*    params;

    if(int_id != SYSCALL_INT_LINE)
    {
        KERNEL_ERROR("Syscall handler called with wrong interrupt\n");
        return;
    }

    cpu_get_syscall_data(cpu_state, stack_state, &func, &params);

    KERNEL_DEBUG(SYSCALL_DEBUG_ENABLED, "Request syscall %d", func);

    if(func < SYSCALL_MAX_ID)
    {
        kernel_interrupt_handlers[func].handler(func, params);
    }
    else 
    {
        KERNEL_ERROR("Unkonwn system call ID %d\n", func);
    }    
}

OS_RETURN_E syscall_init(void)
{
    /* Init the system call handler */
    return kernel_interrupt_register_int_handler(SYSCALL_INT_LINE, 
                                                 syscall_handler);
}

OS_RETURN_E syscall_do(const SYSCALL_FUNCTION_E func, void* params)
{
    /* Checks if the system call exists */
    if(func >= SYSCALL_MAX_ID)
    {
        return OS_ERR_SYSCALL_UNKNOWN;
    }

    /* Generate the syscall */
    cpu_syscall(func, params);

    return OS_NO_ERR;
}