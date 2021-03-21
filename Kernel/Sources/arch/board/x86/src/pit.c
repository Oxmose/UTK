/*******************************************************************************
 * @file pit.c
 *
 * @see pit.h
 *
 * @author Alexy Torres Aurora Dugo
 *
 * @date 09/03/2021
 *
 * @version 1.0
 *
 * @brief PIT (Programmable interval timer) driver.
 *
 * @details PIT (Programmable interval timer) driver. Used as the basic timer
 * source in the kernel. This driver provides basic access to the PIT.
 *
 * @copyright Alexy Torres Aurora Dugo
 ******************************************************************************/

#include <cpu.h>                /* CPU manipulation */
#include <kernel_output.h>      /* Kernel output methods */
#include <interrupts.h>         /* Interrupts management */
#include <interrupt_settings.h> /* Interrupts settings */
#include <stdint.h>             /* Generic int types */
#include <stddef.h>             /* Standard definition */
#include <time_management.h>    /* Timer factory */
#include <critical.h>           /* Critical sections */
#include <panic.h>              /* Kernel panic */

/* UTK configuration file */
#include <config.h>

/* Tests header file */
#ifdef TEST_MODE_ENABLED
#include <test_bank.h>
#endif

/* Header include */
#include <pit.h>

/*******************************************************************************
 * GLOBAL VARIABLES
 ******************************************************************************/

/** @brief Keeps track on the PIT enabled state. */
static uint32_t disabled_nesting;

/** @brief Keeps track of the PIT tick frequency. */
static uint32_t tick_freq;

/** @brief PIT driver instance. */
kernel_timer_t pit_driver = {
    .get_frequency  = pit_get_frequency,
    .set_frequency  = pit_set_frequency,
    .enable         = pit_enable,
    .disable        = pit_disable,
    .set_handler    = pit_set_handler,
    .remove_handler = pit_remove_handler,
    .get_irq        = pit_get_irq
};

/*******************************************************************************
 * FUNCTIONS
 ******************************************************************************/

/**
 * @brief Initial PIT interrupt handler.
 *
 * @details PIT interrupt handler set at the initialization of the PIT.
 * Dummy routine setting EOI.
 *
 * @param[in, out] cpu_state The cpu registers before the interrupt.
 * @param[in] int_id The interrupt line that called the handler.
 * @param[in, out] stack_state The stack state before the interrupt.
 */
static void dummy_handler(cpu_state_t* cpu_state, uintptr_t int_id,
                          stack_state_t* stack_state)
{
    (void)cpu_state;
    (void)int_id;
    (void)stack_state;

    /* EOI */
    kernel_interrupt_set_irq_eoi(PIT_IRQ_LINE);
}

OS_RETURN_E pit_init(void)
{
    OS_RETURN_E err;

    /* Init system times */
    disabled_nesting = 1;

    /* Set PIT frequency */
    pit_set_frequency(PIT_INIT_FREQ);

    /* Set PIT interrupt handler */
    err = kernel_interrupt_register_irq_handler(PIT_IRQ_LINE, dummy_handler);
    if(err != OS_NO_ERR)
    {
        return err;
    }

    KERNEL_DEBUG(PIT_DEBUG_ENABLED, "[PIT] Initialization end");

#ifdef TEST_MODE_ENABLED
    pit_test();
    pit_test2();
    pit_test3();
#endif

    /* Enable PIT IRQ */
    pit_enable();

    return err;
}

void pit_enable(void)
{
    uint32_t int_state;

    ENTER_CRITICAL(int_state);

    if(disabled_nesting > 0)
    {
        --disabled_nesting;
    }

    KERNEL_DEBUG(PIT_DEBUG_ENABLED, 
                 "[PIT] Enable (nesting %d)", 
                 disabled_nesting);

    if(disabled_nesting == 0)
    {
        kernel_interrupt_set_irq_mask(PIT_IRQ_LINE, 1);
    }

    EXIT_CRITICAL(int_state);
}

void pit_disable(void)
{
    uint32_t    int_state;

    ENTER_CRITICAL(int_state);

    if(disabled_nesting < UINT32_MAX)
    {
        ++disabled_nesting;
    }

    KERNEL_DEBUG(PIT_DEBUG_ENABLED, 
                 "[PIT] Disable (nesting %d)", 
                 disabled_nesting);
    kernel_interrupt_set_irq_mask(PIT_IRQ_LINE, 0);

    EXIT_CRITICAL(int_state);
}

void pit_set_frequency(const uint32_t freq)
{
    uint32_t    int_state;

    if(freq < PIT_MIN_FREQ || freq > PIT_MAX_FREQ)
    {
        KERNEL_ERROR("Set PIT timer frequency out of bound: %d\n", freq);
        KERNEL_PANIC(OS_ERR_OUT_OF_BOUND);
    }

    ENTER_CRITICAL(int_state);

    /* Disable PIT IRQ */
    pit_disable();

    tick_freq  = freq;

    /* Set clock frequency */
    uint16_t tick_freq = (uint16_t)((uint32_t)PIT_QUARTZ_FREQ / freq);
    cpu_outb(PIT_COMM_SET_FREQ, PIT_COMM_PORT);
    cpu_outb(tick_freq & 0x00FF, PIT_DATA_PORT);
    cpu_outb(tick_freq >> 8, PIT_DATA_PORT);

    KERNEL_DEBUG(PIT_DEBUG_ENABLED, 
                 "[PIT] New PIT frequency set (%d)", 
                 freq);
    
    EXIT_CRITICAL(int_state);

    /* Enable PIT IRQ */
    pit_enable();
}

uint32_t pit_get_frequency(void)
{
    return tick_freq;
}

OS_RETURN_E pit_set_handler(void(*handler)(
                                 cpu_state_t*,
                                 uintptr_t,
                                 stack_state_t*
                                 ))
{
    OS_RETURN_E err;
    uint32_t    int_state;

    if(handler == NULL)
    {
        return OS_ERR_NULL_POINTER;
    }

    ENTER_CRITICAL(int_state);

    pit_disable();

    /* Remove the current handler */
    err = kernel_interrupt_remove_irq_handler(PIT_IRQ_LINE);
    if(err != OS_NO_ERR)
    {
        EXIT_CRITICAL(int_state);
        pit_enable();
        return err;
    }

    err = kernel_interrupt_register_irq_handler(PIT_IRQ_LINE, handler);
    if(err != OS_NO_ERR)
    {
        EXIT_CRITICAL(int_state);
        pit_enable();
        return err;
    }

    KERNEL_DEBUG(PIT_DEBUG_ENABLED, 
                 "[PIT] New PIT handler set at 0x%p", 
                 handler);

    EXIT_CRITICAL(int_state);
    pit_enable();

    return err;
}

OS_RETURN_E pit_remove_handler(void)
{
    KERNEL_DEBUG(PIT_DEBUG_ENABLED, 
                 "[PIT] Default PIT handler set at 0x%p", 
                 dummy_handler);

    return pit_set_handler(dummy_handler);
}

uint32_t pit_get_irq(void)
{
    return PIT_IRQ_LINE;
}