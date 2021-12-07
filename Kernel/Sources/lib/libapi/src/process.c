/*******************************************************************************
 * @file process.c
 *
 * @see process.h
 *
 * @author Alexy Torres Aurora Dugo
 *
 * @date 22/03/2021
 *
 * @version 1.0
 *
 * @brief Process related functions.
 *
 * @details Process related functions. This module defines the user API to 
 * create, manage and delete processes and threads.
 *
 * @copyright Alexy Torres Aurora Dugo
 ******************************************************************************/

#include <stddef.h>          /* Standard definitions */
#include <stdint.h>          /* Generic int types */
#include <scheduler.h>       /* Scheduler */
#include <sys/syscall_api.h> /* Syscall API */

/* UTK configuration file */
#include <config.h>

/* Header file */
#include <sys/process.h>

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

int32_t fork(void)
{
    int32_t pid;

    pid = 0;
    syscall_do(SYSCALL_FORK, &pid);

    /* Here pid = 0 for the new process */
    return pid;
}

int32_t waitpid(const int32_t pid, 
                int32_t* status, 
                int32_t* term_cause, 
                OS_RETURN_E* err)
{
    waitpid_params_t params;

    params.pid = pid;
    
    syscall_do(SYSCALL_WAITPID, &params);

    if(status != NULL)
    {
        *status = params.status;
    }
    if(term_cause != NULL)
    {
        *term_cause = params.term_cause;
    }
    if(err != NULL)
    {
        *err = params.error;
    }

    return params.pid;
}