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

#include <stddef.h>        /* Standard definitions */
#include <stdint.h>        /* Generic int types */
#include <scheduler.h>     /* Scheduler */
#include <syscall.h>       /* Syscall maanger */
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
    int32_t current_pid;
    int32_t new_pid;
    int32_t params;

    /* Get the current PID */
    current_pid = sched_get_pid();

    syscall_do(SYSCALL_FORK, &params);

    /* If the current pid is different, we are in the forked process */
    new_pid = sched_get_pid();
    if(new_pid != current_pid)
    {
        return 0;
    }
    else
    {
        return params;
    }
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