/*******************************************************************************
 * @file uhashtable.h
 *
 * @see uhashtable.c
 *
 * @author Alexy Torres Aurora Dugo
 *
 * @date 11/12/2021
 *
 * @version 1.0
 *
 * @brief Unsigned hash table structures.
 *
 * @details Unsigned hash table structures. Hash table are used to dynamically
 * store data, while growing when needed. This type of hash table can store data
 * pointers and values of the size of a pointer.
 *
 * @warning This implementation is not thread safe.
 *
 * @copyright Alexy Torres Aurora Dugo
 ******************************************************************************/

#ifndef __LIB_UHASHTABLE_H_
#define __LIB_UHASHTABLE_H_

/*******************************************************************************
 * INCLUDES
 ******************************************************************************/

#include <stddef.h>       /* Standard definitons */
#include <stdint.h>       /* Generic int types */
#include <kernel_error.h> /* Kernel error codes */

/*******************************************************************************
 * CONSTANTS
 ******************************************************************************/

/* None */

/*******************************************************************************
 * STRUCTURES AND TYPES
 ******************************************************************************/

/** @brief Unsigned hashtable allocator structure. */
typedef struct
{
    /**
     * @brief The memory allocation function used by the allocator.
     *
     * @param[in] alloc_size The size in bytes to be allocated.
     *
     * @return A pointer to the allocated memory is returned. NULL is returned
     * if no memory was allocated.
     */
    void*(*malloc)(size_t alloc_size);

    /**
     * @brief The memory free function used by the allocator.
     *
     * @param[out] ptr The start address of the memory to free.
     */
    void(*free)(void* ptr);
} uhashtable_alloc_t;

/** @brief Unsigned hash table entry structure. */
typedef struct
{
    /** @brief Pointer sized key, contains a unsigned integer of the size of a
     * pointer.
     */
    uintptr_t key;

    /** @brief Data associated to the key. */
    void* data;

    /** @brief Tells if the entry is used or not. */
    bool_t is_used;
} uhashtable_entry_t;

/** @brief Unsigned hash table structure. */
typedef struct
{
    /** @brief Hash table allocator. */
    uhashtable_alloc_t allocator;

    /** @brief Hash table entries. */
    uhashtable_entry_t** entries;

    /** @brief Current hash table capacity. */
    size_t capacity;

    /** @brief Current hash table size. */
    size_t size;

    /** @brief Contains the number of deleted item still in the table. */
    size_t graveyard_size;
} uhashtable_t;

/*******************************************************************************
 * MACROS
 ******************************************************************************/

/**
 * @brief Create an allocator structure.
 *
 * @param[in] malloc The memory allocation function used by the allocator.
 * @param[in] free The memory free function used by the alloctor.
 */
#define UHASHTABLE_ALLOCATOR(malloc, free) (uhashtable_alloc_t){malloc, free}

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
 * @brief Creates a new unsigned hash table.
 *
 * @details Creates a new unsigned hash table. The hash table will be
 * initialized and its memory allocated. It should be destroyed once the hash
 * table is no longer in use.
 *
 * @param[in] allocator The allocator to be used when allocating and freeing the
 * hash table.
 * @param[out] error The buffer to return the error status.
 *
 * @returns A pointer to the newly created table is returned.
 */
uhashtable_t* uhashtable_create(uhashtable_alloc_t allocator,
                                OS_RETURN_E* error);

/**
 * @brief Destroys an unsigned hash table.
 *
 * @details Destroys an unsigned hash table. The memory used by the table is
 * released. It is the responsability of the user to free the memory used by the
 * data contained in the hash table.
 *
 * @param[out] table The unsigned hash table to destroy.
 *
 * @returns The error status is returned.
 */
OS_RETURN_E uhashtable_destroy(uhashtable_t* table);

/**
 * @brief Returns the value attached to the key provided in parameters.
 *
 * @details Returns the value attached to the key provided in parameters. If the
 * key is not in the table, OS_ERR_NO_SUCH_ID is returned.
 *
 * @param[in] table The table to search.
 * @param[in] key The key to search.
 * @param[out] data The data buffer to receive the data associated to the key.
 *
 * @returns The error status is returned.
 */
OS_RETURN_E uhashtable_get(const uhashtable_t* table,
                           const uintptr_t key,
                           void** data);

/**
 * @brief Sets a value in the hash table.
 *
 * @details Sets a value in the hash table. If the value already exists, the
 * previous value is overwritten, otherwise, a new entry is created.
 *
 * @param[in,out] table The table to set the data into.
 * @param[in] key The key to associate to the data.
 * @param[in] data The data to set in the table.
 *
 * @returns The error status is returned.
 */
OS_RETURN_E uhashtable_set(uhashtable_t* table,
                           const uintptr_t key,
                           void* data);

/**
 * @brief Removes an entry from the table.
 *
 * @details Removes an entry from the table. This function returns the value
 * attached to the key provided in parameters. If the
 * key is not in the table, OS_ERR_NO_SUCH_ID is returned.
 *
 * @param[in] table The table to remove the entry in.
 * @param[in] key The key to remove.
 * @param[out] data The data buffer to receive the data associated to the key.
 * This parameter can be set to NULL is the user does not want to retreive the
 * removed data.
 *
 * @returns The error status is returned.
 */
OS_RETURN_E uhashtable_remove(uhashtable_t* table,
                              const uintptr_t key,
                              void** data);

#endif /* #ifndef __LIB_UHASHTABLE_H_ */

/* EOF */