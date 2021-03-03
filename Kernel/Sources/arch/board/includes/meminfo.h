/*******************************************************************************
 * @file meminfo.h
 * 
 * @see meminfo.c
 *
 * @author Alexy Torres Aurora Dugo
 *
 * @date 02/03/2021
 *
 * @version 1.0
 *
 * @brief Kernel memory detector.
 * 
 * @details This module is used to detect the memory mapping of the system.
 * 
 * @copyright Alexy Torres Aurora Dugo
 ******************************************************************************/

#ifndef __BSP_MEMINFO_H_
#define __BSP_MEMINFO_H_

#include <stddef.h> /* Standard definition */
#include <stdint.h> /* Generic int types */

/*******************************************************************************
 * CONSTANTS
 ******************************************************************************/

/*******************************************************************************
* STRUCTURES
******************************************************************************/

/** @brief Defines a memory range with its type as defined by the multiboot 
 * standard.
 */
struct mem_range
{
    /** @brief Range's base address. */
    uintptr_t base;

    /** @brief Range's limit. */
    uintptr_t limit;

    /** @brief Range's memory type. */
    uint32_t type;
};

/** 
 * @brief Defines mem_range_t type as a shorcut for struct mem_range.
 */
typedef struct mem_range mem_range_t;

/*******************************************************************************
* FUNCTIONS
******************************************************************************/

/** 
 * @brief Inititalizes the kernel's memory map.
 * 
 * @brief Initializes the kernel's memory map while detecting the system's
 * memory organization.
 * 
 * @return The success state or the error code. 
 * - OS_NO_ERR is returned if no error is encountered. 
 * - No other return value is possible.
 */
OS_RETURN_E memory_map_init(void);


#endif /* #ifndef __BSP_MEMINFO_H_ */