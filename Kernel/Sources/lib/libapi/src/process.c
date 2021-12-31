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

/*******************************************************************************
 * INCLUDES
 ******************************************************************************/

/* Included headers */
#include <stddef.h>          /* Standard definitions */
#include <stdint.h>          /* Generic int types */
#include <scheduler.h>       /* Scheduler */
#include <sys/syscall_api.h> /* Syscall API */

/* Configuration files */
#include <config.h>
#include <test_bank.h>

/* Header file */
#include <sys/process.h>

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
 * STATIC FUNCTIONS DECLARATIONS
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

void exit(int32_t ret_value)
{
    syscall_do(SYSCALL_EXIT, (void*)ret_value);
}
/************************************ EOF *************************************/