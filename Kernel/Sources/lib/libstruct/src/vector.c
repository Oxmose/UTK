/*******************************************************************************
 * @file vector.c
 *
 * @see vector.h
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
 * @copyright Alexy Torres Aurora Dugo
 ******************************************************************************/

#include <stddef.h>        /* Standard definitions */
#include <stdint.h>        /* Generic int types */
#include <string.h>        /* String manipulation */
#include <kernel_error.h>  /* Kernel errors */

/* UTK configuration file */
#include <config.h>

/* Header file */
#include <vector.h>

/*******************************************************************************
 * CONSTANTS
 ******************************************************************************/

/** 
 * @brief Growth factor used when the vector has not space left.
 * 
 * @warning This value must be greater than 1.
*/
#define VECTOR_GROWTH_FACTOR 2

/*******************************************************************************
 * STRUCTURES
 ******************************************************************************/

/* None */

/*******************************************************************************
 * GLOBAL VARIABLES
 ******************************************************************************/

/* None */

/*******************************************************************************
 * STATIC FUNCTIONS DECLARATION
 ******************************************************************************/

/* None */


/*******************************************************************************
 * FUNCTIONS
 ******************************************************************************/

/** 
 * @brief Used to grow the size of a vector. The new vector is filled with 
 * previous data.
 * 
 * @param[out] vector The vector to update.
 * @param[out] new_size The new size to be computed.
 * @param[out] new_array The array to receive the created memory region.
*/

#define GROW_VECTOR_SIZE(vector, new_size, new_array) {                     \
    if(vector->capacity == vector->size)                                    \
    {                                                                       \
        new_size = vector->capacity * VECTOR_GROWTH_FACTOR;                 \
        if(new_size == 0)                                                   \
        {                                                                   \
            new_size = VECTOR_GROWTH_FACTOR;                                \
        }                                                                   \
                                                                            \
        /* Check if did not overflow on the size */                         \
        if(new_size <= vector->capacity)                                    \
        {                                                                   \
            return OS_ERR_OUT_OF_BOUND;                                     \
        }                                                                   \
                                                                            \
        /* Allocate new array */                                            \
        new_array = vector->allocator.malloc(new_size * sizeof(void*));     \
        if(new_array == NULL)                                               \
        {                                                                   \
            return OS_ERR_MALLOC;                                           \
        }                                                                   \
                                                                            \
        /* Copy array */                                                    \
        memcpy(new_array, vector->array, vector->size * sizeof(void*));     \
                                                                            \
        /* Free old array */                                                \
        vector->allocator.free(vector->array);                              \
        vector->array    = new_array;                                       \
        vector->capacity = new_size;                                        \
    }                                                                       \
}

OS_RETURN_E vector_init(vector_t* vector,
                        vector_alloc_t allocator,
                        void* init_data,
                        const size_t size)
{
    size_t i;

    if(vector == NULL || allocator.malloc == NULL || allocator.free == NULL)
    {
        return OS_ERR_NULL_POINTER;
    }

    /* Allocate the data */
    vector->array = NULL;
    if(size != 0)
    {
        vector->array = allocator.malloc(size * sizeof(void*));
        if(vector->array == NULL)
        {
            return OS_ERR_MALLOC;
        }
    }

    /* Initialize the data */
    for(i = 0; i < size; ++i)
    {
        vector->array[i] = init_data;
    }

    /* Initialize the attributes */
    vector->allocator = allocator;
    vector->size      = size;
    vector->capacity  = size;

    return OS_NO_ERR;
}

OS_RETURN_E vector_destroy(vector_t* vector)
{
    if(vector == NULL)
    {
        return OS_ERR_NULL_POINTER;
    }

    /* Release the data */
    if(vector->array != NULL)
    {
        vector->allocator.free(vector->array);
    }

    /* Reset the attributes */
    vector->array     = NULL;
    vector->size      = 0;
    vector->capacity  = 0;

    return OS_NO_ERR;
}

OS_RETURN_E vector_clear(vector_t* vector)
{
    if(vector == NULL)
    {
        return OS_ERR_NULL_POINTER;
    }

    vector->size = 0;

    return OS_NO_ERR;
}

OS_RETURN_E vector_copy(vector_t* dst, const vector_t* src)
{
    OS_RETURN_E err;

    if(dst == NULL || src == NULL)
    {
        return OS_ERR_NULL_POINTER;
    }

    err = vector_init(dst, src->allocator, NULL, src->capacity);
    if(err != OS_NO_ERR)
    {
        return err;
    }

    /* Here we do not need to copy the entire array, just the part that contains
     * valid data as size <= capacity.
     */
    dst->size = src->size;
    memcpy(dst->array, src->array, src->size * sizeof(void*));

    return OS_NO_ERR;
}

OS_RETURN_E vector_shrink_to_fit(vector_t* vector)
{
    void*       new_array;

    if(vector == NULL)
    {
        return OS_ERR_NULL_POINTER;
    }

    /* Only resize if the capacity is different than the size */
    if(vector->capacity > vector->size)
    {
        if(vector->size != 0)
        {
            /* Allocate new array */
            new_array = vector->allocator.malloc(vector->size * sizeof(void*));
            if(new_array == NULL)
            {
                return OS_ERR_MALLOC;
            }

            /* Copy array */
            memcpy(new_array, vector->array, vector->size * sizeof(void*));

            /* Free old array */
            vector->allocator.free(vector->array);
            vector->array    = new_array;
            vector->capacity = vector->size;
        }
        else 
        {
            /* Free all memory */
            vector->allocator.free(vector->array);
            vector->array    = NULL;
            vector->capacity = 0;
        }
    }    

    return OS_NO_ERR;
}

OS_RETURN_E vector_resize(vector_t* vector, const size_t size)
{
    void*       new_array;

    if(vector == NULL)
    {
        return OS_ERR_NULL_POINTER;
    }

    /* Only resize if the capacity is different than the capacity */
    if(vector->capacity < size)
    {
        if(size != 0)
        {
            /* Allocate new array */
            new_array = vector->allocator.malloc(size * sizeof(void*));
            if(new_array == NULL)
            {
                return OS_ERR_MALLOC;
            }

            /* Copy array */
            memcpy(new_array, 
                vector->array, 
                MAX(vector->size, size) * sizeof(void*));

            /* Free old array */
            vector->allocator.free(vector->array);
            vector->array    = new_array;
            vector->capacity = size;
            vector->size     = size;
        }
        else 
        {
            /* Free all memory */
            vector->allocator.free(vector->array);
            vector->array    = NULL;
            vector->capacity = 0;
            vector->size     = 0;
        }
    }    
    else 
    {
        vector->size = size;
    }

    return OS_NO_ERR;
}

OS_RETURN_E vector_insert(vector_t* vector, 
                          void* data, 
                          const size_t position)
{
    size_t new_size;
    size_t i;
    void** new_array;

    if(vector == NULL)
    {
        return OS_ERR_NULL_POINTER;
    }

    if(position > vector->size)
    {
        return OS_ERR_OUT_OF_BOUND;
    }

    /* First, check if we should update the capacity of the vector */
    GROW_VECTOR_SIZE(vector, new_size, new_array);
    
    /* Move the old data and insert the new data */    
    for(i = vector->size; i > position; --i)
    {
        vector->array[i] = vector->array[i - 1];
    }
    vector->array[position] = data;
    ++vector->size;
    
    return OS_NO_ERR;
}

OS_RETURN_E vector_push(vector_t* vector, void* data)
{
    size_t new_size;
    void** new_array;

    if(vector == NULL)
    {
        return OS_ERR_NULL_POINTER;
    }

    /* First, check if we should update the capacity of the vector */
    GROW_VECTOR_SIZE(vector, new_size, new_array);
    
    /* Insert the new data */    
    vector->array[vector->size++] = data;

    return OS_NO_ERR;
}

OS_RETURN_E vector_pop(vector_t* vector, void** data)
{
    if(vector == NULL || data == NULL)
    {
        return OS_ERR_NULL_POINTER;
    }

    if(vector->size == 0)
    {
        return OS_ERR_OUT_OF_BOUND;
    }
    
    /* Return the last data */    
    *data = vector->array[--vector->size];

    return OS_NO_ERR;
}

OS_RETURN_E vector_get(const vector_t* vector, 
                       const size_t position, 
                       void** data)
{
    if(vector == NULL || data == NULL)
    {
        return OS_ERR_NULL_POINTER;
    }

    if(position >= vector->size)
    {
        return OS_ERR_OUT_OF_BOUND;
    }

    /* Return the data */    
    *data = vector->array[position]; 

    return OS_NO_ERR;
}

OS_RETURN_E vector_set(vector_t* vector, const size_t position, void* data)
{
    if(vector == NULL)
    {
        return OS_ERR_NULL_POINTER;
    }

    if(position >= vector->size)
    {
        return OS_ERR_OUT_OF_BOUND;
    }

    /* Sets the data */    
    vector->array[position] = data; 

    return OS_NO_ERR;
}