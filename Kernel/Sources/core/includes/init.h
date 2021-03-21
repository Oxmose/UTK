/*******************************************************************************
 * @file init.h
 *
 * @see init.c
 *
 * @author Alexy Torres Aurora Dugo
 *
 * @date 19/03/2021
 *
 * @version 1.0
 *
 * @brief Kernel's init thread
 *
 * @details Kernel's init thread. Starts the first processes.
 *
 * @copyright Alexy Torres Aurora Dugo
 ******************************************************************************/

#ifndef __CORE_INIT_H_
#define __CORE_INIT_H_

#include <stdint.h>     /* Generic int types */
#include <interrupts.h> /* Interrupts management */

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
 * @brief INIT thread routine.
 *
 * @details INIT thread routine.
 *
 * @param[in] args The argument to send to the INIT thread, usualy null.
 *
 * @warning The INIT thread routine should never return.
 *
 * @return NULL always, should never return.
 */
void* init_sys(void* args);

#endif /* #ifndef __CORE_INIT_H_ */