/***************************************************************************//**
 * @file mutex.c
 *
 * @see mutex.h
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

#include <scheduler.h>       /* Scheduler constants */
#include <atomic.h>          /* Spinlock API */
#include <kernel_output.h>   /* Kernel outputs */
#include <string.h>          /* Standard memory lib */
#include <kernel_error.h>    /* Kernel errors */
#include <sys/process.h>     /* Process and threads management API */
#include <sys/syscall_api.h> /* System calls API */

/* UTK configuration file */
#include <config.h>

/* Header file */
#include <mutex.h>

/*******************************************************************************
 * CONSTANTS
 ******************************************************************************/

/** @brief Mutex state unlocked. */
#define MUTEX_STATE_UNLOCKED    0
/** @brief Mutex state locked. */
#define MUTEX_STATE_LOCKED      1
/** @brief Mutex state locked with waiting threads. */
#define MUTEX_STATE_LOCKED_WAIT 2
/** @brief Mutex state waiting to be initialized. */
#define MUTEX_STATE_WAIT_INIT   3
/** @brief Mutex state destroyed. */
#define MUTEX_STATE_DESTROYED   4

/** @brief Defines the maximal number of threads locked on the mutex. */
#define MUTEX_MAX_LOCKED_THREAD 0xFFFFFFFF

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

OS_RETURN_E mutex_init(mutex_t* mutex,
                       const uint32_t flags,
                       const uint16_t priority)
{
    sched_param_t sched_params;

    if(mutex == NULL)
    {
        return OS_ERR_NULL_POINTER;
    }

    /* Check priority integrity */
    if(priority > KERNEL_LOWEST_PRIORITY &&
       priority != MUTEX_PRIORITY_ELEVATION_NONE)
    {
        return OS_ERR_FORBIDEN_PRIORITY;
    }

    /* Init the mutex*/
    memset(mutex, 0, sizeof(mutex_t));

    mutex->flags      = flags | priority << 8;

    syscall_do(SYSCALL_SCHED_GET_PARAMS, &sched_params);
    if(sched_params.error != OS_NO_ERR)
    {
        return sched_params.error;
    }

    mutex->owner = sched_params.tid;
    mutex->state = MUTEX_STATE_UNLOCKED;

    KERNEL_DEBUG(MUTEX_DEBUG_ENABLED, "Mutex 0x%p initialized", mutex);

    return OS_NO_ERR;
}

OS_RETURN_E mutex_destroy(mutex_t* mutex)
{
    futex_t futex;

    if(mutex == NULL)
    {
        return OS_ERR_NULL_POINTER;
    }
    /* Wakeup all threads locked on the mutex */
    futex.addr   = (uint32_t*)&mutex->state;
    futex.val    = MUTEX_MAX_LOCKED_THREAD;

    mutex->owner = -1;

    ATOMIC_STORE(&mutex->state, MUTEX_STATE_DESTROYED);
    syscall_do(SYSCALL_FUTEX_WAKE, &futex);

    KERNEL_DEBUG(MUTEX_DEBUG_ENABLED, "Mutex 0x%p destroyed", mutex);

    return futex.error;
}

OS_RETURN_E mutex_lock(mutex_t* mutex)
{
    OS_RETURN_E   err;
    uint32_t      prio;
    uint32_t      recursive;
    int32_t       mutex_state;
    sched_param_t sched_params;
    futex_t       futex;

    /* Check if mutex is initialized */
    if(mutex == NULL)
    {
        return OS_ERR_NULL_POINTER;
    }

    /* Check if we can enter the critical section, also check if the mutex
     * has not been destroyed
     */
    if(mutex->state != MUTEX_STATE_UNLOCKED &&
       mutex->state != MUTEX_STATE_LOCKED   &&
       mutex->state != MUTEX_STATE_LOCKED_WAIT)
    {
        return OS_ERR_NOT_INITIALIZED;
    }

    /* Prepare data in case of specific parameters */
    prio = (mutex->flags >> 8) & MUTEX_PRIORITY_ELEVATION_NONE;
    recursive = mutex->flags & MUTEX_FLAG_RECURSIVE;
    if(prio != MUTEX_PRIORITY_ELEVATION_NONE || recursive != 0)
    {
        syscall_do(SYSCALL_SCHED_GET_PARAMS, &sched_params);

        if(sched_params.error != OS_NO_ERR)
        {
            return sched_params.error;
        }
    }

    /* If the current thread is the mutex's locked, just return */
    if(recursive != 0 && sched_params.tid == mutex->locker_tid)
    {
        return OS_NO_ERR;
    }

    /* Get mutex state */
    mutex_state = ATOMIC_CAS(&mutex->state,
                             MUTEX_STATE_UNLOCKED,
                             MUTEX_STATE_LOCKED);
    if(mutex_state != MUTEX_STATE_UNLOCKED)
    {
        do
        {
            /* Last check, if some process were already waitign or
             * if the mutex unlocked, then try to lock it again in
             * the next check, oterwise, we are still locked and need to wait.
             */
            if(mutex_state == MUTEX_STATE_LOCKED_WAIT ||
               ATOMIC_CAS(&mutex->state,
                          MUTEX_STATE_LOCKED,
                          MUTEX_STATE_LOCKED_WAIT) != MUTEX_STATE_UNLOCKED)
            {
                futex.addr = (uint32_t*)&mutex->state;
                futex.val  = MUTEX_STATE_LOCKED_WAIT;
                syscall_do(SYSCALL_FUTEX_WAIT, &futex);
                if(futex.error != OS_NO_ERR)
                {
                    return futex.error;
                }
            }

            /* We were woken up, check the new state of the mutex */
            mutex_state = ATOMIC_CAS(&mutex->state,
                                     MUTEX_STATE_UNLOCKED,
                                     MUTEX_STATE_LOCKED_WAIT);

            /* Sanity check */
            if(mutex_state != MUTEX_STATE_UNLOCKED &&
               mutex_state != MUTEX_STATE_LOCKED   &&
               mutex_state != MUTEX_STATE_LOCKED_WAIT)
            {
                return OS_ERR_NOT_INITIALIZED;
            }
        }while(mutex_state != MUTEX_STATE_UNLOCKED);
    }

    /* We aquired the mutex, set the relevant information */
    if(recursive != 0)
    {
        mutex->locker_tid = sched_params.tid;
    }

    /* Set the inherited priority */
    if(prio != MUTEX_PRIORITY_ELEVATION_NONE)
    {
        mutex->acquired_thread_priority = sched_params.priority;

        sched_params.priority = prio >> 8;
        syscall_do(SYSCALL_SCHED_SET_PARAMS, &sched_params);

        if(sched_params.error != OS_NO_ERR)
        {
            /* Unlock the mutex and return error */
            err = mutex_unlock(mutex);
            if(err != OS_NO_ERR)
            {
                /* If the had an error unlocking the mutex, kill the process */
                syscall_do(SYSCALL_EXIT, (void*)err);
            }
            return err;
        }
    }

    KERNEL_DEBUG(MUTEX_DEBUG_ENABLED, "Mutex 0x%p aquired", mutex);

    return OS_NO_ERR;
}

OS_RETURN_E mutex_unlock(mutex_t* mutex)
{
    uint32_t      prio;
    uint32_t      mutex_state;
    sched_param_t sched_params;
    futex_t       futex;

    /* Check if mutex is initialized */
    if(mutex == NULL)
    {
        return OS_ERR_NULL_POINTER;
    }

    /* Set back old data */
    prio = (mutex->flags >> 8) & MUTEX_PRIORITY_ELEVATION_NONE;
    if(prio != MUTEX_PRIORITY_ELEVATION_NONE)
    {
        /* Update the thread priority */
        syscall_do(SYSCALL_SCHED_GET_PARAMS, &sched_params);
        if(sched_params.error != OS_NO_ERR)
        {
            return sched_params.error;
        }

        sched_params.priority = mutex->acquired_thread_priority;

        syscall_do(SYSCALL_SCHED_SET_PARAMS, &sched_params);
        if(sched_params.error != OS_NO_ERR)
        {
            return sched_params.error;
        }
    }
    mutex->locker_tid = -1;

    /* Get mutex state */
    mutex_state = ATOMIC_FETCH_ADD(&mutex->state, -1);

    /* If other threads wait for the mutex */
    if(mutex_state != MUTEX_STATE_LOCKED)
    {
        ATOMIC_STORE(&mutex->state, MUTEX_STATE_UNLOCKED);        /* Wake only one thread */
        futex.addr = (uint32_t*)&mutex->state;
        futex.val  = 1;
        syscall_do(SYSCALL_FUTEX_WAKE, &futex);
        if(futex.error != OS_NO_ERR)
        {
            return futex.error;
        }
    }
    else if(mutex_state != MUTEX_STATE_LOCKED &&
            mutex_state != MUTEX_STATE_UNLOCKED &&
            mutex_state != MUTEX_STATE_LOCKED_WAIT)
    {
        /* Here the mutex was in another state, which is undefined behavior */
        return OS_ERR_NOT_INITIALIZED;
    }

    KERNEL_DEBUG(MUTEX_DEBUG_ENABLED, "Mutex 0x%p released", mutex);

    return OS_NO_ERR;
}

OS_RETURN_E mutex_trylock(mutex_t* mutex, int32_t* value)
{
    OS_RETURN_E err;
    uint32_t    prio;
    uint32_t    recursive;
    int32_t     mutex_state;

    sched_param_t sched_params;

    /* Check if mutex is initialized */
    if(mutex == NULL || value == NULL)
    {
        return OS_ERR_NULL_POINTER;
    }

    /* Check if we can enter the critical section, also check if the mutex
     * has not been destroyed
     */
    if(mutex->state != MUTEX_STATE_UNLOCKED &&
       mutex->state != MUTEX_STATE_LOCKED   &&
       mutex->state != MUTEX_STATE_LOCKED_WAIT)
    {
        return OS_ERR_NOT_INITIALIZED;
    }


    /* Prepare data in case of specific parameters */
    prio = (mutex->flags >> 8) & MUTEX_PRIORITY_ELEVATION_NONE;
    recursive = mutex->flags & MUTEX_FLAG_RECURSIVE;
    if(prio != MUTEX_PRIORITY_ELEVATION_NONE || recursive != 0)
    {
        syscall_do(SYSCALL_SCHED_GET_PARAMS, &sched_params);

        if(sched_params.error != OS_NO_ERR)
        {
            return sched_params.error;
        }
    }

    /* If the current thread is the mutex's locked, just return */
    if(recursive != 0 && sched_params.tid == mutex->locker_tid)
    {
        return OS_NO_ERR;
    }

    /* Get mutex state */
    mutex_state = ATOMIC_CAS(&mutex->state,
                             MUTEX_STATE_UNLOCKED,
                             MUTEX_STATE_LOCKED);

    *value = mutex_state;
    if(mutex_state != MUTEX_STATE_UNLOCKED)
    {
        return OS_ERR_UNAUTHORIZED_ACTION;
    }

    /* We aquired the mutex, set the relevant information */
    if(recursive != 0)
    {
        mutex->locker_tid = sched_params.tid;
    }

    /* Set the inherited priority */
    if(prio != MUTEX_PRIORITY_ELEVATION_NONE)
    {
        mutex->acquired_thread_priority = sched_params.priority;

        sched_params.priority = prio >> 8;
        syscall_do(SYSCALL_SCHED_SET_PARAMS, &sched_params);

        if(sched_params.error != OS_NO_ERR)
        {
            /* Unlock the mutex and return error */
            err = mutex_unlock(mutex);
            if(err != OS_NO_ERR)
            {
                /* If the had an error unlocking the mutex, kill the process */
                syscall_do(SYSCALL_EXIT, (void*)err);
            }
            return err;
        }
    }

    KERNEL_DEBUG(MUTEX_DEBUG_ENABLED, "Mutex 0x%p aquired", mutex);

    return OS_NO_ERR;
}