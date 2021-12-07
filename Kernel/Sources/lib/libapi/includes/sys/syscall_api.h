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
#include <scheduler.h>    /* Scheduler API */

/*******************************************************************************
 * CONSTANTS
 ******************************************************************************/

/* None */

/*******************************************************************************
 * STRUCTURES
 ******************************************************************************/

/** @brief Memory allocation system call parameters.*/
struct memmgt_page_alloc_param
{
    /** @brief The number of pages to allocate. */
    size_t page_count;

    /** @brief Receives the start address of the allocated memory, filled by the 
     * system call.
     */
    void* start_addr;

    /** @brief Receives the system call error status. */
    OS_RETURN_E error;
};

/** @brief Short name for struct memmgt_page_alloc_param */
typedef struct memmgt_page_alloc_param memmgt_page_alloc_param_t;

/** @brief waitpid function system call parameters.*/
struct waitpid_params
{
    /** @brief The return values of the main process' thread. */
    int32_t status;

    /** @brief The pid of the waited process. */
    int32_t pid;

    /** @brief The process termination cause. */
    THREAD_TERMINATE_CAUSE_E term_cause;

    /** @brief Receives the system call error status. */
    OS_RETURN_E error;
};

/** @brief Short name for struct waitpid_params */
typedef struct waitpid_params waitpid_params_t;

/** @brief Scheduling parameters structure.*/
struct sched_param
{
    /** @brief The pid of the current process. */
    int32_t pid;

    /** @brief The tid of the calling thread. */
    int32_t tid;

    /** @brief The priority of the calling thread. */
    uint32_t priority;

    /** @brief Receives the system call error status. */
    OS_RETURN_E error;
};

/** @brief Short name for struct sched_param */
typedef struct sched_param sched_param_t;

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