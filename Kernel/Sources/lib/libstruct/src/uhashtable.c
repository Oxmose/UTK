/*******************************************************************************
 * @file uhashtable.c
 *
 * @see uhashtable.h
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
 * @copyright Alexy Torres Aurora Dugo
 ******************************************************************************/

/*******************************************************************************
 * INCLUDES
 ******************************************************************************/

/* Included headers */
#include <stddef.h>        /* Standard definitions */
#include <stdint.h>        /* Generic int types */
#include <string.h>        /* String manipulation */
#include <kernel_error.h>  /* Kernel error codes */

/* Configuration files */
#include <config.h>
#include <test_bank.h>

/* Header file */
#include <uhashtable.h>

/*******************************************************************************
 * CONSTANTS
 ******************************************************************************/

/**
 * @brief Initial capacity and size of the hashtable.
 *
 * @warning Must be a power of 2.
 */
#define HT_INITIAL_SIZE 16

/** @brief Maximal factor size of the graveyard. */
#define HT_MAX_GRAVEYARD_FACTOR 0.3f

/** @brief Maximal load factor (including graveyard).
 *
 * @warning: Must always be strictly less than 1.0
*/
#define HT_MAX_LOAD_FACTOR 0.7f

/** @brief FNV offset used for the hash function. */
#define FNV_OFFSET_BASIS 14695981039346656037UL

/** @brief Prime used in the FNV hash. */
#define FNV_PRIME 1099511628211UL

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

/************************* Imported global variables **************************/
/* None */

/************************* Exported global variables **************************/
/* None */

/************************** Static global variables ***************************/
/* None */

/*******************************************************************************
 * STATIC FUNCTIONS DECLARATIONS
 ******************************************************************************/

/**
 * @brief 64 bits hash function for uintptr_t keys.
 *
 * @details This function computes the 64 bits hash function for uintptr_t keys.
 * The algorithm used is the FNV-1a hash algorithm.
 *
 * @param[in] key The key to hash.
 * @return The computed hash is returned.
 */
inline static uint64_t uhash_64(const uintptr_t key);

/**
 * @brief Sets the data for a given entry in the unsigned hash table.
 *
 * @details Sets the data for a given entry in the unsigned hash table. If the
 * entry does not exist it is created.
 *
 * @warning This function does not perform any check on the data, the key or the
 * table itself.
 *
 * @param table The table to set the value.
 * @param key The key to associate the data with.
 * @param data The data to set.
 *
 * @return The error status is returned.
 */
static OS_RETURN_E uhashtable_set_entry(uhashtable_t* table,
                                        const uintptr_t key,
                                        void* data);

/**
 * @brief Replaces an entry in the table, without allocating the entry.
 *
 * @details Replaces an entry in the table, without allocating the entry. The
 * entry beging used it the one given as parameter.
 *
 * @param[out] table The table to use.
 * @param[in] entry The entry to place.
 */
static void uhashtable_rehash_entry(uhashtable_t* table,
                                    uhashtable_entry_t* entry);

/**
 * @brief Rehashes the table and grows the table by a certain factor.
 *
 * @details Rehash the table and grows the table by a certain factor. The growth
 * factor must be greater or equal to 1. In the latter case, only the rehashing
 * is done.
 *
 * @param[out] table The table to grow.
 * @param[in] growth The growth factor to use.
 *
 * @return The error status is returned.
 */
static OS_RETURN_E uhashtable_rehash(uhashtable_t* table, const float growth);

/*******************************************************************************
 * FUNCTIONS
 ******************************************************************************/

inline static uint64_t uhash_64(const uintptr_t key)
{
    uint64_t hash;
    size_t   i;

    /* We use the FNV-1a method to hash our integer */
    hash = FNV_OFFSET_BASIS;

    for(i = 0; i < sizeof(uintptr_t); ++i)
    {
        hash ^= (uint64_t)((uint8_t)(key >> (i * 8)));
        hash *= FNV_PRIME;
    }

    return hash;
}

static OS_RETURN_E uhashtable_set_entry(uhashtable_t* table,
                                        const uintptr_t key,
                                        void* data)
{
    uint64_t hash;
    size_t   entry_index;

    /* Get the hash and ensure it does not overflow the current capacity */
    hash        = uhash_64(key);
    entry_index = (size_t)(hash & (uint64_t)(table->capacity - 1));

    /* Search for the entry in the table. Note that due to the fact that our
     * load factor should always be under 1.0, there is at least one entry that
     * is NULL. Hence we cannot have an infinite loop here. */
    while(table->entries[entry_index] != NULL &&
          table->entries[entry_index]->is_used == TRUE)
    {
        /* Found the entry */
        if(table->entries[entry_index]->key == key)
        {
            table->entries[entry_index]->data = data;
            return OS_NO_ERR;
        }

        /* Increment the index */
        entry_index = (entry_index + 1) % table->capacity;
    }

    /* If we are here, we did not find the data, allocate if needed */
    if(table->entries[entry_index] == NULL)
    {
        table->entries[entry_index] =
            table->allocator.malloc(sizeof(uhashtable_entry_t));
        if(table->entries[entry_index] == NULL)
        {
            return OS_ERR_MALLOC;
        }

        ++table->size;
    }
    else
    {
        /* We found an entry from the graveyard, remove it */
        --table->graveyard_size;
    }

    /* Set the data */
    table->entries[entry_index]->key     = key;
    table->entries[entry_index]->data    = data;
    table->entries[entry_index]->is_used = TRUE;

    return OS_NO_ERR;
}

static void uhashtable_rehash_entry(uhashtable_t* table,
                                    uhashtable_entry_t* entry)
{
    uint64_t hash;
    size_t   entry_index;

    /* Get the hash and ensure it does not overflow the current capacity */
    hash        = uhash_64(entry->key);
    entry_index = (size_t)(hash & (uint64_t)(table->capacity - 1));

    /* Search for the entry in the table. Note that due to the fact that our
     * load factor should always be under 1.0, there is at least one entry that
     * is NULL. Hence we cannot have an infinite loop here. */
    while(table->entries[entry_index] != NULL &&
          table->entries[entry_index]->is_used == TRUE)
    {
        /* Found the entry */
        if(table->entries[entry_index]->key == entry->key)
        {
            table->entries[entry_index]->data = entry->data;
            return;
        }

        /* Increment the index */
        entry_index = (entry_index + 1) % table->capacity;
    }

    /* Set the data */
    table->entries[entry_index] = entry;
}

static OS_RETURN_E uhashtable_rehash(uhashtable_t* table, const float growth)
{
    size_t               i;
    size_t               new_capacity;
    size_t               old_capacity;
    uhashtable_entry_t** new_entries;
    uhashtable_entry_t** old_entries;

    new_capacity = (size_t)((float)table->capacity * growth);

    /* Check if did not overflow on the size */
    if(new_capacity <= table->capacity)
    {
        return OS_ERR_OUT_OF_BOUND;
    }

    /* Save old entries */
    old_entries  = table->entries;
    old_capacity = table->capacity;

    /* Create a new entry table */
    new_entries = table->allocator.malloc(sizeof(uhashtable_entry_t*) *
                                          new_capacity);
    if(new_entries == NULL)
    {
        return OS_ERR_MALLOC;
    }

    memset(new_entries, 0, sizeof(uhashtable_entry_t*) * new_capacity);
    table->capacity = new_capacity;
    table->entries  = new_entries;

    /* Rehash the table, removes fragmentation and graveyard. */
    for(i = 0; i < old_capacity; ++i)
    {
        if(old_entries[i] != NULL)
        {
            /* Check if it was an entry that was used */
            if(old_entries[i]->is_used == TRUE)
            {
                uhashtable_rehash_entry(table, old_entries[i]);
            }
            else
            {
                /* The entry was not used, we can free it */
                table->allocator.free(old_entries[i]);
            }
        }
    }

    /* Clean data */
    table->graveyard_size = 0;
    table->allocator.free(old_entries);

    return OS_NO_ERR;
}

uhashtable_t* uhashtable_create(uhashtable_alloc_t allocator,
                                OS_RETURN_E* error)
{
    uhashtable_t* table;

    if(allocator.free == NULL || allocator.malloc == NULL)
    {
        if(error != NULL)
        {
            *error = OS_ERR_NULL_POINTER;
        }
        return NULL;
    }

    /* Initialize the table */
    table = allocator.malloc(sizeof(uhashtable_t));
    if(table == NULL)
    {
        if(error != NULL)
        {
            *error = OS_ERR_MALLOC;
        }
        return NULL;
    }

    table->entries = allocator.malloc(sizeof(uhashtable_entry_t*) *
                                      HT_INITIAL_SIZE);

    if(table->entries == NULL)
    {
        allocator.free(table);
        if(error != NULL)
        {
            *error = OS_ERR_MALLOC;
        }
        return NULL;
    }

    memset(table->entries, 0, sizeof(uhashtable_entry_t*) * HT_INITIAL_SIZE);
    table->allocator      = allocator;
    table->capacity       = HT_INITIAL_SIZE;
    table->size           = 0;
    table->graveyard_size = 0;

    if(error != NULL)
    {
        *error = OS_NO_ERR;
    }

    return table;
}

OS_RETURN_E uhashtable_destroy(uhashtable_t* table)
{
    size_t i;

    if(table == NULL || table->entries == NULL)
    {
        return OS_ERR_NULL_POINTER;
    }

    /* Free the memory used by the table */
    for(i = 0; i < table->capacity; ++i)
    {
        if(table->entries[i] != NULL)
        {
            table->allocator.free(table->entries[i]);
        }
    }
    table->allocator.free(table->entries);

    /* Reset attributes */
    table->size           = 0;
    table->capacity       = 0;
    table->graveyard_size = 0;
    table->entries        = NULL;

    /* Free memory */
    table->allocator.free(table);

    return OS_NO_ERR;
}

OS_RETURN_E uhashtable_get(const uhashtable_t* table,
                           const uintptr_t key,
                           void** data)
{
    uint64_t hash;
    size_t   entry_index;

    if(table == NULL || data == NULL || table->entries == NULL)
    {
        return OS_ERR_NULL_POINTER;
    }

    /* Get the hash and ensure it does not overflow the current capacity */
    hash        = uhash_64(key);
    entry_index = (size_t)(hash & (uint64_t)(table->capacity - 1));

    /* Search for the entry in the table. Note that due to the fact that our
     * load factor should always be under 1.0, there is at least one entry that
     * is NULL. Hence we cannot have an infinite loop here. */
    while(table->entries[entry_index] != NULL)
    {
        /* Found the entry */
        if(table->entries[entry_index]->key == key &&
           table->entries[entry_index]->is_used == TRUE)
        {
            *data = table->entries[entry_index]->data;
            return OS_NO_ERR;
        }

        /* Increment the index */
        entry_index = (entry_index + 1) % table->capacity;
    }

    return OS_ERR_NO_SUCH_ID;
}

OS_RETURN_E uhashtable_set(uhashtable_t* table,
                           const uintptr_t key,
                           void* data)
{
    OS_RETURN_E err;

    if(table == NULL || table->entries == NULL)
    {
        return OS_ERR_NULL_POINTER;
    }

    /* Check if the current load is under the threshold, double the size */
    if((float)table->capacity * HT_MAX_LOAD_FACTOR <
       table->size + table->graveyard_size)
    {
        err = uhashtable_rehash(table, 2);
        if(err != OS_NO_ERR)
        {
            return err;
        }
    }

    /* Insert the entry */
    return uhashtable_set_entry(table, key, data);
}

OS_RETURN_E uhashtable_remove(uhashtable_t* table,
                              const uintptr_t key,
                              void** data)
{
    uint64_t    hash;
    size_t      entry_index;
    OS_RETURN_E err;

    if(table == NULL || table->entries == NULL)
    {
        return OS_ERR_NULL_POINTER;
    }

    /* Get the hash and ensure it does not overflow the current capacity */
    hash        = uhash_64(key);
    entry_index = (size_t)(hash & (uint64_t)(table->capacity - 1));

    /* Check if the graveyard load is under the threshold */
    if((float)table->capacity * HT_MAX_GRAVEYARD_FACTOR < table->graveyard_size)
    {
        /* Rehash with a growth factor or 1 to keep the same capacity */
        err = uhashtable_rehash(table, 1);
        if(err != OS_NO_ERR)
        {
            return err;
        }
    }

    /* Search for the entry in the table. Note that due to the fact that our
     * load factor should always be under 1.0, there is at least one entry that
     * is NULL. Hence we cannot have an infinite loop here. */
    while(table->entries[entry_index] != NULL)
    {
        /* Found the entry */
        if(table->entries[entry_index]->key == key &&
           table->entries[entry_index]->is_used == TRUE)
        {
            if(data != NULL)
            {
                *data = table->entries[entry_index]->data;
            }
            table->entries[entry_index]->is_used = FALSE;

            ++table->graveyard_size;
            --table->size;

            return OS_NO_ERR;
        }

        /* Increment the index */
        entry_index = (entry_index + 1) % table->capacity;
    }

    return OS_ERR_NO_SUCH_ID;
}

/************************************ EOF *************************************/