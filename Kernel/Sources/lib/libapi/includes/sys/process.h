/*******************************************************************************
 * @file process.h
 *
 * @see process.c
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

#ifndef __LIB_PROCESS_H_
#define __LIB_PROCESS_H_

#include <stddef.h>    /* Standard definitons */
#include <stdint.h>    /* Generic int types */
#include <scheduler.h> /* Scheduler API */

/* UTK configuration file */
#include <config.h>

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
 * @brief Forks the current process.
 * 
 * @details Forks the current process. A complete copy of the current process
 * will be done and memory will be marked as COW for both new and current 
 * process. Only the calling thread will be copied to the new process.
 * 
 * @return -1 is retuned on error. 0 is returned for the new process and the PID
 * of the new process is returned for the current calling process.
 */
int32_t fork(void);

/** 
 * @brief Waits for child process to finish.
 * 
 * @details Waits for child process to finish. If the process does not exist or
 * is not a child of the current process, the function reutrns immediatly.
 * 
 * @param[in] pid The pid of the process to wait.
 * @param[out] status The buffer to receive the return value of the process.
 * @param[out] term_cause The buffer to receive the process termination cause.
 * @param[out] err The error or success status buffer.
 * 
 * @return The function return the pid of the waited process on succes. On error
 * -1 is returned.
 */
int32_t waitpid(const int32_t pid, 
                int32_t* status, 
                int32_t* term_cause, 
                OS_RETURN_E* err);

#endif /* #ifndef __LIB_PROCESS_H_ */