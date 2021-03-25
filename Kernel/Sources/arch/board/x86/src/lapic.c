/*******************************************************************************
 * @file lapic.c
 *
 * @see lapic.h
 *
 * @author Alexy Torres Aurora Dugo
 *
 * @date 08/03/2021
 *
 * @version 1.0
 *
 * @brief Local APIC (Advanced programmable interrupt controler) driver.
 *
 * @details Local APIC (Advanced programmable interrupt controler) driver.
 * Manages x86 IRQs from the IO-APIC. The driver also allow the use of the LAPIC
 * timer as a timer source. IPI (inter processor interrupt) are also possible
 * thanks to the driver.
 *
 * @warning This driver uses the PIT (Programmable interval timer) to initialize
 * the LAPIC timer. The PIT must be present and initialized to use this driver.
 *
 * @copyright Alexy Torres Aurora Dugo
 ******************************************************************************/

#include <mmio.h>                 /* Memory mapped IOs */
#include <stdint.h>               /* Generic int types */
#include <stddef.h>               /* Standard definitions */
#include <interrupt_settings.h>   /* Interrupts settings */
#include <acpi.h>                 /* ACPI driver */
#include <memmgt.h>               /* Memory management */
#include <critical.h>             /* Critical sections */
#include <kernel_output.h>        /* Output manager */
#include <time_management.h>      /* Timer factory */
#include <pit.h>                  /* PIT driver */
#include <panic.h>                /* Kernel panic */

/* UTK configuration file */
#include <config.h>

/* Tests header file */
#ifdef TEST_MODE_ENABLED
#include <test_bank.h>
#endif

/* Header file */
#include <lapic.h>

/*******************************************************************************
 * GLOBAL VARIABLES
 ******************************************************************************/

/** @brief Local APIC controller address */
static void* lapic_base_addr;

/** @brief LAPIC state */
static uint32_t initialized = 0;

/** @brief Wait interrupt flag. */
static volatile uint8_t  wait_int;

/** @brief LAPIC frequency in Hz */
static uint32_t          global_lapic_freq;

/** @brief Initial LAPIC frequency in Hz */
static uint32_t          init_lapic_timer_frequency;

/** @brief LAPIC timer driver instance. */
kernel_timer_t lapic_timer_driver = {
    .get_frequency  = lapic_timer_get_frequency,
    .set_frequency  = lapic_timer_set_frequency,
    .enable         = lapic_timer_enable,
    .disable        = lapic_timer_disable,
    .set_handler    = lapic_timer_set_handler,
    .remove_handler = lapic_timer_remove_handler,
    .get_irq        = lapic_timer_get_irq
};

/*******************************************************************************
 * FUNCTIONS
 ******************************************************************************/

/**
 * @brief Read Local APIC register, the access is a memory mapped IO.
 *
 * @param reg[in] The register of the Local APIC to read.
 * 
 * @return The value contained in the Local APIC register.
 */
__inline__ static uint32_t lapic_read(uint32_t reg)
{
    return mapped_io_read_32((void*)((uintptr_t)lapic_base_addr + reg));
}

/**
 * @brief Write Local APIC register, the acces is a memory mapped IO.
 *
 * @param reg[in] The register of the Local APIC to write.
 * @param data[in] The value to write in the register.
 */
__inline__ static void lapic_write(uint32_t reg, uint32_t data)
{
    mapped_io_write_32((void*)((uintptr_t)lapic_base_addr + reg), data);
}

/**
 * @brief LAPIC dummy handler.
 * 
 * @details LAPIC dummy handler. This handler simply acknowledge the interrut.
 * 
 * @param cpu_state[in] The cpu registers structure.
 * @param int_id[in] The interrupt number.
 * @param stack_state[in] The stack state before the interrupt that contain cs, 
 * eip, error code and the eflags register value.
 */
static void lapic_dummy_handler(cpu_state_t* cpu_state, uintptr_t int_id,
                                stack_state_t* stack_state)
{
    (void)cpu_state;
    (void)int_id;
    (void)stack_state;

    kernel_interrupt_set_irq_eoi(LAPIC_TIMER_INTERRUPT_LINE);
}

/**
 * @brief PIT interrupt initialisation handler. 
 * 
 * @details PIT interrupt initialisation handler. The PIT will trigger two 
 * interrupts to init the LAPIC timer. This is used to get the LAPIC timer 
 * frequency.
 *
 * @param cpu_state[in] The cpu registers structure.
 * @param int_id[in] The interrupt number.
 * @param stack_state[in] The stack state before the interrupt that contain cs, 
 * eip, error code and the eflags register value.
 */
static void lapic_init_pit_handler(cpu_state_t* cpu_state, uintptr_t int_id,
                                   stack_state_t* stack_state)
{
    (void)cpu_state;
    (void)int_id;
    (void)stack_state;

    if(wait_int == 1)
    {
        ++wait_int;
        /* Set LAPIC init counter to -1 */
        lapic_write(LAPIC_TICR, 0xFFFFFFFF);
    }
    else if(wait_int == 2)
    {
        /* Stop the LAPIC timer */
        lapic_write(LAPIC_TIMER, LAPIC_LVT_INT_MASKED);
        wait_int = 0;
    }

    kernel_interrupt_set_irq_eoi(PIT_IRQ_LINE);
}

OS_RETURN_E lapic_init(void)
{
    OS_RETURN_E err;

    /* Check IO-APIC support */
    if(acpi_get_io_apic_count() == 0 || acpi_get_lapic_count() == 0)
    {
        return OS_ERR_NOT_SUPPORTED;
    }

    /* Get Local APIC base address */
    lapic_base_addr = acpi_get_lapic_addr();

    /* Map the LAPIC */
    err = memory_declare_hw((uintptr_t)lapic_base_addr, KERNEL_PAGE_SIZE);
    if(err != OS_NO_ERR)
    {
        KERNEL_ERROR("Could not declare IO-APIC region\n");
        return err;
    }
    memory_mmap_direct(lapic_base_addr, lapic_base_addr, 0x1000, 0, 0, 0, 1);

    /* Enable all interrupts */
    lapic_write(LAPIC_TPR, 0);

    /* Set logical destination mode */
    lapic_write(LAPIC_DFR, 0xffffffff);
    lapic_write(LAPIC_LDR, 0x01000000);

    /* Spurious Interrupt Vector Register */
    lapic_write(LAPIC_SVR, 0x100 | LAPIC_SPURIOUS_INT_LINE);

    KERNEL_DEBUG(LAPIC_DEBUG_ENABLED, "[LAPIC] Initialized");

#ifdef TEST_MODE_ENABLED
    lapic_test();
    lapic_test2();
#endif

    initialized = 1;

    return OS_NO_ERR;
}

int32_t lapic_get_id(void)
{
    /* Check IO-APIC support */
    if(initialized == 0)
    {
        return -1;
    }

    return (lapic_read(LAPIC_ID) >> 24);
}

OS_RETURN_E lapic_send_ipi_init(const uint32_t lapic_id)
{
    OS_RETURN_E err;
    uint32_t    int_state;

    KERNEL_DEBUG(LAPIC_DEBUG_ENABLED, "[LAPIC] Send INIT IPI");

    /* Check IO-APIC support */
    if(initialized == 0)
    {
        return OS_ERR_NOT_SUPPORTED;
    }

    /* Check LACPI id */
    err = acpi_check_lapic_id(lapic_id);
    if(err != OS_NO_ERR)
    {
        return err;
    }

    ENTER_CRITICAL(int_state);

    /* Send IPI */
    lapic_write(LAPIC_ICRHI, lapic_id << ICR_DESTINATION_SHIFT);
    lapic_write(LAPIC_ICRLO, ICR_INIT | ICR_PHYSICAL |
                ICR_ASSERT | ICR_EDGE | ICR_NO_SHORTHAND);

    /* Wait for pending sends */
    while ((lapic_read(LAPIC_ICRLO) & ICR_SEND_PENDING) != 0);

    EXIT_CRITICAL(int_state);

    return err;
}

OS_RETURN_E lapic_send_ipi_startup(const uint32_t lapic_id,
                                   const uint32_t vector)
{
    OS_RETURN_E err;
    uint32_t    int_state;

    KERNEL_DEBUG(LAPIC_DEBUG_ENABLED, "[LAPIC] Send STARTUP IPI");

    /* Check IO-APIC support */
    if(initialized == 0)
    {
        return OS_ERR_NOT_SUPPORTED;
    }

    /* Check LACPI id */
    err = acpi_check_lapic_id(lapic_id);
    if(err != OS_NO_ERR)
    {
        return err;
    }

    ENTER_CRITICAL(int_state);

    /* Send IPI */
    lapic_write(LAPIC_ICRHI, lapic_id << ICR_DESTINATION_SHIFT);
    lapic_write(LAPIC_ICRLO, (vector & 0xFF) | ICR_STARTUP | ICR_PHYSICAL |
                ICR_ASSERT | ICR_EDGE | ICR_NO_SHORTHAND);
    /* Wait for pending sends */
    while ((lapic_read(LAPIC_ICRLO) & ICR_SEND_PENDING) != 0)
    {}

    EXIT_CRITICAL(int_state);

    return err;
}

OS_RETURN_E lapic_send_ipi(const uint32_t lapic_id, const uint32_t vector)
{
    OS_RETURN_E err;
    uint32_t    int_state;

    KERNEL_DEBUG(LAPIC_DEBUG_ENABLED, "[LAPIC] Send IPI");

    /* Check IO-APIC support */
    if(initialized == 0)
    {
        return OS_ERR_NOT_SUPPORTED;
    }

    /* Check LACPI id */
    err = acpi_check_lapic_id(lapic_id);
    if(err != OS_NO_ERR)
    {
        return err;
    }

    ENTER_CRITICAL(int_state);

    /* Send IPI */
    lapic_write(LAPIC_ICRHI, lapic_id << ICR_DESTINATION_SHIFT);
    lapic_write(LAPIC_ICRLO, (vector & 0xFF) | ICR_PHYSICAL |
                ICR_ASSERT | ICR_EDGE | ICR_NO_SHORTHAND);

    /* Wait for pending sends */
    while ((lapic_read(LAPIC_ICRLO) & ICR_SEND_PENDING) != 0)
    {}

    EXIT_CRITICAL(int_state);

    return err;
}

void lapic_set_int_eoi(const uint32_t interrupt_line)
{
    if(interrupt_line > MAX_INTERRUPT_LINE)
    {
        KERNEL_ERROR("Could not EOI IRQ %d, IRQ not found\n", interrupt_line);
        KERNEL_PANIC(OS_ERR_NO_SUCH_IRQ_LINE);
    }

    lapic_write(LAPIC_EOI, 0);

    KERNEL_DEBUG(LAPIC_DEBUG_ENABLED, "[LAPIC] EOI %d", interrupt_line);
}


OS_RETURN_E lapic_timer_init(void)
{
    uint32_t    lapic_timer_tick_10ms;
    OS_RETURN_E err;

    KERNEL_DEBUG(LAPIC_DEBUG_ENABLED, "[LAPIC] Timer Initialization");

    /* Check IO-APIC support */
    if(initialized == 0)
    {
        return OS_ERR_NOT_SUPPORTED;
    }

    /* Init LAPIC TIMER */
    wait_int = 1;
    lapic_write(LAPIC_TDCR, LAPIC_DIVIDER_16);

    /* Set PIT period of 10 ms and handler */
    pit_set_frequency(100);

    err = pit_set_handler(lapic_init_pit_handler);
    if(err != OS_NO_ERR)
    {
        return err;
    }

    /* Wait for interrupts to gather the timer data */
    pit_enable();

    kernel_interrupt_restore(1);
    while(wait_int != 0);
    kernel_interrupt_disable();

    pit_disable();

    err = pit_remove_handler();
    if(err != OS_NO_ERR)
    {
        return err;
    }

    /* Get the count of ticks in 10ms */
    lapic_timer_tick_10ms = 0xFFFFFFFF - lapic_read(LAPIC_TCCR);

    /* Get the frequency */
    init_lapic_timer_frequency = 100 * lapic_timer_tick_10ms;

    /* Compute the new tick count */
    global_lapic_freq = init_lapic_timer_frequency / LAPIC_INIT_FREQ;

    /* Register LAPI dummy handler */
    err = kernel_interrupt_register_int_handler(LAPIC_TIMER_INTERRUPT_LINE,
                                                lapic_dummy_handler);
    if(err != OS_NO_ERR)
    {
        return err;
    }
    /* Init interrupt */
    lapic_write(LAPIC_TIMER, LAPIC_TIMER_INTERRUPT_LINE |
                LAPIC_TIMER_MODE_PERIODIC);

    /* Set new timer count */
    lapic_write(LAPIC_TDCR, LAPIC_DIVIDER_16);
    lapic_write(LAPIC_TICR, global_lapic_freq);

    lapic_set_int_eoi(LAPIC_TIMER_INTERRUPT_LINE);

#ifdef TEST_MODE_ENABLED
    lapic_timer_test();
#endif

    return err;
}

uint32_t lapic_timer_get_frequency(void)
{
    uint32_t freq;
    uint32_t int_state;

    ENTER_CRITICAL(int_state);

    freq = init_lapic_timer_frequency / global_lapic_freq;
    
    EXIT_CRITICAL(int_state);

    return freq;
}

void lapic_timer_set_frequency(const uint32_t frequency)
{
    uint32_t int_state;

    /* Check IO-APIC support */
    if(initialized == 0)
    {
        KERNEL_ERROR("Set LAPIC timer frequency before initialization\n");
        KERNEL_PANIC(OS_ERR_NOT_SUPPORTED);
    }

    if(frequency < 20 || frequency > 8000)
    {
        KERNEL_ERROR("Set LAPIC timer frequency out of bound: %d\n", frequency);
        KERNEL_PANIC(OS_ERR_OUT_OF_BOUND);
    }

    KERNEL_DEBUG(LAPIC_DEBUG_ENABLED, 
                 "[LAPIC] Timer set frequency %d", 
                 frequency);

    ENTER_CRITICAL(int_state);

    /* Compute the new tick count */
    global_lapic_freq = init_lapic_timer_frequency / frequency;

    /* Set new timer count */
    lapic_write(LAPIC_TDCR, LAPIC_DIVIDER_16);
    lapic_write(LAPIC_TICR, global_lapic_freq);

    EXIT_CRITICAL(int_state);
}

void lapic_timer_enable(void)
{
    uint32_t int_state;

    /* Check support */
    if(initialized == 0)
    {
        KERNEL_ERROR("Enable LAPIC timer frequency before initialization\n");
        KERNEL_PANIC(OS_ERR_NOT_SUPPORTED);
    }

    KERNEL_DEBUG(LAPIC_DEBUG_ENABLED, "[LAPIC] Timer enable");

    ENTER_CRITICAL(int_state);

    /* Enable interrupt */
    lapic_write(LAPIC_TIMER, LAPIC_TIMER_INTERRUPT_LINE |
                LAPIC_TIMER_MODE_PERIODIC);

    EXIT_CRITICAL(int_state);
}

void lapic_timer_disable(void)
{
    uint32_t int_state;

    /* Check support */
    if(initialized == 0)
    {
        KERNEL_ERROR("Disable LAPIC timer frequency before initialization\n");
        KERNEL_PANIC(OS_ERR_NOT_SUPPORTED);
    }

    KERNEL_DEBUG(LAPIC_DEBUG_ENABLED, "[LAPIC] Timer disable");

    ENTER_CRITICAL(int_state);

    /* Disable interrupt */
    lapic_write(LAPIC_TIMER, LAPIC_LVT_INT_MASKED);

    EXIT_CRITICAL(int_state);
}

OS_RETURN_E lapic_timer_set_handler(void(*handler)(
                                    cpu_state_t*,
                                    uintptr_t,
                                    stack_state_t*
                                    ))
{
    OS_RETURN_E err;
    uint32_t    int_state;

    /* Check support */
    if(initialized == 0)
    {
        return OS_ERR_NOT_SUPPORTED;
    }

    if(handler == NULL)
    {
        return OS_ERR_NULL_POINTER;
    }

    lapic_timer_disable();

    ENTER_CRITICAL(int_state);

    /* Remove the current handler */
    err = kernel_interrupt_remove_int_handler(LAPIC_TIMER_INTERRUPT_LINE);
    if(err != OS_NO_ERR)
    {
        EXIT_CRITICAL(int_state);
        lapic_timer_enable();
        return err;
    }

    err = kernel_interrupt_register_int_handler(LAPIC_TIMER_INTERRUPT_LINE,
                                                handler);
    if(err != OS_NO_ERR)
    {
        EXIT_CRITICAL(int_state);
        return err;
    }

    EXIT_CRITICAL(int_state);

    KERNEL_DEBUG(LAPIC_DEBUG_ENABLED, 
                 "[LAPIC] New LAPIC timer handler set (0x%p)",
                 handler);

    lapic_timer_enable();

    return OS_NO_ERR;
}

OS_RETURN_E lapic_timer_remove_handler(void)
{
    /* Check support */
    if(initialized == 0)
    {
        return OS_ERR_NOT_SUPPORTED;
    }

    KERNEL_DEBUG(LAPIC_DEBUG_ENABLED, 
                 "[LAPIC] Removed timer handler");

    return lapic_timer_set_handler(lapic_dummy_handler);
}

uint32_t lapic_timer_get_irq(void)
{
    return LAPIC_TIMER_INTERRUPT_LINE;
}