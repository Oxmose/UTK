/*******************************************************************************
 * @file critical.h
 *
 * @see critical.c
 *
 * @author Alexy Torres Aurora Dugo
 *
 * @date 28/02/2021
 *
 * @version 1.0
 *
 * @brief Kernel's concurency management module.
 *
 * @details Kernel's concurency management module. Defines the different basic
 * synchronization primitives used in the kernel.
 *
 * @copyright Alexy Torres Aurora Dugo
 ******************************************************************************/

#ifndef __CORE_CRITICAL_H_
#define __CORE_CRITICAL_H_

/*******************************************************************************
 * INCLUDES
 ******************************************************************************/

#include <interrupts.h> /* Interrupts management */

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

/**
 * @brief Enters a critical section in the kernel.
 *
 * @param[out] x The critical state at section's entrance.
 *
 * @details Enters a critical section in the kernel. Save interrupt state and
 * disables interrupts.
 */
#define ENTER_CRITICAL(x) {         \
    x = kernel_interrupt_disable(); \
}

/**
 * @brief Exits a critical section in the kernel.
 *
 * @param[in] x The critical state at section's entrance.
 *
 * @details Exits a critical section in the kernel. Restore the previous
 * interrupt state.
 */
#define EXIT_CRITICAL(x) {           \
    kernel_interrupt_restore(x);     \
}

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

/* None */

#endif /* #ifndef __CORE_CRITICAL_H_ */

/************************************ EOF *************************************/