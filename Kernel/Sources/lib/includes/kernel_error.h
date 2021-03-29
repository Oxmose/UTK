/*******************************************************************************
 * @file kernel_error.h
 *
 * @author Alexy Torres Aurora Dugo
 *
 * @date 26/03/2021
 *
 * @version 1.0
 *
 * @brief Kernel error definitions.
 *
 * @details Kernel error definitions. Contains the UTK error codes definition.
 *
 * @copyright Alexy Torres Aurora Dugo
 ******************************************************************************/

#ifndef __LIB_KERNEL_ERROR_H_
#define __LIB_KERNEL_ERROR_H_

#include <config.h> /* Kernel configuration */


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

/** @brief System return states enumeration. */
enum OS_RETURN
{
    /** @brief No error occured. */
    OS_NO_ERR                              = 0,
    /** @brief A null pointer was detected. */
    OS_ERR_NULL_POINTER                    = 1,
    /** @brief Malloc could not allocated memory. */
    OS_ERR_MALLOC                          = 2,
    /** @brief Error while comparing signatures. */
    OS_ERR_WRONG_SIGNATURE                 = 3,
    /** @brief Incorrect checksum detected. */
    OS_ERR_CHECKSUM_FAILED                 = 4,
    /** @brief Unsuported action. */
    OS_ERR_NOT_SUPPORTED                   = 5,
    /** @brief The requested IRQ does not exist. */
    OS_ERR_NO_SUCH_IRQ                     = 6,
    /** @brief Need initialization. */
    OS_ERR_NOT_INITIALIZED                 = 7,
    /** @brief Incorrect value was requested. */
    OS_ERR_INCORRECT_VALUE                 = 8,
    /** @brief ID was not found. */
    OS_ERR_NO_SUCH_ID                      = 9,
    /** @brief Requested an unauthorized action. */
    OS_ERR_UNAUTHORIZED_ACTION             = 10,
    /** @brief No more free memory to allocated. */
    OS_ERR_NO_MORE_FREE_MEM                = 11,
    /** @brief Request is out of bound. */
    OS_ERR_OUT_OF_BOUND                    = 12,
    /** @brief Tried to use non mapped memory. */
    OS_ERR_MEMORY_NOT_MAPPED               = 13,
    /** @brief Tries to map an already mapped memory region. */
    OS_ERR_MAPPING_ALREADY_EXISTS          = 14,
    /** @brief Unknown system call. */
    OS_ERR_SYSCALL_UNKNOWN                 = 15,
    /** @brief Tried to register an already registered interrupt. */
    OS_ERR_INTERRUPT_ALREADY_REGISTERED    = 16,
    /** @brief Tried to use an unregistered interrupt. */
    OS_ERR_INTERRUPT_NOT_REGISTERED        = 17,
    /** @brief A forbiden priority has been used. */
    OS_ERR_FORBIDEN_PRIORITY               = 18,
    /** @brief Tried to use an unauthorized interrupt line. */
    OR_ERR_UNAUTHORIZED_INTERRUPT_LINE     = 19,
    /** @brief Allignement error. */
    OS_ERR_ALIGN                           = 20
};

/**
 * @brief Defines OS_RETURN_E type as a shorcut for enum OS_RETURN.
 */
typedef enum OS_RETURN OS_RETURN_E;

#endif /* __LIB_KERNEL_ERROR_H_ */
