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

/* UTK configuration file */
#include <config.h>

/*******************************************************************************
 * CONSTANTS
 ******************************************************************************/

/*******************************************************************************
 * STRUCTURES
 ******************************************************************************/

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

#endif /* #ifndef __LIB_PROCESS_H_ */