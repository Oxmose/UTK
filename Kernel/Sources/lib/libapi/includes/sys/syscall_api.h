/*******************************************************************************
 * @file syscall.h
 * 
 * @see syscall.c
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

#ifndef __LIB_SYSCALL_H_
#define __LIB_SYSCALL_H_

#include <stdint.h>       /* Generic int types */
#include <stddef.h>       /* Standard definitions */
#include <kernel_error.h> /* Kernel error codes */
#include <syscall.h>      /* Kernel syscall manager */

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
 * @brief Raises a system call.
 * 
 * @details Raises a system call. Thus function uses the CPU API to raise the 
 * system call with the desired method. The system calls parameters are passed
 * by the CPU API.
 * 
 * @param[in] func The system ID to raise.
 * @param[in, out] params The system call parameters.
 *
 * @return OS_NO_ERR is returned on success. Otherwise an error is returned.
 */
OS_RETURN_E syscall_do(const SYSCALL_FUNCTION_E func, void* params);

#endif /* #ifndef __LIB_SYSCALL_H_ */