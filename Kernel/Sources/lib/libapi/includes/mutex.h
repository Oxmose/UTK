/*******************************************************************************
 * @file mutex.h
 *
 * @see mutex.c
 *
 * @author Alexy Torres Aurora Dugo
 *
 * @date 05/12/2021
 *
 * @version 3.0
 *
 * @brief Mutex synchronization primitive.
 *
 * @details Mutex synchronization primitive implementation. Avoids priority
 * inversion by allowing the user to set a priority to the mutex, then all
 * threads that acquire this mutex will see their priority elevated to the
 * mutex's priority level.
 * The mutex  waiting list is a FIFO with no regard to the waiting threads
 * priority.
 *
 * @warning Mutex can only be used when the current system is running and the
 * scheduler initialized.
 *
 * @copyright Alexy Torres Aurora Dugo
 ******************************************************************************/

#ifndef __LIB_MUTEX_H_
#define __LIB_MUTEX_H_

/*******************************************************************************
 * INCLUDES
 ******************************************************************************/

#include <stddef.h>       /* Standard definitions */
#include <stdint.h>       /* Generic int types */
#include <kernel_error.h> /* Kernel error API */

/*******************************************************************************
 * CONSTANTS
 ******************************************************************************/

/** @brief Mutex flags: emtpy flag. */
#define MUTEX_FLAG_NONE               0x00000000
/** @brief Mutex flags: recursive capable mutex flag. */
#define MUTEX_FLAG_RECURSIVE          0x00000001
/** @brief Mutex flags: priority elevation disabled flag. */
#define MUTEX_PRIORITY_ELEVATION_NONE 0x0000FFFF

/*******************************************************************************
 * STRUCTURES
 ******************************************************************************/

/** @brief Mutex structure definition. */
typedef struct
{
    /**
     * @brief Mutex lock state
     */
    volatile int32_t state;

    /**
     * @brief Mutex flags.
     * @details The flags are defined bitwise;
     *  - [0]    = Recursive mutex.
     *  - [1-7]  = Unused (for future use).
     *  - [8-24] =  Mutex's priority.
     */
    uint32_t flags;

    /** @brief Priority of the thread that aquired the mutex. */
    uint32_t acquired_thread_priority;

    /** @brief TID of the thread that acquired the lock. */
    int32_t locker_tid;

    /** @brief TID of the owner thread */
    int32_t owner;
} mutex_t;

/*******************************************************************************
 * MACROS
 ******************************************************************************/

/* None */

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
 * @brief Initializes the mutex structure.
 *
 * @details Initiallizes the mutex structure. The mutex state is define at its
 * initial state: mutex unlocked.
 *
 * @param[out] mutex The pointer to the mutex to initialize.
 * @param[in] flags Mutex flags, see defines to get all the possible mutex
 * flags.
 * @param[in] priority The priority of the mutex, this is the priority the
 * thread that acquired the mutex will inherit. MUTEX_PRIORITY_ELEVATION_NONE
 * means not priority elevation on aquirance.
 *
 * @return The success state or the error code.
 * - OS_NO_ERR is returned if no error is encountered.
 * - OS_ERR_FORBIDEN_PRIORITY is returned if the desired priority is not
 *   allowed.
 * - OS_ERR_NULL_POINTER is returned if the pointer to the mutex to
 *   initialize is NULL.
 */
OS_RETURN_E mutex_init(mutex_t* mutex,
                       const uint32_t flags,
                       const uint16_t priority);

/**
 * @brief Detroys the mutex.
 *
 * @details Destroys the mutex given as parameter. The function will also unlock
 * all the threads locked on this mutex.
 *
 * @param[in, out] mutex The mutex to destroy.
 *
 * @return The success state or the error code.
 * - OS_NO_ERR is returned if no error is encountered.
 * - OS_ERR_NULL_POINTER is returned if the pointer to the mutex to destroy is
 *   NULL.
 * - OS_ERR_MUTEX_UNINITIALIZED is returned if the mutex has not been
 *   initialized.
 */
OS_RETURN_E mutex_destroy(mutex_t* mutex);

/**
 * @brief Lock on the mutex given as parameter.
 *
 * @details Lock on the mutex given as parameter. The function will block the
 * thread until it can aquire the mutex.
 *
 * @param[in] mutex The mutex to lock on.
 *
 * @return The success state or the error code.
 * - OS_NO_ERR is returned if no error is encountered.
 * - OS_ERR_NULL_POINTER is returned if the pointer to the mutex is NULL.
 * - OS_ERR_MUTEX_UNINITIALIZED is returned if the mutex has not been
 *   initialized.
 */
OS_RETURN_E mutex_lock(mutex_t* mutex);

/**
 * @brief Unlocks the mutex given as parameter.
 *
 * @details Unlocks the mutex given as parameter. The function might schedule to a
 * nest prioritary thread.
 *
 * @param[in] mutex The mutex to unlock.
 *
 * @return The success state or the error code.
 * - OS_NO_ERR is returned if no error is encountered.
 * - OS_ERR_NULL_POINTER is returned if the pointer to the mutex is NULL.
 * - OS_ERR_MUTEX_UNINITIALIZED is returned if the mutex has not been
 *   initialized.
 */
OS_RETURN_E mutex_unlock(mutex_t* mutex);

/**
 * @brief Try to lock on the mutex given as parameter.
 *
 * @details Try to lock on the mutex given as parameter. The function will
 * return the current state of the mutex. The state of an unlocked mutex is 0.
 *
 * @param[in] mutex The mutex to lock on.
 * @param[out] value The buffer that receive the mutex state.
 *
 * @return The success state or the error code.
 * - OS_NO_ERR is returned if no error is encountered.
 * - OS_ERR_NULL_POINTER is returned if the pointer to the mutex is NULL.
 * - OS_ERR_MUTEX_UNINITIALIZED is returned if the mutex has not been
 *   initialized.
 */
OS_RETURN_E mutex_trylock(mutex_t* mutex, int32_t* value);

#endif /* #ifndef __LIB_MUTEX_H_ */

/* EOF */