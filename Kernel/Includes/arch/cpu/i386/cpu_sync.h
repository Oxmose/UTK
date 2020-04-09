/*******************************************************************************
 * @file cpu_sync.h
 *
 * @author Alexy Torres Aurora Dugo
 *
 * @date 07/04/2020
 *
 * @version 1.0
 *
 * @brief i386 CPU synchronization functions
 *
 * @details i386 CPU synchronization functions. Wraps inline assembly calls for 
 * ease of development.
 *
 * @copyright Alexy Torres Aurora Dugo
 ******************************************************************************/

#ifndef __I386_CPU_SYNC_H_
#define __I386_CPU_SYNC_H_

#include <lib/stdint.h>           /* Generic int types */
#include <lib/stddef.h>           /* Standard definition */

/*******************************************************************************
 * CONSTANTS
 ******************************************************************************/

/* None */

/*******************************************************************************
 * STRUCTURES
 ******************************************************************************/

/* None */

/*******************************************************************************
 * FUNCTIONS
 ******************************************************************************/

/**
 * @brief Compare and swap word atomicaly.
 *
 * @details This function can be used by synchronization primitive to compare
 * and swap a word atomicaly. cpu_compare_and_swap implements the usual compare
 * and swap behavior.
 *
 * @return The value of the lock, either old val if the lock was not aquired or
 * newval if the lock was aquired.
 *
 * @param[in,out] p_val The pointer to the lock.
 * @param[in] oldval The old value to swap.
 * @param[in] newval The new value to be swapped.
 */
__inline__ static uint32_t cpu_compare_and_swap(volatile uint32_t* p_val,
                                                const int oldval,
                                                const int newval)
{
    uint32_t prev;
    __asm__ __volatile__ (
            "lock cmpxchg %1, %2\n"
            "setne %%al"
                : "=a" (prev)
                : "r" (newval), "m" (*p_val), "0" (oldval)
                : "memory");
    return prev;
}

/**
 * @brief Test and set atomic operation.
 *
 * @details This function can be used by synchronization primitive to test
 * and set a word atomicaly. s implements the usual test and set behavior.
 *
 * @param[in,out] lock The spinlock to apply the test on.
 */
__inline__ static int cpu_test_and_set(volatile uint32_t* lock)
{
        return cpu_compare_and_swap(lock, 0, 1);
}

/**
 * @brief Returns the current CPU id.
 * 
 * @details The function returns the CPU id on which the call is made.
 *
 * @return The current CPU id is returned on succes and -1 is return in case 
 * of error.
 */
int32_t cpu_get_id(void);

/**
 * @brief Returns the number of booted CPU.
 *
 * @return uint32_t The number of booted CPU.
 */
uint32_t cpu_get_booted_cpu_count(void);

#endif /* #ifndef __I386_CPU_SYNC_H_ */
