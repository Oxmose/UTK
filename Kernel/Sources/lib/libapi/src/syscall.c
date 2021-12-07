/*******************************************************************************
 * @file syscall.c
 * 
 * @see syscall.h
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

#include <stdint.h>             /* Generic int types */
#include <interrupt_settings.h> /* Interrupt settings */
#include <cpu_api.h>            /* CPU API */
#include <kernel_output.h>      /* Kernel output */
#include <scheduler.h>          /* Scheduler */
#include <kernel_error.h>       /* Kernel error codes */
#include <panic.h>              /* Kernel panic */

/* UTK configuration file */
#include <config.h>

/* Header file */
#include <syscall.h>

/*******************************************************************************
 * CONSTANTS
 ******************************************************************************/

/* None */

/*******************************************************************************
 * STRUCTURES
 ******************************************************************************/

/* None */

/*******************************************************************************
 * GLOBAL VARIABLES
 ******************************************************************************/

/* None */

/*******************************************************************************
 * STATIC FUNCTIONS DECLARATION
 ******************************************************************************/

/* None */

/*******************************************************************************
 * FUNCTIONS
 ******************************************************************************/

OS_RETURN_E syscall_do(const SYSCALL_FUNCTION_E func, void* params)
{
    /* Checks if the system call exists */
    if(func >= SYSCALL_MAX_ID)
    {
        return OS_ERR_SYSCALL_UNKNOWN;
    }

    /* Generate the syscall */
    cpu_syscall(func, params);

    return OS_NO_ERR;
}