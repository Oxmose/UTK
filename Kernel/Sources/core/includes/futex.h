/*******************************************************************************
 * @file futex.h
 *
 * @see futex.c
 *
 * @author Alexy Torres Aurora Dugo
 *
 * @date 06/12/2021
 *
 * @version 1.0
 *
 * @brief Kernel's futex API.
 *
 * @details Kernel's futex API. This module implements the futex system calls
 * and futex management.
 *
 * @copyright Alexy Torres Aurora Dugo
 ******************************************************************************/

#ifndef __CORE_FUTEX_H_
#define __CORE_FUTEX_H_

/*******************************************************************************
 * INCLUDES
 ******************************************************************************/

#include <stdint.h>       /* Standard int definitions */
#include <kernel_error.h> /* Kernel error API */
#include <syscall.h>      /* System call manager */

/*******************************************************************************
 * CONSTANTS
 ******************************************************************************/

/* None */

/*******************************************************************************
 * STRUCTURES AND TYPES
 ******************************************************************************/

/** @brief Futex structure definition. */
typedef struct
{
    /** @brief Futex atomic memory region */
    uint32_t* addr;

    /** @brief Futex waiting value or number of threads to wake */
    uint32_t val;

    /** @brief The futex's error state */
    OS_RETURN_E error;
} futex_t;

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
 * @brief Initializes the futex facility.
 *
 * @details Initializes the futex facility. If the function was not able to
 * alocate the necessary resources, a kernel panic is generated.
 */
void futex_init(void);

/**
 * @brief System call handler to wait on a given futex.
 *
 * @details System call handler to wait on a given futex. This system call
 * receive the futex to wait and the value to observe as parameters.
 *
 * @param[in] func The syscall function ID, must correspond to the futex_wait
 * call.
 * @param[in, out] params The parameters used by the function, must be of type
 * futex_t.
 */
void futex_wait(const SYSCALL_FUNCTION_E func, void* params);

/**
 * @brief System call handler to wake on a given futex.
 *
 * @details System call handler to wake on a given futex. This system call
 * receive the futex to wake the threads on the value to observe. Wake can
 * wake multiple threads depending on the valu provided in parameters.
 *
 * @param[in] func The syscall function ID, must correspond to the futex_wake
 * call.
 * @param[in, out] params The parameters used by the function, must be of type
 * futex_t.
 */
void futex_wake(const SYSCALL_FUNCTION_E func, void* params);

#endif /* #ifndef __CORE_FUTEX_H_ */

/************************************ EOF *************************************/