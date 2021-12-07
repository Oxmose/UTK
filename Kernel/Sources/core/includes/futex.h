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

#include <stdint.h>       /* Standard int definitions */
#include <kernel_error.h> /* Kernel error API */

/* UTK configuration file */
#include <config.h>

/*******************************************************************************
 * CONSTANTS
 ******************************************************************************/

/** @brief Futex structure definition. */
struct futex
{
    /** @brief Futex atomic memory region */
    uint32_t* addr;

    /** @brief Futex waiting value */
    uint32_t wait;

    /** @brief The futex's error state */
    OS_RETURN_E error;
};

/**
 * @brief Defines futex_t type as a shorcut for struct futex.
 */
typedef struct futex futex_t;

/*******************************************************************************
 * STRUCTURES
 ******************************************************************************/

/* None */

/*******************************************************************************
 * FUNCTIONS
 ******************************************************************************/

/* None */

#endif /* #ifndef __CORE_FUTEX_H_ */