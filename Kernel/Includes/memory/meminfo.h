/*******************************************************************************
 * @file meminfo.h
 * 
 * @see meminfo.c
 *
 * @author Alexy Torres Aurora Dugo
 *
 * @date 25/01/2018
 *
 * @version 1.0
 *
 * @brief Kernel memory detector.
 * 
 * @details This module is used to detect the memory mapping of the system.
 * 
 * @copyright Alexy Torres Aurora Dugo
 ******************************************************************************/

#ifndef __MEMORY_MEMINFO_H_
#define __MEMORY_MEMINFO_H_

#include <lib/stddef.h> /* Standard definition */
#include <lib/stdint.h> /* Generic int types */

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

/** 
 * @brief Returns the usage in bytes of the kernel heap.
 * 
 * @return The usage in bytes of the kernel heap.
 */
uint64_t meminfo_kernel_heap_usage(void);

/** 
 * @brief Returns the size in bytes of the kernel heap.
 * 
 * @return The size in bytes of the kernel heap.
 */
uint64_t meminfo_kernel_heap_size(void);

/** 
 * @brief Returns the usage in bytes of the memory.
 * 
 * @return The usage in bytes of the memory.
 */
uint64_t meminfo_kernel_memory_usage(void);

/** 
 * @brief Returns the size in bytes of the kernel size in memory (reserved).
 * 
 * @return The size in bytes of the kernel size in memory (reserved).
 */
uint64_t meminfo_kernel_total_size(void);

/** 
 * @brief Returns the size in bytes of the system's memory.
 * 
 * @return The size in bytes of the system's memory.
 */
uint64_t meminfo_get_memory_size(void);

#endif /* #ifndef __MEMORY_MEMINFO_H_ */