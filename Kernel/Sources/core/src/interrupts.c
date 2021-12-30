/*******************************************************************************
 * @file interrupts.c
 *
 * @see interrupts.h
 *
 * @author Alexy Torres Aurora Dugo
 *
 * @date 14/12/2017
 *
 * @version 2
 *
 * @brief Interrupt manager.
 *
 * @details Interrupt manager. Allows to attach ISR to interrupt lines and
 * manage IRQ used by the CPU. We also define the general interrupt handler
 * here.
 *
 * @copyright Alexy Torres Aurora Dugo
 ******************************************************************************/
/*******************************************************************************
 * INCLUDES
 ******************************************************************************/

/* Included headers */
#include <stdint.h>             /* Generic int types */
#include <stddef.h>             /* Standard definitions */
#include <string.h>             /* String manipulation */
#include <cpu_settings.h>       /* CPU settings */
#include <cpu_settings.h>       /* CPU structures */
#include <cpu_api.h>            /* CPU management */
#include <interrupt_settings.h> /* CPU interrupts settings */
#include <panic.h>              /* Kernel panic */
#include <kernel_output.h>      /* Kernel output methods */
#include <critical.h>           /* Critical sections */
#include <scheduler.h>          /* Kernel scheduler */

/* Configuration files */
#include <config.h>
#include <test_bank.h>

/* Header file */
#include <interrupts.h>

/*******************************************************************************
 * CONSTANTS
 ******************************************************************************/

/* None */

/*******************************************************************************
 * STRUCTURES AND TYPES
 ******************************************************************************/

/* None */

/*******************************************************************************
 * MACROS
 ******************************************************************************/

/* None */

/*******************************************************************************
 * GLOBAL VARIABLES
 ******************************************************************************/

/************************* Imported global variables **************************/
/* None */

/************************* Exported global variables **************************/
/** @brief Stores the handlers for each interrupt. not static because used
 * by the exceptions module.
 */
custom_handler_t kernel_interrupt_handlers[INT_ENTRY_COUNT];

/************************** Static global variables ***************************/
/** @brief The current interrupt driver to be used by the kernel. */
static interrupt_driver_t interrupt_driver;

/** @brief Stores the number of spurious interrupts since the initialization of
 * the kernel.
 */
static uint32_t spurious_interrupt;

/*******************************************************************************
 * STATIC FUNCTIONS DECLARATIONS
 ******************************************************************************/

/**
 * @brief Initial placeholder for the IRQ mask driver.
 *
 * @param irq_number Unused.
 * @param enabled Unused.
 */
static void init_driver_set_irq_mask(const uint32_t irq_number,
                                     const bool_t enabled);

/**
 * @brief Initial placeholder for the IRQ EOI driver.
 *
 * @param irq_number Unused.
 */
static void init_driver_set_irq_eoi(const uint32_t irq_number);

/**
 * @brief Initial placeholder for the spurious handler driver.
 *
 * @param int_number Unused.
 */
static INTERRUPT_TYPE_E init_driver_handle_spurious(const uint32_t int_number);

/**
 * @brief Initial placeholder for the get int line driver.
 *
 * @param irq_number Unused.
 */
static int32_t init_driver_get_irq_int_line(const uint32_t irq_number);

/**
 * @brief Kernel's spurious interrupt handler.
 *
 * @details Spurious interrupt handler. This function should only be
 * called by an assembly interrupt handler. The function will handle spurious
 * interrupts.
 */
static void spurious_handler(void);

/*******************************************************************************
 * FUNCTIONS
 ******************************************************************************/

static void init_driver_set_irq_mask(const uint32_t irq_number,
                                     const bool_t enabled)
{
    (void)irq_number;
    (void)enabled;
}

static void init_driver_set_irq_eoi(const uint32_t irq_number)
{
    (void)irq_number;
}

static INTERRUPT_TYPE_E init_driver_handle_spurious(const uint32_t int_number)
{
    (void)int_number;
    return INTERRUPT_TYPE_REGULAR;
}

static int32_t init_driver_get_irq_int_line(const uint32_t irq_number)
{
    (void)irq_number;
    return 0;
}

static void spurious_handler(void)
{
    KERNEL_DEBUG(INTERRUPTS_DEBUG_ENABLED,
                 "[INTERRUPTS] Spurious interrupt %d",
                 spurious_interrupt);

    ++spurious_interrupt;

    return;
}

void kernel_interrupt_handler(cpu_state_t cpu_state,
                              uintptr_t int_id,
                              stack_state_t stack_state)
{
    kernel_thread_t* curr_thread;
    void(*handler)(cpu_state_t*, uintptr_t, stack_state_t*);

    /* Save the thread context if exists */
    curr_thread = sched_get_current_thread();
    if(curr_thread != NULL)
    {
        cpu_save_context(&cpu_state, &stack_state, curr_thread);
    }

    /* If interrupts are disabled */
    if(cpu_get_saved_interrupt_state(&cpu_state, &stack_state) == 0 &&
       int_id != PANIC_INT_LINE &&
       int_id != SCHEDULER_SW_INT_LINE &&
       int_id >= MIN_INTERRUPT_LINE)
    {
        KERNEL_DEBUG(INTERRUPTS_DEBUG_ENABLED,
                     "[INTERRUPTS] Blocked interrupt %u",
                     int_id);
        return;
    }

    if(int_id == PANIC_INT_LINE)
    {
        panic_handler(&cpu_state, int_id, &stack_state);
    }

    KERNEL_DEBUG(INTERRUPTS_DEBUG_ENABLED,
                 "[INTERRUPTS] Interrupt %d",
                 int_id);

    /* Check for spurious interrupt */
    if(interrupt_driver.driver_handle_spurious(int_id) ==
       INTERRUPT_TYPE_SPURIOUS)
    {
        spurious_handler();
        return;
    }

    KERNEL_DEBUG(INTERRUPTS_DEBUG_ENABLED,
                 "[INTERRUPTS] Non spurious %d",
                 int_id);

    /* Select custom handlers */
    if(int_id < INT_ENTRY_COUNT &&
       kernel_interrupt_handlers[int_id].handler != NULL)
    {
        handler = kernel_interrupt_handlers[int_id].handler;
    }
    else
    {
        handler = panic_handler;
    }

    /* Execute the handler */
    handler(&cpu_state, int_id, &stack_state);
}

void kernel_interrupt_init(void)
{
    KERNEL_DEBUG(INTERRUPTS_DEBUG_ENABLED,
                 "[INTERRUPTS] Initializing interrupt manager.");

    /* Blank custom interrupt handlers */
    memset(kernel_interrupt_handlers,
           0,
           sizeof(custom_handler_t) * INT_ENTRY_COUNT);

    /* Attach the special PANIC interrupt for when we don't know what to do */
    kernel_interrupt_handlers[PANIC_INT_LINE].handler = panic_handler;

    /* Init state */
    kernel_interrupt_disable();
    spurious_interrupt = 0;

    /* Init driver */
    interrupt_driver.driver_get_irq_int_line = init_driver_get_irq_int_line;
    interrupt_driver.driver_handle_spurious  = init_driver_handle_spurious;
    interrupt_driver.driver_set_irq_eoi      = init_driver_set_irq_eoi;
    interrupt_driver.driver_set_irq_mask     = init_driver_set_irq_mask;

    KERNEL_TEST_POINT(interrupt_test);
}

OS_RETURN_E kernel_interrupt_set_driver(const interrupt_driver_t* driver)
{
    uint32_t int_state;

    if(driver == NULL ||
       driver->driver_set_irq_eoi == NULL ||
       driver->driver_set_irq_mask == NULL ||
       driver->driver_handle_spurious == NULL ||
       driver->driver_get_irq_int_line == NULL)
    {
        return OS_ERR_NULL_POINTER;
    }

    ENTER_CRITICAL(int_state);

    interrupt_driver = *driver;

    EXIT_CRITICAL(int_state);

    KERNEL_DEBUG(INTERRUPTS_DEBUG_ENABLED,
                 "[INTERRUPTS] Set new interrupt driver at 0x%p.",
                 driver);

    return OS_NO_ERR;
}

OS_RETURN_E kernel_interrupt_register_int_handler(const uint32_t interrupt_line,
                                       void(*handler)(
                                             cpu_state_t*,
                                             uintptr_t,
                                             stack_state_t*
                                             )
                                       )
{
    uint32_t int_state;

    if(interrupt_line < MIN_INTERRUPT_LINE ||
       interrupt_line > MAX_INTERRUPT_LINE)
    {
        return OR_ERR_UNAUTHORIZED_INTERRUPT_LINE;
    }

    if(handler == NULL)
    {
        return OS_ERR_NULL_POINTER;
    }

    ENTER_CRITICAL(int_state);

    if(kernel_interrupt_handlers[interrupt_line].handler != NULL)
    {
        EXIT_CRITICAL(int_state);
        return OS_ERR_INTERRUPT_ALREADY_REGISTERED;
    }

    kernel_interrupt_handlers[interrupt_line].handler = handler;

    KERNEL_DEBUG(INTERRUPTS_DEBUG_ENABLED,
                 "[INTERRUPTS] Added INT %u handler at 0x%p",
                 interrupt_line,
                 handler);

    EXIT_CRITICAL(int_state);

    return OS_NO_ERR;
}

OS_RETURN_E kernel_interrupt_remove_int_handler(const uint32_t interrupt_line)
{
    uint32_t int_state;

    if(interrupt_line < MIN_INTERRUPT_LINE ||
       interrupt_line > MAX_INTERRUPT_LINE)
    {
        return OR_ERR_UNAUTHORIZED_INTERRUPT_LINE;
    }

    ENTER_CRITICAL(int_state);

    if(kernel_interrupt_handlers[interrupt_line].handler == NULL)
    {
        EXIT_CRITICAL(int_state);
        return OS_ERR_INTERRUPT_NOT_REGISTERED;
    }

    kernel_interrupt_handlers[interrupt_line].handler = NULL;

    KERNEL_DEBUG(INTERRUPTS_DEBUG_ENABLED,
                 "[INTERRUPTS] Removed INT %u handle",
                 interrupt_line);
    EXIT_CRITICAL(int_state);

    return OS_NO_ERR;
}

OS_RETURN_E kernel_interrupt_register_irq_handler(const uint32_t irq_number,
                                       void(*handler)(
                                             cpu_state_t*,
                                             uintptr_t,
                                             stack_state_t*
                                             )
                                       )
{
    int32_t int_line;

    /* Get the interrupt line attached to the IRQ number. */
    int_line = interrupt_driver.driver_get_irq_int_line(irq_number);

    if(int_line < 0)
    {
        return OS_ERR_NO_SUCH_IRQ;
    }

    return kernel_interrupt_register_int_handler(int_line, handler);
}

OS_RETURN_E kernel_interrupt_remove_irq_handler(const uint32_t irq_number)
{
    int32_t int_line;

    /* Get the interrupt line attached to the IRQ number. */
    int_line = interrupt_driver.driver_get_irq_int_line(irq_number);

    if(int_line < 0)
    {
        return OS_ERR_NO_SUCH_IRQ;
    }

    return kernel_interrupt_remove_int_handler(int_line);
}

void kernel_interrupt_restore(const uint32_t prev_state)
{
    if(prev_state != 0)
    {
        KERNEL_DEBUG(INTERRUPTS_DEBUG_ENABLED,
                     "[INTERRUPTS] --- Enabled HW INT ---");
        cpu_set_interrupt();
    }
}

uint32_t kernel_interrupt_disable(void)
{
    uint32_t old_state = cpu_get_interrupt_state();

    if(old_state == 0)
    {
        return 0;
    }

    cpu_clear_interrupt();

    KERNEL_DEBUG(INTERRUPTS_DEBUG_ENABLED,
                 "[INTERRUPTS] --- Disabled HW INT ---");

    return old_state;
}

void kernel_interrupt_set_irq_mask(const uint32_t irq_number,
                                   const uint32_t enabled)
{
    KERNEL_DEBUG(INTERRUPTS_DEBUG_ENABLED,
                 "[INTERRUPTS] IRQ Mask change: %u %u",
                 irq_number,
                 enabled);
    interrupt_driver.driver_set_irq_mask(irq_number, enabled);
}

void kernel_interrupt_set_irq_eoi(const uint32_t irq_number)
{
    KERNEL_DEBUG(INTERRUPTS_DEBUG_ENABLED,
                 "[INTERRUPTS] IRQ EOI: %u",
                 irq_number);
    interrupt_driver.driver_set_irq_eoi(irq_number);
}

/************************************ EOF *************************************/