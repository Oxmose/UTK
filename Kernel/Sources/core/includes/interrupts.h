/*******************************************************************************
 * @file interrupts.h
 *
 * @see interrupts.c
 *
 * @author Alexy Torres Aurora Dugo
 *
 * @date 28/02/2021
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

#ifndef __CORE_INTERRUPTS_H_
#define __CORE_INTERRUPTS_H_

#include <stdint.h>       /* Generic int types */
#include <stddef.h>       /* Standard definitions */
#include <cpu_settings.h> /* CPU structures and settings */
#include <kernel_error.h> /* Kernel error codes */

/*******************************************************************************
 * INCLUDES
 ******************************************************************************/

/* None */

/*******************************************************************************
 * CONSTANTS
 ******************************************************************************/

/* None */

/*******************************************************************************
 * STRUCTURES AND TYPES
 ******************************************************************************/

/**
 * @brief Interrupt types enumeration.
 */
typedef enum
{
    /** @brief Spurious interrupt type. */
    INTERRUPT_TYPE_SPURIOUS,
    /** @brief Regular interrupt type. */
    INTERRUPT_TYPE_REGULAR
} INTERRUPT_TYPE_E;

/** @brief Custom interrupt handler structure. */
typedef struct
{
    /** @brief Handler's state.*/
    bool_t enabled;
    /** @brief Handler's entry point. */
    void(*handler)(cpu_state_t*, uintptr_t, stack_state_t*);
} custom_handler_t;

/** @brief Defines the basic interface for an interrupt management driver (let
 * it be PIC or IO APIC for instance).
 */
typedef struct
{
    /** @brief The function should enable or diable an IRQ given the IRQ number
     * used as parameter.
     *
     * @details The function should enable or diable an IRQ given the IRQ number
     * used as parameter.
     *
     * @param[in] irq_number The number of the IRQ to enable/disable.
     * @param[in] enabled Must be set to TRUE to enable the IRQ and FALSE to
     * disable the IRQ.
     */
    void (*driver_set_irq_mask)(const uint32_t irq_number,
                                const bool_t enabled);

    /**
     * @brief The function should acknowleges an IRQ.
     *
     * @details The function should acknowleges an IRQ.
     *
     * @param[in] irq_number The irq number to acknowledge.
     */
    void (*driver_set_irq_eoi)(const uint32_t irq_number);

    /**
     * @brief The function should check if the serviced interrupt is a spurious
     * interrupt. It also should handle the spurious interrupt.
     *
     * @details The function should check if the serviced interrupt is a
     * spurious interrupt. It also should handle the spurious interrupt.
     *
     * @param[in] int_number The interrupt number of the interrupt to test.
     *
     * @return The function will return the interrupt type.
     * - INTERRUPT_TYPE_SPURIOUS if the current interrupt is a spurious one.
     * - INTERRUPT_TYPE_REGULAR if the current interrupt is a regular one.
     */
    INTERRUPT_TYPE_E (*driver_handle_spurious)(const uint32_t int_number);

    /**
     * @brief Returns the interrupt line attached to an IRQ.
     *
     * @details Returns the interrupt line attached to an IRQ. -1 is returned
     * if the IRQ number is not supported by the driver.
     *
     * @return The interrupt line attached to an IRQ. -1 is returned if the IRQ
     * number is not supported by the driver.
     */
    int32_t (*driver_get_irq_int_line)(const uint32_t irq_number);
} interrupt_driver_t;

/*******************************************************************************
 * MACROS
 ******************************************************************************/

/* None */

/*******************************************************************************
 * GLOBAL VARIABLES
 ******************************************************************************/

/* Imported global variables */
/* None */

/* Exported global variables */
/* None */

/* Static global variables */
/* None */

/*******************************************************************************
 * FUNCTIONS
 ******************************************************************************/

/**
 * @brief Initializes the kernel's interrupt manager.
 *
 * @details Blanks the handlers memory, initializes panic and spurious interrupt
 * lines handlers.
 */
void kernel_interrupt_init(void);

/**
 * @brief Set the driver to be used by the kernel to manage interrupts.
 *
 * @details Changes the current interrupt manager by the new driver given as
 * parameter. The old driver structure is removed from memory.
 *
 * @param[in] driver The driver structure to be used by the interrupt manager.
 *
 * @return The success state or the error code.
 * - OS_NO_ERR is returned if no error is encountered.
 * - OS_ERR_NULL_POINTER is returned if one of the driver's function pointers
 * is NULL or the driver's pointer is NULL.
 */
OS_RETURN_E kernel_interrupt_set_driver(const interrupt_driver_t* driver);

/**
 * @brief Registers a new interrupt handler for the desired IRQ number.
 *
 * @details Registers a custom interrupt handler to be executed. The IRQ
 * number must be greater or equal to the minimal authorized custom IRQ number
 * and less than the maximal one.
 *
 * @param[in] irq_number The IRQ number to attach the handler to.
 * @param[in] handler The handler for the desired interrupt.
 *
 * @return The success state or the error code.
 * - OS_NO_ERR is returned if no error is encountered.
 * - OR_ERR_UNAUTHORIZED_INTERRUPT_LINE is returned if the IRQ attached to the
 * interrupt line is not allowed.
 * - OS_ERR_NO_SUCH_IRQ_LINE is returned if the IRQ number is not supported.
 * - OS_ERR_NULL_POINTER is returned if the pointer
 * to the handler is NULL.
 * - OS_ERR_INTERRUPT_ALREADY_REGISTERED is returned if a
 * handler is already registered for this RIQ number.
 */
OS_RETURN_E kernel_interrupt_register_irq_handler(const uint32_t irq_number,
                                       void(*handler)(
                                             cpu_state_t*,
                                             uintptr_t,
                                             stack_state_t*
                                             )
                                       );

/**
 * @brief Unregisters an interrupt handler for the desired IRQ number.
 *
 * @details Unregisters a custom interrupt handler to be executed. The IRQ
 * number must be greater or equal to the minimal authorized custom IRQ number
 * and less than the maximal one.
 *
 * @param[in] irq_number The IRQ number to detach the handler from.
 *
 * @return The success state or the error code.
 * - OS_NO_ERR is returned if no error is encountered.
  * - OR_ERR_UNAUTHORIZED_INTERRUPT_LINE is returned if the IRQ attached to the
 * interrupt line is not allowed.
 * - OS_ERR_NO_SUCH_IRQ_LINE is returned if the IRQ number is not supported.
 * - OS_ERR_INTERRUPT_NOT_REGISTERED is returned if the IRQ has no handler
 * attached.
 */
OS_RETURN_E kernel_interrupt_remove_irq_handler(const uint32_t irq_number);

/**
 * @brief Registers an interrupt handler for the desired interrupt line.
 *
 * @details Registers a custom interrupt handler to be executed. The interrupt
 * line must be greater or equal to the minimal authorized custom interrupt line
 * and less than the maximal one.
 *
 * @param[in] interrupt_line The interrupt line to attach the handler to.
 * @param[in] handler The handler for the desired interrupt.
 *
 * @return The success state or the error code.
 * - OS_NO_ERR is returned if no error is encountered.
 * - OR_ERR_UNAUTHORIZED_INTERRUPT_LINE is returned if the desired
 * interrupt line is not allowed.
 * - OS_ERR_NULL_POINTER is returned if the pointer
 * to the handler is NULL.
 * - OS_ERR_INTERRUPT_ALREADY_REGISTERED is returned if a
 * handler is already registered for this interrupt line.
 */
OS_RETURN_E kernel_interrupt_register_int_handler(const uint32_t interrupt_line,
                                       void(*handler)(
                                             cpu_state_t*,
                                             uintptr_t,
                                             stack_state_t*
                                             )
                                       );

/**
 * @brief Unregisters a new interrupt handler for the desired interrupt line.
 *
 * @details Unregisters a custom interrupt handler to be executed. The interrupt
 * line must be greater or equal to the minimal authorized custom interrupt line
 * and less than the maximal one.
 *
 * @param[in] interrupt_line The interrupt line to deattach the handler from.
 *
 * @return The success state or the error code.
 * - OS_NO_ERR is returned if no error is encountered.
 * - OR_ERR_UNAUTHORIZED_INTERRUPT_LINE is returned if the desired
 * interrupt line is not allowed.
 * - OS_ERR_INTERRUPT_NOT_REGISTERED is returned if the interrupt line has no
 * handler attached.
 */
OS_RETURN_E kernel_interrupt_remove_int_handler(const uint32_t interrupt_line);

/**
 * @brief Restores the CPU interrupts state.
 *
 * @details Restores the CPU interrupts state by setting the EFLAGS accordingly.
 *
 * @param[in] prev_state The previous interrupts state that has to be retored.
 */
void kernel_interrupt_restore(const uint32_t prev_state);

/**
 * @brief Disables the CPU interrupts.
 *
 * @details Disables the CPU interrupts by setting the EFLAGS accordingly.
 *
 * @return The current interrupt state is returned to be restored latter in the
 * execution of the kernel.
 */
uint32_t kernel_interrupt_disable(void);

/**
 * @brief Sets the IRQ mask for the IRQ number given as parameter.
 *
 * @details Sets the IRQ mask for the IRQ number given as parameter.
 *
 * @param[in] irq_number The irq number to enable/disable.
 * @param[in] enabled Must be set to 1 to enable the IRQ or 0 to disable the
 * IRQ.
 */
void kernel_interrupt_set_irq_mask(const uint32_t irq_number,
                                   const uint32_t enabled);

/**
 * @brief Acknowleges an IRQ.
 *
 * @details Acknowleges an IRQ.
 *
 * @param[in] irq_number The irq number to acknowledge.
 */
void kernel_interrupt_set_irq_eoi(const uint32_t irq_number);

#endif /* #ifndef __CORE_INTERRUPTS_H_ */

/* EOF */
