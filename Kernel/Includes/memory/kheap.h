/*******************************************************************************
 * @file kheap.h
 * 
 * @see kheap.c
 *
 * @author Alexy Torres Aurora Dugo
 *
 * @date 10/01/2018
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

#ifndef __MEMORY_KHEAP_H_
#define __MEMORY_KHEAP_H_

#include <lib/stdint.h> /* Generic int types */
#include <lib/stddef.h> /* Standard definitions */

/*******************************************************************************
 * CONSTANTS
 ******************************************************************************/

/*******************************************************************************
 * STRUCTURES
 ******************************************************************************/

/** @brief Kernel's heap allocator list node. */
struct list
{
    /** @brief Next node of the list. */
    struct list* next;
    /** @brief Previous node of the list. */
    struct list* prev;
};

/** 
 * @brief Defines list_t type as a shorcut for struct list.
 */
typedef struct list list_t;

/** @brief Kernel's heap allocator memory chunk representation. */
struct mem_chunk
{
    /** @brief Memory chunk list. */
    list_t all;

    /** @brief Used flag. */
    int8_t used;
    union

    /** @brief If used, the union contains the chunk's data, else a list of free
     * mem.
     */
    {
	       uint8_t* data;
	       list_t   free;
    };
};

/** 
 * @brief Defines mem_chunk_t type as a shorcut for struct mem_chunk.
 */
typedef struct mem_chunk mem_chunk_t;

/** @brief Kernel's heap allocator settings. */
enum heap_enum
{
    /** @brief Num size. */
    NUM_SIZES   = 32,

    /** @brief Memory chunk alignement. */
    ALIGN       = 4,

    /** @brief Chink minimal size. */
    MIN_SIZE    = sizeof(list_t),

    /** @brief Header size. */
    HEADER_SIZE = __builtin_offsetof(mem_chunk_t, data),
};

/*******************************************************************************
 * FUNCTIONS
 ******************************************************************************/

/**
 * @brief Initializes the kernel's heap.
 * 
 * @details Setups kernel heap management. It will also allign kernel heap start
 * and initialize the basic heap parameters such as its size.
 * 
 * @return OS_NO_ERR is returned on success, otherwise an error code is 
 * returned.
 */
OS_RETURN_E kheap_init(void);

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

#endif /* #ifndef __MEMORY_KHEAP_H_ */