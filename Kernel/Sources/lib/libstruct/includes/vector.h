/*******************************************************************************
 * @file vector.h
 *
 * @see vector.c
 *
 * @author Alexy Torres Aurora Dugo
 *
 * @date 09/12/2021
 *
 * @version 1.0
 *
 * @brief Vector structures.
 *
 * @details Vector structures. Vectors are used to dynamically store data, while
 * growing when needed. This type of vector can store data pointers and values
 * of the size of a pointer.
 * 
 * @warning This implementation is not thread safe.
 *
 * @copyright Alexy Torres Aurora Dugo
 ******************************************************************************/

#ifndef __LIB_VECTOR_H_
#define __LIB_VECTOR_H_

#include <stddef.h>       /* Standard definitons */
#include <stdint.h>       /* Generic int types */
#include <kernel_error.h> /* Kernel error codes */

/* UTK configuration file */
#include <config.h>

/*******************************************************************************
 * CONSTANTS
 ******************************************************************************/

/* None */

/*******************************************************************************
 * STRUCTURES
 ******************************************************************************/


/** @brief Vector allocator structure. */
struct vector_alloc
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
};

/**
 * @brief Defines vector_alloc_t type as a shorcut for struct vector_alloc.
 */
typedef struct vector_alloc vector_alloc_t;

/** @brief Vector structure. */
struct vector
{
    /** @brief The allocator used by this vector. */
    vector_alloc_t allocator;

    /** @brief Storage array of the vector */
    void** array;

    /** @brief Current vector's size. */
    size_t size;

    /** @brief Current vector's capacity. */
    size_t capacity;
};

/**
 * @brief Defines vector_t type as a shorcut for struct vector.
 */
typedef struct vector vector_t;

/*******************************************************************************
 * FUNCTIONS
 ******************************************************************************/

/** 
 * @brief Create an allocator structure.
 * 
 * @param[in] malloc The memory allocation function used by the allocator.
 * @param[in] free The memory free function used by the allocator.
 */
#define VECTOR_ALLOCATOR(malloc, free) (vector_alloc_t){malloc, free}

/**
 * @brief Creates a new vector.
 *
 * @details Creates a new vector of the size given as parameter. The vector will
 * allocate the required memory and initialize the memory with the value 
 * provided as parameter.
 * 
 * @param[out] vector A pointer to the vector to initialize.
 * @param[in] allocator The allocator to be used when allocating and freeing the
 * vector.
 * @param[in] init_data The initial data to set to the elements of the vector 
 * during its creation.
 * @param[in] size The initial size of the vector.
 * 
 * @returns The error status is returned.
 */
OS_RETURN_E vector_init(vector_t* vector,
                        vector_alloc_t allocator,
                        void* init_data,
                        const size_t size);

/**
 * @brief Destroys a vector.
 *
 * @details Destroys a vector. The memory used by the vector is released, 
 * however, if its elements were allocated dynamically, it is the responsability
 * of the user to release the memory of these elements.
 * 
 * @param[out] vector The vector to destroy.
 *
 * @returns The error status is retuned.
 */
OS_RETURN_E vector_destroy(vector_t* vector);

/**
 * @brief Clears a vector.
 *
 * @details Clears a vector. The memory used by the vector is not released and 
 * its capacity unchanged. If its elements were allocated dynamically, it is the 
 * responsability of the user to release the memory of these elements.
 * 
 * @param[out] vector The vector to clear.
 *
 * @returns The error status is retuned.
 */
OS_RETURN_E vector_clear(vector_t* vector);

/**
 * @brief Performs a copy of a vector.
 * 
 * @details Copies a vector to another vector. The destination vector is 
 * initialized. It is the responsability of the user to make sure the
 * destination vector already released all its resources. The function 
 * initialize the destination vector before performing the copy.
 * 
 * @param[out] dst The destination vector.
 * @param[in] src The source vector.
 * 
 * @return The error status is retuned.
 */
OS_RETURN_E vector_copy(vector_t* dst, const vector_t* src);

/**
 * @brief Shrinks a vector.
 *
 * @details Shrinks a vector. The memory used by the vector that is empty is 
 * released and its capacity set to fit the number of element that the vector 
 * contains.
 * 
 * @param[out] vector The vector to shrink.
 *
 * @returns The error status is retuned.
 */
OS_RETURN_E vector_shrink_to_fit(vector_t* vector);

/**
 * @brief Resizes a vector.
 *
 * @details Resizes a vector. If the size set is smaller than the current 
 * capacity of the vector, the size is reduced but the capacity unchanged.
 *  If its elements were allocated dynamically, it is the responsability of the 
 * user to release the memory of these elements. If the size is biger than the 
 * vector's capacity, it is expanded to fit the new size.
 * 
 * @param[out] vector The vector to resize.
 * @param[in] size The size of apply to the vector.
 *
 * @returns The error status is retuned.
 */
OS_RETURN_E vector_resize(vector_t* vector, const size_t size);

/**
 * @brief Inserts an element in the vector at the position provided in the
 * parameters.
 * 
 * @details Inserts an element in the vector at the position provided in the
 * parameters. If the position is greater than the size, an error is returned.
 * 
 * @param[out] vector The vector to modify.
 * @param[in] data The data to insert.
 * @param[in] position The position to insert the data to.
 * 
 * @return The error status is retuned. 
 */
OS_RETURN_E vector_insert(vector_t* vector, 
                          void* data, 
                          const size_t position);

/**
 * @brief Inserts an element at the end of the vector.
 * 
 * @details Inserts an element at the end of the vector.
 * 
 * @param[out] vector The vector to modify.
 * @param[in] data The data to insert.
 * 
 * @return The error status is retuned. 
 */
OS_RETURN_E vector_push(vector_t* vector, void* data);

/**
 * @brief Removes an element at the end of the vector.
 * 
 * @details Removes an element at the end of the vector.
 * 
 * @param[out] vector The vector to modify.
 * @param[out] data The data buffer to retreive the element that was removed.
 * 
 * @return The error status is retuned. 
 */
OS_RETURN_E vector_pop(vector_t* vector, void** data);

/**
 * @brief Returns the value of an element.
 * 
 * @details Returns the value of an element at the position provided in
 * parameters. Note that the vector's array can be accesses directly from the 
 * structure's attributes. This function tests the bounds of the vector to
 * ensure the user accesses a position inside the vector.
 * 
 * @param[in] vector The vector to use.
 * @param[in] position The position of the element to retrieve.
 * @param[out] data The data buffer to retreive the element.
 * 
 * @return The error status is retuned. 
 */
OS_RETURN_E vector_get(const vector_t* vector, 
                       const size_t position, 
                       void** data);

/**
 * @brief Sets the value of an element.
 * 
 * @details Sets the value of an element at the position provided in
 * parameters. Note that the vector's array can be accesses directly from the 
 * structure's attributes. This function tests the bounds of the vector to
 * ensure the user accesses a position inside the vector.
 * 
 * @param[out] vector The vector to use.
 * @param[in] position The position of the element to retrieve.
 * @param[in] data The data to set at the given position.
 * 
 * @return The error status is retuned. 
 */
OS_RETURN_E vector_set(vector_t* vector, const size_t position, void* data);

#endif /* #ifndef __LIB_VECTOR_H_ */