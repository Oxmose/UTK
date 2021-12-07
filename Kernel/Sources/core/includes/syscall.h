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

#include <stdint.h>       /* Generic int types */
#include <stddef.h>       /* Standard definitions */
#include <kernel_error.h> /* Kernel error codes */

/*******************************************************************************
 * CONSTANTS
 ******************************************************************************/

/* None */

/*******************************************************************************
 * STRUCTURES
 ******************************************************************************/

/** @brief Defines the ID for each system call */
enum SYSCALL_FUNCTION
{
    SYSCALL_FORK = 0,
    SYSCALL_WAITPID,          
    SYSCALL_EXIT,             
    SYSCALL_FUTEX_WAIT,      
    SYSCALL_FUTEX_WAKE,       
    SYSCALL_SCHED_GET_PARAMS, 
    SYSCALL_SCHED_SET_PARAMS, 
    SYSCALL_MUTEX_CREATE,     
    SYSCALL_MUTEX_DESTROY,    
    /* 9 */
    SYSCALL_MAX_ID
};

/** @brief Short name for enum SYSCALL_FUNCTION */
typedef enum SYSCALL_FUNCTION SYSCALL_FUNCTION_E;

/** @brief Defines the system call handler structure. */
struct syscall_handler
{
    /** @brief System call handler routine. */
    void(*handler)(SYSCALL_FUNCTION_E, void*);
};

/** 
 * @brief Defines syscall_handler_t type as a shorcut for 
 * struct syscall_handler.
 */
typedef struct syscall_handler syscall_handler_t;

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