/*******************************************************************************
 * @file memalloc.h
 *
 * @see memalloc.c
 *
 * @author Alexy Torres Aurora Dugo
 *
 * @date 20/10/2018
 *
 * @version 1.0
 *
 * @brief Kernel memory physical frame and virtual page allocator.
 *
 * @details Kernel memory physical frame and virtual page allocator. This module
 * allows to allocate and deallocate physical memory frame and virtual pages.
 *
 * @copyright Alexy Torres Aurora Dugo
 ******************************************************************************/

#ifndef __MEMORY_MEMALLOC_H_
#define __MEMORY_MEMALLOC_H_

#include <lib/stddef.h> /* Standard definitions */
#include <lib/stdint.h> /* Generic int types */

/*******************************************************************************
 * CONSTANTS
 ******************************************************************************/

/*******************************************************************************
 * STRUCTURES
 ******************************************************************************/

/**
 * @brief Memory area definition structure. Keeps the information about an area
 * in the system.
 */
struct mem_area
{
    /** @brief Area's start address. */
    uintptr_t start;

    /** @brief Area's size. */
    size_t size;

    /** @brief Previous node of the structure. */
    struct mem_area* next;

    /** @brief Next node of the structure. */
    struct mem_area* prev;
};

/** @brief Shortcut for struct mem_area type. */
typedef struct mem_area mem_area_t;

/** @brief Kernel free frame pool. */
extern mem_area_t* kernel_free_frames;

/** @brief Kernel free page pool. */
extern mem_area_t* kernel_free_pages;


/*******************************************************************************
 * FUNCTIONS
 ******************************************************************************/

/**
 * @brief Initilizes the memory allocator.
 * 
 * @details Initializes the memory allocator by detecting the kernel memory 
 * ranges. The memory pools are then initialized.
 * 
 * @return OS_NO_ERR is returned in case of success. Otherwise an error code is
 * returned.
 */
OS_RETURN_E memalloc_init(void);

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
void* memalloc_alloc_kframes(const size_t frame_count, OS_RETURN_E* err);

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
OS_RETURN_E memalloc_free_kframes(void* frame_addr, const size_t frame_count);

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
void* memalloc_alloc_kpages(const size_t page_count, OS_RETURN_E* err);

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
OS_RETURN_E memalloc_free_kpages(void* page_addr, const size_t page_count);

#endif /* #ifndef __MEMORY_MEMALLOC_H_ */