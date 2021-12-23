/*******************************************************************************
 * @file atomic.h
 *
 * @see atomic.c
 *
 * @author Alexy Torres Aurora Dugo
 *
 * @date 05/12/2021
 *
 * @version 1.0
 *
 * @brief Atomic primitives implementation.
 *
 * @details Atomic primitives implementation. Defines the different basic
 * synchronization primitives used in the kernel and by the user.
 *
 * @copyright Alexy Torres Aurora Dugo
 ******************************************************************************/

#ifndef __LIB_ATOMIC_H_
#define __LIB_ATOMIC_H_

/*******************************************************************************
 * INCLUDES
 ******************************************************************************/

#include <stdint.h>    /* Standard int definitons */
#include <cpu_api.h>   /* CPU API for atomic */

/*******************************************************************************
 * CONSTANTS
 ******************************************************************************/

/** @brief Initial spinlock value */
#define SPINLOCK_INIT_VALUE 0

/*******************************************************************************
 * STRUCTURES AND TYPES
 ******************************************************************************/

/**
 * @brief Defines the spinlock structure.
 *
 */
typedef volatile uint32_t spinlock_t;

/*******************************************************************************
 * MACROS
 ******************************************************************************/

/**
 * @brief Initialize a spinlock.
 *
 * @details Initialize the spinlock to the start value.
 */
#define SPINLOCK_INIT(lock) {    \
    *lock = SPINLOCK_INIT_VALUE; \
}

/**
 * @brief Lock the spinlock.
 *
 * @details Lock the spinlock if possible. If not, the calling thread will wait
 * in this function until the lock is free.
 *
 * @param lock The spinlock to use.
 */
#define SPINLOCK_LOCK(lock) { \
    cpu_lock_spinlock(&lock); \
}

/**
 * @brief Unlock the spinlock.
 *
 * @details Unlock the spinlock.This function is non blocking.
 *
 * @param lock The spinlock to use.
 */
#define SPINLOCK_UNLOCK(lock) { \
    lock = 0;                   \
}

/**
 * @brief Compare and swap primitive.
 *
 * @details The compare and swap primitive allows to compare a value stored in
 * the memory and replace it with a new value is the old value equals a certain
 * value.
 *
 * @param[in, out] memory The memory region to be compared and swaped, on
 * 32bits.
 * @param[in] old_val The old value to compare.
 * @param[in] new_val The new value to compare.
 *
 * @return The value stored in the memory is returned.
 */
#define ATOMIC_CAS(memory, old_val, new_val) \
    cpu_compare_and_swap(memory, old_val, new_val)

/**
 * @brief Atomically fecth the value in the memory and add a givne value to it.
 *
 * @details This primitive fecthes the value stored at the memory region given
 * as parameter. It adds the value provided in parameters and returns the value
 * before the addition operation.
 *
 * @param[out] memory The memory region to fetch and add.
 * @param[in] val The value to add.
 *
 * @returns The previous value contained in the memory region is returned.
 */

#define ATOMIC_FETCH_ADD(memory, val) cpu_fetch_and_add(memory, val)

/**
 * @brief Atomically stores a value in the memory.
 *
 * @details Atomically stores a value in the memory.
 *
 * @param[out] memory The memory region to modify.
 * @param[in] val The value to store.
 */
#define ATOMIC_STORE(memory, val) cpu_atomic_store(memory, val)

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

/* None */

#endif /* #ifndef __LIB_ATOMIC_H_ */

/* EOF */