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

#ifndef __CORE_SYSCALL_H_
#define __CORE_SYSCALL_H_

/*******************************************************************************
 * INCLUDES
 ******************************************************************************/

#include <stdint.h>       /* Generic int types */
#include <stddef.h>       /* Standard definitions */
#include <kernel_error.h> /* Kernel error codes */

/*******************************************************************************
 * CONSTANTS
 ******************************************************************************/

/* None */

/*******************************************************************************
 * STRUCTURES AND TYPES
 ******************************************************************************/

/** @brief Defines the ID for each system call */
typedef enum
{
    SYSCALL_FORK = 0,
    SYSCALL_WAITPID,
    SYSCALL_EXIT,
    SYSCALL_FUTEX_WAIT,
    SYSCALL_FUTEX_WAKE,
    SYSCALL_SCHED_GET_PARAMS,
    SYSCALL_SCHED_SET_PARAMS,
    SYSCALL_PAGE_ALLOC,
    /* 7 */
    SYSCALL_MAX_ID
} SYSCALL_FUNCTION_E;

/** @brief Defines the system call handler structure. */
typedef struct
{
    /** @brief System call handler routine. */
    void(*handler)(SYSCALL_FUNCTION_E, void*);
} syscall_handler_t;


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
 * @brief Initializes the system call manager.
 *
 * @details Initializes the system call manager. The system call table is
 * validated and the system call interrupts / raise method are initialized.
 */
void syscall_init(void);

#endif /* #ifndef __CORE_SYSCALL_H_ */

/************************************ EOF *************************************/