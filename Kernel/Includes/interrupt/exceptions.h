/*******************************************************************************
 * @file exceptions.h
 * 
 * @see exceptions.c
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

#ifndef __INTERRUPTS_EXCEPTIONS_H_
#define __INTERRUPTS_EXCEPTIONS_H_

#include <lib/stdint.h>         /* Generic int types */
#include <lib/stddef.h>         /* Standard definitions */
#include <cpu_structs.h>        /* CPU structures */

/*******************************************************************************
 * CONSTANTS
 ******************************************************************************/

/*******************************************************************************
 * STRUCTURES
 ******************************************************************************/

/*******************************************************************************
 * FUNCTIONS
 ******************************************************************************/

/**
 * @brief Initializes the exception manager.
 * 
 * @details Blanks the handlers memory and initialize the first 32 exceptions to
 * catch intel exceptions.
 * 
 * @return The success state or the error code. 
 * - OS_NO_ERR is returned if no error is encountered. 
 */
OS_RETURN_E kernel_exception_init(void);

/**
 * @brief Registers a new exception handler for the desired exception line.
 * 
 * @details Registers a custom exception handler to be executed. The exception 
 * line must be greater or equal to the minimal authorized custom exception line 
 * and less than the maximal one.
 *
 * @param[in] exception_line The exception line to attach the handler to.
 * @param[in] handler The handler for the desired exception.
 * 
 * @return The success state or the error code. 
 * - OS_NO_ERR is returned if no error is encountered. 
 * - OR_ERR_UNAUTHORIZED_INTERRUPT_LINE is returned if the desired
 * exception line is not allowed. 
 * - OS_ERR_NULL_POINTER is returned if the pointer
 * to the handler is NULL. 
 * - OS_ERR_INTERRUPT_ALREADY_REGISTERED is returned if a 
 * handler is already registered for this exception line.
 */
OS_RETURN_E kernel_exception_register_handler(const uint32_t exception_line,
                                       void(*handler)(
                                             cpu_state_t*,
                                             uintptr_t,
                                             stack_state_t*
                                             )
                                       );

/**
 * @brief Unregisters a new exception handler for the desired exception line.
 * 
 * @details Unregisters a custom exception handler to be executed. The exception 
 * line must be greater or equal to the minimal authorized custom exception line 
 * and less than the maximal one.
 *
 * @param[in] exception_line The exception line to deattach the handler from.
 * 
 * @return The success state or the error code. 
 * - OS_NO_ERR is returned if no error is encountered. 
 * - OR_ERR_UNAUTHORIZED_INTERRUPT_LINE is returned if the desired
 * exception line is not allowed.
 * - OS_ERR_INTERRUPT_NOT_REGISTERED is returned if the exception line has no
 * handler attached.
 */
OS_RETURN_E kernel_exception_remove_handler(const uint32_t exception_line);

#endif /* #ifndef __INTERRUPTS_EXCEPTIONS_H_ */