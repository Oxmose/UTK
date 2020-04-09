/*******************************************************************************
 * @file semaphore.h
 *
 * @see semaphore.c
 *
 * @author Alexy Torres Aurora Dugo
 *
 * @date 17/12/2017
 *
 * @version 2.0
 *
 * @brief Semaphore synchronization primitive.
 *
 * @details Semaphore synchronization primitive implemantation.
 * The semaphore are used to synchronyse the threads. The semaphore waiting list
 * is a FIFO with no regards of the waiting threads priority.
 *
 * @warning Semaphores can only be used when the current system is running and
 * the scheduler initialized.
 *
 * @copyright Alexy Torres Aurora Dugo
 ******************************************************************************/

#ifndef __SYNC_SEMAPHORE_H_
#define __SYNC_SEMAPHORE_H_

#include <lib/stddef.h>        /* Standard definitions */
#include <lib/stdint.h>        /* Generic int types */
#include <core/kernel_queue.h> /* Kernel queues */
#include <sync/critical.h>     /* Critical sections */

/*******************************************************************************
 * CONSTANTS
 ******************************************************************************/

/*******************************************************************************
 * STRUCTURES
 ******************************************************************************/

/** @brief Semaphore structure definition. */
struct semaphore
{
    /** @brief FIFO threads waiting queue. Stores the threads locked on the
     * semaphore.
     */
    kernel_queue_t* waiting_threads;

    /** @brief Semaphore counter */
    volatile int32_t sem_level;

    /** @brief Semaphore initialization state. */
    int32_t init;

#if MAX_CPU_COUNT > 1
    /** @brief Critical section spinlock. */
    spinlock_t lock;
#endif
};

/**
 * @brief Defines semaphore_t type as a shorcut for struct semaphore.
 */
typedef struct semaphore semaphore_t;

/*******************************************************************************
 * FUNCTIONS
 ******************************************************************************/

/**
 * @brief Initializes the semaphore structure.
 *
 * @details Initializes the semaphore structure. The initial state of a
 * semaphore is given by the init_level parameter.
 *
 * @param[out] sem The pointer to the semaphore to initialize.
 * @param[in] init_level The initial value to set the semaphore with.
 *
 * @return The success state or the error code.
 * - OS_NO_ERR is returned if no error is encountered.
 * - OS_ERR_NULL_POINTER is returned if the pointer to the semaphore to
 *   initialize is NULL.
 */
OS_RETURN_E sem_init(semaphore_t* sem, const int32_t init_level);

/**
 * @brief Destroys the semaphore given as parameter.
 *
 * @details Destroys the semaphore given as parameter. Also unlock all the
 * threads locked on this semaphore.
 *
 * @param[in, out] sem The semaphore to destroy.
 *
 * @return The success state or the error code.
 * - OS_NO_ERR is returned if no error is encountered.
 * - OS_ERR_NULL_POINTER is returned if the pointer to the semaphore to destroy
 *   is NULL.
 * - OS_ERR_SEM_UNINITIALIZED is returned if the semaphore has not been
 *   initialized.
 */
OS_RETURN_E sem_destroy(semaphore_t* sem);

/**
 * @brief Pends on the semaphore given as parameter.
 *
 * @details Pends on the semaphore given as parameter. The calling thread will
 * block on this call until the semaphore is aquired.
 *
 * @param[in] sem The semaphore to pend.
 *
 * @return The success state or the error code.
 * - OS_NO_ERR is returned if no error is encountered.
 * - OS_ERR_NULL_POINTER is returned if the pointer to the semaphore to destroy
 *   is NULL.
 * - OS_ERR_SEM_UNINITIALIZED is returned if the semaphore has not been
 *   initialized.
 */
OS_RETURN_E sem_pend(semaphore_t* sem);

/**
 * @brief Post the semaphore given as parameter.
 *
 * @param[in] sem The semaphore to post.
 *
 * @return The success state or the error code.
 * - OS_NO_ERR is returned if no error is encountered.
 * - OS_ERR_NULL_POINTER is returned if the pointer to the semaphore to destroy
 *   is NULL.
 * - OS_ERR_SEM_UNINITIALIZED is returned if the semaphore has not been
 *   initialized.
 */
OS_RETURN_E sem_post(semaphore_t* sem);

/**
 * @brief Try to pend on the semaphore given as parameter.
 *
 * @details Try to pend on the mutex semaphore as parameter. The function will
 * return the current state of the semaphore. If possible the function will
 * aquire the semaphore.
 *
 * @param[in] sem The semaphore to pend.
 * @param[out] value The buffer that receives the semaphore state.
 *
 * @return The success state or the error code.
 * - OS_SEM_LOCKED is returned if the semaphore is locked.
 * - OS_NO_ERR is returned if no error is encountered.
 * - OS_ERR_NULL_POINTER is returned if the pointer to the semaphore to destroy
 *   is NULL.
 * - OS_ERR_SEM_UNINITIALIZED is returned if the semaphore has not been
 *   initialized.
 */
OS_RETURN_E sem_try_pend(semaphore_t* sem, int32_t* value);

#endif /* #ifndef __SYNC_SEMAPHORE_H_ */