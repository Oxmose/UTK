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
#include <interrupt_settings.h> /* Interrupt settings */
#include <cpu_api.h>            /* CPU API */
#include <kernel_output.h>      /* Kernel output */
#include <scheduler.h>          /* Scheduler */
#include <kernel_error.h>       /* Kernel error codes */
#include <panic.h>              /* Kernel panic */
#include <memmgt.h>             /* Memory management API */
#include <futex.h>              /* Futex API */
/* UTK configuration file */
#include <config.h>

/* Tests header file */
#ifdef TEST_MODE_ENABLED
#include <test_bank.h>
#endif

/* Header file */
#include <syscall.h>

/*******************************************************************************
 * CONSTANTS
 ******************************************************************************/

/* None */

/*******************************************************************************
 * STRUCTURES
 ******************************************************************************/

/* None */

/*******************************************************************************
 * GLOBAL VARIABLES
 ******************************************************************************/

/** @brief Stores the handlers for each system call. */
static syscall_handler_t kernel_interrupt_handlers[SYSCALL_MAX_ID] = {
    {sched_fork_process},             /* SYSCALL_FORK */
    {sched_wait_process_pid},         /* SYSCALL_WAITPID */
    {NULL},                           /* SYSCALL_EXIT */
    {futex_wait},                     /* SYSCALL_FUTEX_WAIT */
    {futex_wake},                     /* SYSCALL_FUTEX_WAKE */
    {sched_get_process_params},       /* SYSCALL_SCHED_GET_PARAMS */
    {sched_set_process_params},       /* SYSCALL_SCHED_SET_PARAMS */
    {memory_alloc_page},              /* SYSCALL_PAGE_ALLOC */
};

/*******************************************************************************
 * STATIC FUNCTIONS DECLARATION
 ******************************************************************************/

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
                            stack_state_t *stack_state);

/*******************************************************************************
 * FUNCTIONS
 ******************************************************************************/

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

    KERNEL_DEBUG(SYSCALL_DEBUG_ENABLED, "[SYSCALL] Request syscall %d", func);

    if(func < SYSCALL_MAX_ID)
    {
        if(kernel_interrupt_handlers[func].handler == NULL)
        {
            KERNEL_ERROR("Tried to call un unknown SYSCALL: %d\n", func);
            KERNEL_PANIC(OS_ERR_SYSCALL_UNKNOWN);
        }
        kernel_interrupt_handlers[func].handler(func, params);
    }
    else
    {
        KERNEL_ERROR("Unknown system call ID %d\n", func);
    }
}

void syscall_init(void)
{
    OS_RETURN_E err;
    /* Init the system call handler */
    err = kernel_interrupt_register_int_handler(SYSCALL_INT_LINE,
                                                syscall_handler);
    if(err != OS_NO_ERR)
    {
        KERNEL_ERROR("Could not initialize system call manager\n");
        KERNEL_PANIC(err);
    }
}