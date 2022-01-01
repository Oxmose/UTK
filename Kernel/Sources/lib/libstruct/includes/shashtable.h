/*******************************************************************************
 * @file shashtable.h
 *
 * @see shashtable.c
 *
 * @author Alexy Torres Aurora Dugo
 *
 * @date 31/12/2021
 *
 * @version 1.0
 *
 * @brief String hash table structures.
 *
 * @details String hash table structures. Hash table are used to dynamically
 * store data, while growing when needed. This type of hash table can store data
 * pointers and values of the size of a pointer.
 *
 * @warning This implementation is not thread safe.
 *
 * @copyright Alexy Torres Aurora Dugo
 ******************************************************************************/

#ifndef __LIB_SHASHTABLE_H_
#define __LIB_SHASHTABLE_H_

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

/** @brief String hashtable allocator structure. */
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
} shashtable_alloc_t;

/** @brief String hash table entry structure. */
typedef struct
{
    /** @brief Pointer sized key, contains a string integer of the size of a
     * pointer.
     */
    char* key;

    /** @brief Data associated to the key. */
    void* data;

    /** @brief Tells if the entry is used or not. */
    bool_t is_used;
} shashtable_entry_t;

/** @brief String hash table structure. */
typedef struct
{
    /** @brief Hash table allocator. */
    shashtable_alloc_t allocator;

    /** @brief Hash table entries. */
    shashtable_entry_t** entries;

    /** @brief Current hash table capacity. */
    size_t capacity;

    /** @brief Current hash table size. */
    size_t size;

    /** @brief Contains the number of deleted item still in the table. */
    size_t graveyard_size;
} shashtable_t;

/*******************************************************************************
 * MACROS
 ******************************************************************************/

/**
 * @brief Create an allocator structure.
 *
 * @param[in] malloc The memory allocation function used by the allocator.
 * @param[in] free The memory free function used by the alloctor.
 */
#define SHASHTABLE_ALLOCATOR(malloc, free) (shashtable_alloc_t){malloc, free}

/*******************************************************************************
 * GLOBAL VARIABLES
 ******************************************************************************/

/************************* Imported global variables **************************/
/* None */

/************************* Exported global variables **************************/
/* None */

/************************** Static global variables ***************************/
/* None */

/*******************************************************************************
 * FUNCTIONS
 ******************************************************************************/

/**
 * @brief Creates a new string hash table.
 *
 * @details Creates a new string hash table. The hash table will be
 * initialized and its memory allocated. It should be destroyed once the hash
 * table is no longer in use.
 *
 * @param[in] allocator The allocator to be used when allocating and freeing the
 * hash table.
 * @param[out] error The buffer to return the error status.
 *
 * @returns A pointer to the newly created table is returned.
 */
shashtable_t* shashtable_create(shashtable_alloc_t allocator,
                                OS_RETURN_E* error);

/**
 * @brief Destroys an string hash table.
 *
 * @details Destroys an string hash table. The memory used by the table is
 * released. It is the responsability of the user to free the memory used by the
 * data contained in the hash table.
 *
 * @param[out] table The string hash table to destroy.
 *
 * @returns The error status is returned.
 */
OS_RETURN_E shashtable_destroy(shashtable_t* table);

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
OS_RETURN_E shashtable_get(const shashtable_t* table,
                           const char* key,
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
OS_RETURN_E shashtable_set(shashtable_t* table,
                           const char* key,
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
OS_RETURN_E shashtable_remove(shashtable_t* table,
                              const char* key,
                              void** data);

#endif /* #ifndef __LIB_SHASHTABLE_H_ */

/************************************ EOF *************************************/