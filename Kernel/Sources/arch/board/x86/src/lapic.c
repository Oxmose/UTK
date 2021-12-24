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

/*******************************************************************************
 * INCLUDES
 ******************************************************************************/

/* Included headers */
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
#include <kernel_error.h>         /* Kernel error codes */

/* Configuration files */
#include <config.h>
#include <test_bank.h>

/* Header file */
#include <lapic.h>

/*******************************************************************************
 * CONSTANTS
 ******************************************************************************/

/** @brief LAPIC ID register's offset. */
#define LAPIC_ID                        0x0020
/** @brief LAPIC version register's offset. */
#define LAPIC_VER                       0x0030
/** @brief LAPIC trask priority register's offset. */
#define LAPIC_TPR                       0x0080
/** @brief LAPIC arbitration policy register's offset. */
#define LAPIC_APR                       0x0090
/** @brief LAPIC processor priority register's offset. */
#define LAPIC_PPR                       0x00A0
/** @brief LAPIC EOI register's offset. */
#define LAPIC_EOI                       0x00B0
/** @brief LAPIC remote read register's offset. */
#define LAPIC_RRD                       0x00C0
/** @brief LAPIC logical destination register's offset. */
#define LAPIC_LDR                       0x00D0
/** @brief LAPIC destination format register's offset. */
#define LAPIC_DFR                       0x00E0
/** @brief LAPIC Spurious interrupt vector register's offset. */
#define LAPIC_SVR                       0x00F0
/** @brief LAPIC in service register's offset. */
#define LAPIC_ISR                       0x0100
/** @brief LAPIC trigger mode register's offset. */
#define LAPIC_TMR                       0x0180
/** @brief LAPIC interrupt request register's offset. */
#define LAPIC_IRR                       0x0200
/** @brief LAPIC error status register's offset. */
#define LAPIC_ESR                       0x0280
/** @brief LAPIC interrupt command (low) register's offset. */
#define LAPIC_ICRLO                     0x0300
/** @brief LAPIC interrupt command (high) register's offset. */
#define LAPIC_ICRHI                     0x0310
/** @brief LAPIC local vector table timer register's offset. */
#define LAPIC_TIMER                     0x0320
/** @brief LAPIC local vector table thermal sensor register's offset. */
#define LAPIC_THERMAL                   0x0330
/** @brief LAPIC local vector table PMC register's offset. */
#define LAPIC_PERF                      0x0340
/** @brief LAPIC local vector table lint0 register's offset. */
#define LAPIC_LINT0                     0x0350
/** @brief LAPIC local vector table lint1 register's offset. */
#define LAPIC_LINT1                     0x0360
/** @brief LAPIC local vector table error register's offset. */
#define LAPIC_ERROR                     0x0370
/** @brief LAPIC timer initial count register's offset. */
#define LAPIC_TICR                      0x0380
/** @brief LAPIC timer current count register's offset. */
#define LAPIC_TCCR                      0x0390
/** @brief LAPIC timer devide configuration register's offset. */
#define LAPIC_TDCR                      0x03E0

/* Delivery Mode */
/** @brief LAPIC delivery mode fixed. */
#define ICR_FIXED                       0x00000000
/** @brief LAPIC delivery mode lowest priority. */
#define ICR_LOWEST                      0x00000100
/** @brief LAPIC delivery mode SMI. */
#define ICR_SMI                         0x00000200
/** @brief LAPIC delivery mode NMI. */
#define ICR_NMI                         0x00000400
/** @brief LAPIC delivery mode init IPI. */
#define ICR_INIT                        0x00000500
/** @brief LAPIC delivery mode startup IPI. */
#define ICR_STARTUP                     0x00000600
/** @brief LAPIC delivery mode external. */
#define ICR_EXTERNAL                    0x00000700

/** @brief LAPIC destination mode physical. */
#define ICR_PHYSICAL                    0x00000000
/** @brief LAPIC destination mode logical. */
#define ICR_LOGICAL                     0x00000800

/** @brief LAPIC Delivery status idle. */
#define ICR_IDLE                        0x00000000
/** @brief LAPIC Delivery status pending. */
#define ICR_SEND_PENDING                0x00001000

/** @brief LAPIC Level deassert enable flag. */
#define ICR_DEASSERT                    0x00000000
/** @brief LAPIC Level deassert disable flag. */
#define ICR_ASSERT                      0x00004000

/** @brief LAPIC trigger mode edge. */
#define ICR_EDGE                        0x00000000
/** @brief LAPIC trigger mode level. */
#define ICR_LEVEL                       0x00008000

/** @brief LAPIC destination shorthand none. */
#define ICR_NO_SHORTHAND                0x00000000
/** @brief LAPIC destination shorthand self only. */
#define ICR_SELF                        0x00040000
/** @brief LAPIC destination shorthand all and self. */
#define ICR_ALL_INCLUDING_SELF          0x00080000
/** @brief LAPIC destination shorthand all but self. */
#define ICR_ALL_EXCLUDING_SELF          0x000C0000

/** @brief LAPIC destination flag shift. */
#define ICR_DESTINATION_SHIFT           24

/** @brief LAPIC Timer mode flag: periodic. */
#define LAPIC_TIMER_MODE_PERIODIC       0x20000
/** @brief LAPIC Timer divider value. */
#define LAPIC_DIVIDER_16                0x3
/** @brief LAPIC Timer initial frequency. */
#define LAPIC_INIT_FREQ                 100
/** @brief LAPIC Timer vector interrupt masked. */
#define LAPIC_LVT_INT_MASKED            0x10000

/*******************************************************************************
 * STRUCTURES AND TYPES
 ******************************************************************************/

/* None */

/*******************************************************************************
 * MACROS
 ******************************************************************************/

#define LAPIC_ASSERT(COND, MSG, ERROR) {                    \
    if((COND) == FALSE)                                     \
    {                                                       \
        PANIC(ERROR, "LAPIC", MSG, TRUE);                   \
    }                                                       \
}

/*******************************************************************************
 * GLOBAL VARIABLES
 ******************************************************************************/

/************************* Imported global variables **************************/
/* None */

/************************* Exported global variables **************************/
/* None */

/************************** Static global variables ***************************/
/** @brief Local APIC controller address */
static void* lapic_base_addr;

/** @brief LAPIC state */
static bool_t initialized = FALSE;

/** @brief Wait interrupt flag. */
static volatile uint8_t wait_int;

/** @brief LAPIC frequency in Hz */
static uint32_t global_lapic_freq;

/** @brief Initial LAPIC frequency in Hz */
static uint32_t init_lapic_timer_frequency;

/** @brief LAPIC timer driver instance. */
static kernel_timer_t lapic_timer_driver = {
    .get_frequency  = lapic_timer_get_frequency,
    .set_frequency  = lapic_timer_set_frequency,
    .enable         = lapic_timer_enable,
    .disable        = lapic_timer_disable,
    .set_handler    = lapic_timer_set_handler,
    .remove_handler = lapic_timer_remove_handler,
    .get_irq        = lapic_timer_get_irq
};

/*******************************************************************************
 * STATIC FUNCTIONS DECLARATIONS
 ******************************************************************************/

/**
 * @brief Read Local APIC register, the access is a memory mapped IO.
 *
 * @param reg[in] The register of the Local APIC to read.
 *
 * @return The value contained in the Local APIC register.
 */
inline static uint32_t lapic_read(uint32_t reg);

/**
 * @brief Write Local APIC register, the acces is a memory mapped IO.
 *
 * @param reg[in] The register of the Local APIC to write.
 * @param data[in] The value to write in the register.
 */
inline static void lapic_write(uint32_t reg, uint32_t data);

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
static void lapic_dummy_handler(cpu_state_t* cpu_state,
                                uintptr_t int_id,
                                stack_state_t* stack_state);

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
static void lapic_init_pit_handler(cpu_state_t* cpu_state,
                                   uintptr_t int_id,
                                   stack_state_t* stack_state);

/*******************************************************************************
 * FUNCTIONS
 ******************************************************************************/

inline static uint32_t lapic_read(uint32_t reg)
{
    return mapped_io_read_32((void*)((uintptr_t)lapic_base_addr + reg));
}

inline static void lapic_write(uint32_t reg, uint32_t data)
{
    mapped_io_write_32((void*)((uintptr_t)lapic_base_addr + reg), data);
}

static void lapic_dummy_handler(cpu_state_t* cpu_state,
                                uintptr_t int_id,
                                stack_state_t* stack_state)
{
    (void)cpu_state;
    (void)int_id;
    (void)stack_state;

    kernel_interrupt_set_irq_eoi(LAPIC_TIMER_INTERRUPT_LINE);
}

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

void lapic_init(void)
{
    OS_RETURN_E err;

    /* Check LAPIC support */
    LAPIC_ASSERT((acpi_get_io_apic_count() != 0 && acpi_get_lapic_count() != 0),
                 "LAPIC is not supported",
                 OS_ERR_NOT_SUPPORTED);

    /* Get Local APIC base address */
    lapic_base_addr = acpi_get_lapic_addr();

    /* Map the LAPIC */
    err = memory_declare_hw((uintptr_t)lapic_base_addr, KERNEL_PAGE_SIZE);
    LAPIC_ASSERT(err == OS_NO_ERR,
                 "Could not declare LAPIC region",
                 err);

    memory_mmap_direct(lapic_base_addr,
                       lapic_base_addr,
                       0x1000,
                       0,
                       0,
                       0,
                       1,
                       NULL);

    /* Enable all interrupts */
    lapic_write(LAPIC_TPR, 0);

    /* Set logical destination mode */
    lapic_write(LAPIC_DFR, 0xffffffff);
    lapic_write(LAPIC_LDR, 0x01000000);

    /* Spurious Interrupt Vector Register */
    lapic_write(LAPIC_SVR, 0x100 | LAPIC_SPURIOUS_INT_LINE);

    KERNEL_DEBUG(LAPIC_DEBUG_ENABLED, "[LAPIC] Initialized");

    KERNEL_TEST_POINT(lapic_test);
    KERNEL_TEST_POINT(lapic_test2);

    initialized = TRUE;
}

int32_t lapic_get_id(void)
{
    /* Check LAPIC support */
    if(initialized == FALSE)
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

    /* Check LAPIC support */
    if(initialized == FALSE)
    {
        return OS_ERR_NOT_INITIALIZED;
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
    while((lapic_read(LAPIC_ICRLO) & ICR_SEND_PENDING) != 0){}

    EXIT_CRITICAL(int_state);

    return err;
}

OS_RETURN_E lapic_send_ipi_startup(const uint32_t lapic_id,
                                   const uint32_t vector)
{
    OS_RETURN_E err;
    uint32_t    int_state;

    KERNEL_DEBUG(LAPIC_DEBUG_ENABLED, "[LAPIC] Send STARTUP IPI");

    /* Check LAPIC support */
    if(initialized == FALSE)
    {
        return OS_ERR_NOT_INITIALIZED;
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
    while ((lapic_read(LAPIC_ICRLO) & ICR_SEND_PENDING) != 0){}

    EXIT_CRITICAL(int_state);

    return err;
}

OS_RETURN_E lapic_send_ipi(const uint32_t lapic_id, const uint32_t vector)
{
    OS_RETURN_E err;
    uint32_t    int_state;

    KERNEL_DEBUG(LAPIC_DEBUG_ENABLED, "[LAPIC] Send IPI");

    /* Check LAPIC support */
    if(initialized == FALSE)
    {
        return OS_ERR_NOT_INITIALIZED;
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
    while((lapic_read(LAPIC_ICRLO) & ICR_SEND_PENDING) != 0){}

    EXIT_CRITICAL(int_state);

    return err;
}

void lapic_set_int_eoi(const uint32_t interrupt_line)
{
    LAPIC_ASSERT(interrupt_line <= MAX_INTERRUPT_LINE,
                 "Could not EOI IRQ (IRQ line to big)",
                 OS_ERR_NO_SUCH_IRQ);

    lapic_write(LAPIC_EOI, 0);

    KERNEL_DEBUG(LAPIC_DEBUG_ENABLED, "[LAPIC] EOI %d", interrupt_line);
}


void lapic_timer_init(void)
{
    uint32_t    lapic_timer_tick_10ms;
    OS_RETURN_E err;

    KERNEL_DEBUG(LAPIC_DEBUG_ENABLED, "[LAPIC] Timer Initialization");

    /* Check LAPIC support */
    LAPIC_ASSERT(initialized == TRUE,
                 "LAPIC not initialized",
                 OS_ERR_NOT_INITIALIZED);

    /* Init LAPIC TIMER */
    wait_int = 1;
    lapic_write(LAPIC_TDCR, LAPIC_DIVIDER_16);

    /* Set PIT period of 10 ms and handler */
    pit_set_frequency(100);

    err = pit_set_handler(lapic_init_pit_handler);
    LAPIC_ASSERT(err == OS_NO_ERR,
                 "Could not set PIT handler",
                 err);

    /* Wait for interrupts to gather the timer data */
    pit_enable();

    kernel_interrupt_restore(1);
    while(wait_int != 0){}
    kernel_interrupt_disable();

    pit_disable();

    err = pit_remove_handler();
    LAPIC_ASSERT(err == OS_NO_ERR,
                 "Could not remove PIT handler",
                 err);

    /* Get the count of ticks in 10ms */
    lapic_timer_tick_10ms = 0xFFFFFFFF - lapic_read(LAPIC_TCCR);

    /* Get the frequency */
    init_lapic_timer_frequency = 100 * lapic_timer_tick_10ms;

    /* Compute the new tick count */
    global_lapic_freq = init_lapic_timer_frequency / LAPIC_INIT_FREQ;

    /* Register LAPI dummy handler */
    err = kernel_interrupt_register_int_handler(LAPIC_TIMER_INTERRUPT_LINE,
                                                lapic_dummy_handler);
    LAPIC_ASSERT(err == OS_NO_ERR,
                 "Could not set LAPIC TIMER handler",
                 err);

    /* Init interrupt */
    lapic_write(LAPIC_TIMER, LAPIC_TIMER_INTERRUPT_LINE |
                LAPIC_TIMER_MODE_PERIODIC);

    /* Set new timer count */
    lapic_write(LAPIC_TDCR, LAPIC_DIVIDER_16);
    lapic_write(LAPIC_TICR, global_lapic_freq);

    lapic_set_int_eoi(LAPIC_TIMER_INTERRUPT_LINE);

    KERNEL_TEST_POINT(lapic_timer_test);
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

    /* Check LAPIC support */
    LAPIC_ASSERT(initialized == TRUE,
                 "Cannot set LAPIC timer frequency before initialization",
                 OS_ERR_NOT_INITIALIZED);

    LAPIC_ASSERT((frequency >= 20 && frequency <= 8000),
                 "LAPIC timer frequency out of bound",
                 OS_ERR_INCORRECT_VALUE);

    KERNEL_DEBUG(LAPIC_DEBUG_ENABLED,
                 "[LAPIC] Timer set frequency %d",
                 frequency);

    /* Compute the new tick count */
    global_lapic_freq = init_lapic_timer_frequency / frequency;

    ENTER_CRITICAL(int_state);

    /* Set new timer count */
    lapic_write(LAPIC_TDCR, LAPIC_DIVIDER_16);
    lapic_write(LAPIC_TICR, global_lapic_freq);

    EXIT_CRITICAL(int_state);
}

void lapic_timer_enable(void)
{
    uint32_t int_state;

    /* Check support */
    LAPIC_ASSERT(initialized == TRUE,
                 "Tried to enable LAPIC timer before initialization",
                 OS_ERR_NOT_INITIALIZED);

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
    LAPIC_ASSERT(initialized == TRUE,
                 "Tried to disable LAPIC timer before initialization",
                 OS_ERR_NOT_INITIALIZED);

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
    if(initialized == FALSE)
    {
        return OS_ERR_NOT_INITIALIZED;
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
        lapic_timer_enable();
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
    if(initialized == FALSE)
    {
        return OS_ERR_NOT_INITIALIZED;
    }

    KERNEL_DEBUG(LAPIC_DEBUG_ENABLED,
                 "[LAPIC] Removed timer handler");

    return lapic_timer_set_handler(lapic_dummy_handler);
}

uint32_t lapic_timer_get_irq(void)
{
    return LAPIC_TIMER_INTERRUPT_LINE;
}

const kernel_timer_t* lapic_timer_get_driver(void)
{
    return &lapic_timer_driver;
}

/************************************ EOF *************************************/