/*******************************************************************************
 * @file lapic.h
 *
 * @see lapic.c
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
 * the LAPIC timer. the PIC must be present and initialized to use this driver.
 *
 * @copyright Alexy Torres Aurora Dugo
 ******************************************************************************/


#ifndef __X86_LAPIC_H_
#define __X86_LAPIC_H_

#include <stdint.h>          /* Generic int types */
#include <kernel_error.h>    /* Kernel error codes */
#include <time_management.h> /* Time manager */

/*******************************************************************************
 * CONSTANTS
 ******************************************************************************/

/* None */

/*******************************************************************************
 * STRUCTURES
 ******************************************************************************/

/* None */

/*******************************************************************************
 * FUNCTIONS
 ******************************************************************************/

/**
 * @brief Initializes the CPU Local APIC.
 *
 * @details Initializes the CPU Local APIC. The function initializes the LAPIC
 * interrupts (enables them), set the LAPIC destination mode and sets the
 * spurious vector.
 *
 * @return The success state or the error code.
 * - OS_NO_ERR is returned if no error is encountered.
 * - OS_ERR_NOT_SUPPORTED is returned if the LAPIC is not supported by the
 * current system.
 */
void lapic_init(void);

/**
 * @brief Initializes the CPU Local APIC Timer.
 *
 * @details Initializes the CPU Local APIC Timer. The function initializes the
 * LAPIC Timer and its interrupt. The Timer is then set to its initial frequency
 * by the driver.
 *
 * @warning This function is not to be used by Application Processors, please
 * use lapic_ap_timer_init for this purpose.
 *
 * @return The success state or the error code.
 * - OS_NO_ERR is returned if no error is encountered.
 * - OS_ERR_NOT_SUPPORTED is returned if the LAPIC is not supported by the
 * current system.
 */
void lapic_timer_init(void);

/**
 * @brief Returns the current CPU Local APIC ID.
 *
 * @details Returns the current CPU Local APIC ID. The current CPU is the CPU on
 * which the function is called.
 *
 * @returns The current CPU Local APIC ID. -1 is returned on error.
 */
int32_t lapic_get_id(void);

/**
 * @brief Sets END OF INTERRUPT for the current CPU Local APIC.
 *
 * @details Sets END OF INTERRUPT for the current CPU Local APIC.
 *
 * @param[in] interrupt_line The intrrupt line for which the EOI should be set.
 */
void lapic_set_int_eoi(const uint32_t interrupt_line);

/**
 * @brief Enables LAPIC Timer ticks.
 *
 * @details Enables LAPIC Timer ticks by clearing the LAPIC Timer's IRQ mask.
 */
void lapic_timer_enable(void);

/**
 * @brief Disables LAPIC Timer ticks.
 *
 * @details Disables LAPIC Timer ticks by setting the LAPIC Timer's IRQ mask.
 */
void lapic_timer_disable(void);

/**
 * @brief Sets the LAPIC Timer's tick frequency.
 *
 * @details Sets the LAPIC Timer's tick frequency. The value must be between
 * 20Hz and 8000Hz.
 *
 * @warning The value must be between 20Hz and 8000Hz
 *
 * @param[in] freq The new frequency to be set to the LAPIC Timer.
 */
void lapic_timer_set_frequency(const uint32_t freq);

/**
 * @brief Returns the LAPIC Timer tick frequency in Hz.
 *
 * @details Returns the LAPIC Timer tick frequency in Hz.
 *
 * @return The LAPIC Timer tick frequency in Hz.
 */
uint32_t lapic_timer_get_frequency(void);

/**
 * @brief Sets the LAPIC Timer tick handler.
 *
 * @details Sets the LAPIC Timer tick handler. This function will be called at
 * each LAPIC Timer tick received.
 *
 * @param[in] handler The handler of the LAPIC Timer interrupt.
 *
 * @return The success state or the error code.
 * - OS_NO_ERR is returned if no error is encountered.
 * - OS_ERR_NULL_POINTER is returned if the handler is NULL.
  * - OR_ERR_UNAUTHORIZED_INTERRUPT_LINE is returned if the LAPIC Timer
  * interrupt line is not allowed.
 * - OS_ERR_NULL_POINTER is returned if the pointer to the handler is NULL.
 * - OS_ERR_INTERRUPT_ALREADY_REGISTERED is returned if a handler is already
 * registered for the LAPIC Timer.
 */
OS_RETURN_E lapic_timer_set_handler(void(*handler)(
                                    cpu_state_t*,
                                    uintptr_t,
                                    stack_state_t*
                                    ));

/**
 * @brief Removes the LAPIC Timer tick handler.
 *
 * @details Removes the LAPIC Timer tick handler.
 *
 * @return The success state or the error code.
 * - OS_NO_ERR is returned if no error is encountered.
 * - OR_ERR_UNAUTHORIZED_INTERRUPT_LINE is returned if the LAPIC Timer interrupt
 * line is not allowed.
 * - OS_ERR_INTERRUPT_NOT_REGISTERED is returned if the LAPIC Timer line has no
 * handler attached.
 */
OS_RETURN_E lapic_timer_remove_handler(void);

/**
 * @brief Returns the LAPIC Timer IRQ number.
 *
 * @details Returns the LAPIC Timer IRQ number.
 *
 * @return The LAPIC Timer IRQ number.
 */
uint32_t lapic_timer_get_irq(void);

/**
 * @brief Returns the LAPIC timer driver.
 * 
 * @details Returns a constant handle to the LAPIC timer driver.
 * 
 * @return A constant handle to the LAPIC timer driver is returned.
 */
const kernel_timer_t* lapic_timer_get_driver(void);

#endif /* #ifndef __X86_LAPIC_H_ */