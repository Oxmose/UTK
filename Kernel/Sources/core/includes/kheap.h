/*******************************************************************************
 * @file kheap.h
 *
 * @see kheap.c
 *
 * @author Alexy Torres Aurora Dugo
 *
 * @date 28/02/2021
 *
 * @version 1.0
 *
 * @brief Kernel's heap allocator.
 *
 * @details Kernel's heap allocator. Allow to dynamically allocate and dealocate
 * memory on the kernel's heap.
 *
 * @warning This allocator is not suited to allocate memory for the process, you
 * should only use it for the kernel.
 *
 * @copyright Alexy Torres Aurora Dugo
 ******************************************************************************/

#ifndef __CORE_KHEAP_H_
#define __CORE_KHEAP_H_

/*******************************************************************************
 * INCLUDES
 ******************************************************************************/

#include <stdint.h> /* Generic int types */
#include <stddef.h> /* Standard definitions */

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

/* Imported global variables */
/* None */

/* Exported global variables */
/* None */

/* Static global variables */
/* None */

/*******************************************************************************
 * FUNCTIONS
 ******************************************************************************/

/**
 * @brief Initializes the kernel's heap.
 *
 * @details Setups kernel heap management. It will also allign kernel heap start
 * and initialize the basic heap parameters such as its size.
 */
void kheap_init(void);

/**
 * @brief Allocate memory from the kernel heap.
 *
 * @details Allocate a chunk of memory form the kernel heap and returns the
 * start address of the chunk.
 *
 * ­@param[in] size The number of byte to allocate.
 *
 * @return A pointer to the start address of the allocated memory is returned.
 * If the memory cannot be allocated, this pointer will be NULL.
 */
void* kmalloc(const size_t size);

/**
 * @brief Free allocated memory.
 *
 * @details Releases allocated memory.IOf the pointer is NULL or has not been
 * allocated previously from the heap, nothing is done.
 *
 * @param[in, out] ptr The start address of the memory area to free.
 */
void kfree(void* ptr);

/**
 * @brief Returns the kernel heap available memory.
 *
 * @details Returns the kernel heap available memory.
 *
 * @return The kernel heap available memory is returned.
 */
uint32_t kheap_get_free(void);

#endif /* #ifndef __CORE_KHEAP_H_ */

/* EOF */