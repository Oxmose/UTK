/*******************************************************************************
 * @file stddef.h
 *
 * @author Alexy Torres Aurora Dugo
 *
 * @date 27/02/2021
 *
 * @version 1.0
 *
 * @brief Standard definitions for the kernel.
 *
 * @details Standard definitions for the kernel.
 *
 * @copyright Alexy Torres Aurora Dugo
 ******************************************************************************/

#ifndef __LIB_STDDEF_H_
#define __LIB_STDDEF_H_

#include <config.h> /* Kernel configuration */
#include <stdint.h> /* Generic int types */

/*******************************************************************************
 * CONSTANTS
 ******************************************************************************/

/** @brief UTK's NULL definition. */
#define NULL ((void *)0)

/** @brief Defines the MIN function, return the minimal value between two
 * variables.
 *
 * @param[in] x The first value to compare.
 * @param[in] x The second value to compare.
 *
 * @return The smallest value.
 */
#define MIN(x, y) ((x) < (y) ? (x) : (y))

/** @brief Defines the MAX function, return the maximal value between two
 * variables.
 *
 * @param[in] x The first value to compare.
 * @param[in] x The second value to compare.
 *
 * @return The biggest value.
 */
#define MAX(x, y) ((x) < (y) ? (y) : (x))

/*******************************************************************************
 * STRUCTURES
 ******************************************************************************/

/**
 * @brief Defines OS_EVENT_ID type as a renaming for int32_t.
 */
typedef int32_t OS_EVENT_ID;

#ifndef __SIZE_TYPE__
#error __SIZE_TYPE__ not defined
#endif

/**
 * @brief Defines size_t type as a renaming for __SIZE_TYPE__.
 */
typedef __SIZE_TYPE__ size_t;

#ifndef __PTRDIFF_TYPE__
#error __PTRDIFF_TYPE__ not defined
#endif

/**
 * @brief Defines ptrdiff_t type as a renaming for __PTRDIFF_TYPE__.
 */
typedef __PTRDIFF_TYPE__ ptrdiff_t;

/**
 * @brief Defines uintptr_t type as address type.
 */
#ifdef ARCH_64_BITS
typedef uint64_t uintptr_t;
#elif defined(ARCH_32_BITS)
typedef uint32_t uintptr_t;
#else 
#error Architecture is not supported by the standard library
#endif

#ifdef ARCH_64_BITS
/** @brief Defines the format for a pointer */
#define PRIPTR  "%lu"
#elif defined(ARCH_32_BITS)
/** @brief Defines the format for a pointer */
#define PRIPTR  "%u"
#else 
#error Architecture is not supported by the standard library
#endif

/*******************************************************************************
 * FUNCTIONS
 ******************************************************************************/

/* None */

#endif /* __LIB_STDDEF_H_ */
