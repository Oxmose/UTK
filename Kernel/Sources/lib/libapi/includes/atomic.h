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

#include <stdint.h>    /* Standard int definitons */
#include <cpu_api.h>   /* CPU API for atomic */

/* UTK configuration file */
#include <config.h>

/*******************************************************************************
 * CONSTANTS
 ******************************************************************************/

/** @brief Initial spinlock value */
#define SPINLOCK_INIT_VALUE 0

/**
 * @brief Initialize a spinlock.
 *
 * @details Initialize the spinlock to the start value.
 */
#define SPINLOCK_INIT(lock) {    \
    *lock = SPINLOCK_INIT_VALUE; \
}

/*******************************************************************************
 * STRUCTURES
 ******************************************************************************/

/**
 * @brief Defines the spinlock structure.
 *
 */
typedef volatile uint32_t spinlock_t;

/*******************************************************************************
 * FUNCTIONS
 ******************************************************************************/

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

#endif /* #ifndef __LIB_ATOMIC_H_ */