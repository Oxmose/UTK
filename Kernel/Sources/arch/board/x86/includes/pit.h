/*******************************************************************************
 * @file pit.h
 *
 * @see pit.c
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


#ifndef __X86_PIT_H_
#define __X86_PIT_H_

#include <stdint.h>          /* Generic int types */
#include <kernel_error.h>    /* Kernel error codes */
#include <time_management.h> /* Time manager */

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
/* None */

/************************** Static global variables ***************************/
/* None */

/*******************************************************************************
 * FUNCTIONS
 ******************************************************************************/

/**
 * @brief Initializes the PIT settings and interrupt management.
 *
 * @details Initializes PIT settings, sets the PIT interrupt manager and enables
 * interrupts for the PIT.
 */
void pit_init(void);

/**
 * @brief Enables PIT ticks.
 *
 * @details Enables PIT ticks by clearing the PIT's IRQ mask.
 */
void pit_enable(void);

/**
 * @brief Disables PIT ticks.
 *
 * @details Disables PIT ticks by setting the PIT's IRQ mask.
 */
void pit_disable(void);

/**
 * @brief Sets the PIT's tick frequency.
 *
 * @details Sets the PIT's tick frequency. The value must be between 20Hz and
 * 8000Hz.
 *
 * @warning The value must be between 20Hz and 8000Hz
 *
 * @param[in] freq The new frequency to be set to the PIT.
 */
void pit_set_frequency(const uint32_t freq);

/**
 * @brief Returns the PIT tick frequency in Hz.
 *
 * @details Returns the PIT tick frequency in Hz.
 *
 * @return The PIT tick frequency in Hz.
 */
uint32_t pit_get_frequency(void);

/**
 * @brief Sets the PIT tick handler.
 *
 * @details Sets the PIT tick handler. This function will be called at each PIT
 * tick received.
 *
 * @param[in] handler The handler of the PIT interrupt.
 *
 * @return The success state or the error code.
 * - OS_NO_ERR is returned if no error is encountered.
 * - OS_ERR_NULL_POINTER is returned if the handler is NULL.
  * - OR_ERR_UNAUTHORIZED_INTERRUPT_LINE is returned if the PIT interrupt line
  * is not allowed.
 * - OS_ERR_NULL_POINTER is returned if the pointer to the handler is NULL.
 * - OS_ERR_INTERRUPT_ALREADY_REGISTERED is returned if a handler is already
 * registered for the PIT.
 */
OS_RETURN_E pit_set_handler(void(*handler)(
                                 cpu_state_t*,
                                 uintptr_t,
                                 stack_state_t*
                                 ));

/**
 * @brief Removes the PIT tick handler.
 *
 * @details Removes the PIT tick handler.
 *
 * @return The success state or the error code.
 * - OS_NO_ERR is returned if no error is encountered.
 * - OR_ERR_UNAUTHORIZED_INTERRUPT_LINE is returned if the PIT interrupt line
 * is not allowed.
 * - OS_ERR_INTERRUPT_NOT_REGISTERED is returned if the PIT line has no handler
 * attached.
 */
OS_RETURN_E pit_remove_handler(void);

/**
 * @brief Returns the PIT IRQ number.
 *
 * @details Returns the PIT IRQ number.
 *
 * @return The PIT IRQ number.
 */
uint32_t pit_get_irq(void);

/**
 * @brief Returns the PIT driver.
 *
 * @details Returns a constant handle to the PIT driver.
 *
 * @return A constant handle to the PIT driver is returned.
 */
const kernel_timer_t* pit_get_driver(void);

#endif /* #ifndef __X86_PIT_H_ */

/************************************ EOF *************************************/