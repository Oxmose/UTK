/*******************************************************************************
 * @file memmgt.h
 * 
 * @see memmgt.c
 *
 * @author Alexy Torres Aurora Dugo
 *
 * @date 02/03/2021
 *
 * @version 1.0
 *
 * @brief Kernel physical memory manager.
 *
 * @details This module is used to detect the memory mapping of the system and 
 * manage physical memory.
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
 * @brief Inititalizes the kernel's memory manager.
 * 
 * @brief Initializes the kernel's memory manager while detecting the system's
 * memory organization.
 * 
 * @return The success state or the error code. 
 * - OS_NO_ERR is returned if no error is encountered. 
 * - No other return value is possible.
 */
OS_RETURN_E memory_manager_init(void);

/**
 * @brief Kernel memory frame allocation.
 * 
 * @details Kernel memory frame allocation. This method gets the desired number
 * of contiguous frames from the kernel frame pool and allocate them.
 * 
 * @param[in] frame_count The number of desired frames to allocate.
 * @param[out] err The error return pointer. OS_NO_ERR in case of success.
 * 
 * @return The address of the first frame of the contiguous block is 
 * returned.
 */
void* alloc_kframes(const size_t frame_count, OS_RETURN_E* err);

/**
 * @brief Kernel memory frame release.
 * 
 * @details Kernel memory frame release. This method releases the desired number
 * of contiguous frames to the kernel frame pool.
 * 
 * @param[in] frame_addr The address of the first frame to release.
 * @param[in] frame_count The number of desired frames to release.
 * 
 * @return OS_NO_ERR is returned on success. Otherwise an error code is 
 * returned.
 */
OS_RETURN_E free_kframes(void* frame_addr, const size_t frame_count);

/**
 * @brief Kernel memory page allocation.
 * 
 * @details Kernel memory page allocation. This method gets the desired number
 * of contiguous pages from the kernel page pool and allocate them.
 * 
 * @param[in] page_count The number of desired pages to allocate.
 * @param[out] err The error return pointer. OS_NO_ERR in case of success.
 * 
 * @return The address of the first page of the contiguous block is 
 * returned.
 */
void* alloc_kpages(const size_t page_count, OS_RETURN_E* err);

/**
 * @brief Kernel memory page release.
 * 
 * @details Kernel memory page release. This method releases the desired number
 * of contiguous pages to the kernel page pool.
 * 
 * @param[in] page_addr The address of the first page to release.
 * @param[in] page_count The number of desired pages to release.
 * 
 * @return OS_NO_ERR is returned on success. Otherwise an error code is 
 * returned.
 */
OS_RETURN_E free_kpages(void* page_addr, const size_t page_count);

#endif /* #ifndef __BSP_MEMINFO_H_ */