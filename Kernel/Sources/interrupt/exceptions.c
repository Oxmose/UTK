/*******************************************************************************
 * @file exceptions.c
 * 
 * @see exceptions.h
 *
 * @author Alexy Torres Aurora Dugo
 *
 * @date 27/05/2018
 *
 * @version 2.0
 *
 * @brief Exceptions manager. 
 * 
 * @warning These functions must be called during or after the interrupts are 
 * set.
 * 
 * @details Exception manager. Allows to attach ISR to exceptions lines.
 * 
 * @copyright Alexy Torres Aurora Dugo
 ******************************************************************************/


#include <lib/stdint.h>         /* Generic int types */
#include <lib/stddef.h>         /* Standard definitions */
#include <core/panic.h>         /* Kernel panic */
#include <core/scheduler.h>     /* Kernel scheduler */
#include <cpu_settings.h>       /* CPU settings */
#include <interrupt_settings.h> /* CPU interrupts settings */
#include <cpu.h>                /* CPU management */
#include <io/kernel_output.h>   /* Kernel output methods */

/* UTK configuration file */
#include <config.h>

/* Tests header file */
#if TEST_MODE_ENABLED
#include <Tests/test_bank.h>
#endif

/* Header file */
#include <interrupt/exceptions.h>

/*******************************************************************************
 * GLOBAL VARIABLES
 ******************************************************************************/

/** @brief Stores the handlers for each exception, defined in exceptions.h */
extern custom_handler_t kernel_interrupt_handlers[INT_ENTRY_COUNT];

#if MAX_CPU_COUNT > 1
/** @brief Critical section spinlock. */
static spinlock_t lock = SPINLOCK_INIT_VALUE;
#endif

/*******************************************************************************
 * FUNCTIONS
 ******************************************************************************/

/**
 * @brief Handle a division by zero exception.
 *
 * @details Handles a divide by zero exception raised by the cpu. The thread
 * will just be killed.
 *
 * @param cpu_state The cpu registers structure.
 * @param int_id The exception number.
 * @param stack_state The stack state before the exception that contain cs, eip,
 * error code and the eflags register value.
 */
static void div_by_zero_handler(cpu_state_t* cpu_state, uintptr_t int_id,
                                stack_state_t* stack_state)
{
    (void)cpu_state;

    /* If the exception line is not the divide by zero exception */
    if(int_id != DIV_BY_ZERO_LINE)
    {
        kernel_error("Divide by zero handler in wrong exception line.\n");
        panic(cpu_state, int_id, stack_state);
    }
    sched_set_thread_termination_cause(THREAD_TERMINATE_CAUSE_DIV_BY_ZERO);
    cpu_set_next_thread_instruction(cpu_state, stack_state, 
                                    (uintptr_t)sched_terminate_thread);
    
}

OS_RETURN_E kernel_exception_init(void)
{
    OS_RETURN_E err;

#if EXCEPTION_KERNEL_DEBUG == 1
    kernel_serial_debug("Initializing exception manager.\n");
#endif

    err = kernel_exception_register_handler(DIV_BY_ZERO_LINE,
                                            div_by_zero_handler);
    if(err != OS_NO_ERR)
    {
        return err;
    }

#if TEST_MODE_ENABLED 
    exception_test();
#endif

    return OS_NO_ERR;
}

OS_RETURN_E kernel_exception_register_handler(const uint32_t exception_line,
                                       void(*handler)(
                                             cpu_state_t*,
                                             uintptr_t,
                                             stack_state_t*
                                             )
                                       )
{
    uint32_t int_state;

    if((int32_t)exception_line < MIN_EXCEPTION_LINE ||
       exception_line > MAX_EXCEPTION_LINE)
    {
        return OR_ERR_UNAUTHORIZED_INTERRUPT_LINE;
    }

    if(handler == NULL)
    {
        return OS_ERR_NULL_POINTER;
    }

#if MAX_CPU_COUNT > 1
    ENTER_CRITICAL(int_state, &lock);
#else
    ENTER_CRITICAL(int_state);
#endif

    if(kernel_interrupt_handlers[exception_line].handler != NULL)
    {

#if MAX_CPU_COUNT > 1
        EXIT_CRITICAL(int_state, &lock);
#else
        EXIT_CRITICAL(int_state);
#endif

        return OS_ERR_INTERRUPT_ALREADY_REGISTERED;
    }

    kernel_interrupt_handlers[exception_line].handler = handler;
    kernel_interrupt_handlers[exception_line].enabled = 1;

#if EXCEPTION_KERNEL_DEBUG == 1
    kernel_serial_debug("Added exception %u handler at 0x%p\n",
                        exception_line, handler);
#endif

#if MAX_CPU_COUNT > 1
    EXIT_CRITICAL(int_state, &lock);
#else
    EXIT_CRITICAL(int_state);
#endif

    return OS_NO_ERR;
}

OS_RETURN_E kernel_exception_remove_handler(const uint32_t exception_line)
{
    uint32_t int_state;

    if((int32_t)exception_line < MIN_EXCEPTION_LINE ||
       exception_line > MAX_EXCEPTION_LINE)
    {
        return OR_ERR_UNAUTHORIZED_INTERRUPT_LINE;
    }

#if MAX_CPU_COUNT > 1
    ENTER_CRITICAL(int_state, &lock);
#else
    ENTER_CRITICAL(int_state);
#endif

    if(kernel_interrupt_handlers[exception_line].handler == NULL)
    {

#if MAX_CPU_COUNT > 1
        EXIT_CRITICAL(int_state, &lock);
#else
        EXIT_CRITICAL(int_state);
#endif

        return OS_ERR_INTERRUPT_NOT_REGISTERED;
    }

    kernel_interrupt_handlers[exception_line].handler = NULL;
    kernel_interrupt_handlers[exception_line].enabled = 0;

#if EXCEPTION_KERNEL_DEBUG == 1
    kernel_serial_debug("Removed exception %u handle\n", exception_line);
#endif

#if MAX_CPU_COUNT > 1
    EXIT_CRITICAL(int_state, &lock);
#else
    EXIT_CRITICAL(int_state);
#endif

    return OS_NO_ERR;
}