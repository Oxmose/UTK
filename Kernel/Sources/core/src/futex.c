/*******************************************************************************
 * @file futex.c
 *
 * @see futex.h
 *
 * @author Alexy Torres Aurora Dugo
 *
 * @date 06/12/2021
 *
 * @version 1.0
 *
 * @brief Kernel's futex API.
 *
 * @details Kernel's futex API. This module implements the futex system calls
 * and futex management.
 *
 * @copyright Alexy Torres Aurora Dugo
 ******************************************************************************/

#include <stdint.h>             /* Generic int types */

/* UTK configuration file */
#include <config.h>

/* Tests header file */
#ifdef TEST_MODE_ENABLED
#include <test_bank.h>
#endif

/* Header file */
#include <futex.h>

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

void futex_wait(const SYSCALL_FUNCTION_E func, void* params)
{
    futex_t* func_params;

    func_params = (futex_t*)params;

    if(func != SYSCALL_FUTEX_WAIT)
    {
        if(func_params != NULL)
        {
            func_params->addr  = NULL;
            func_params->wait  = 0;
            func_params->error = OS_ERR_UNAUTHORIZED_ACTION;
        }
        return;
    }
    if(func_params == NULL)
    {
        return;
    }

    
}