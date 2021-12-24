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

/*******************************************************************************
 * INCLUDES
 ******************************************************************************/

#include <stdint.h> /* Standard Integer types */

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

/**
 * @brief IDLE thread routine.
 *
 * @details IDLE thread routine. This thread should always be ready, it is the
 * only thread running when no other trhread are ready. It allows better power
 * consumption management and CPU usage computation.
 *
 * @param[in] args The argument to send to the IDLE thread, usualy null.
 *
 * @warning The IDLE thread routine should never return.
 *
 * @return NULL always, should never return.
 */
void* idle_sys(void* args);

/**
 * @brief Returns the number of time the idle thread was schedulled.
 *
 * @details Returns the number of time the idle thread was schedulled.
 *
 * @return The number of time the idle thread was schedulled.
 */
uint64_t sched_get_idle_schedule_count(void);

#endif /* #ifndef __CORE_INIT_H_ */

/************************************ EOF *************************************/