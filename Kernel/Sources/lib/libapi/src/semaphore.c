/*******************************************************************************
 * @file semaphore.c
 *
 * @see semaphore.h
 *
 * @author Alexy Torres Aurora Dugo
 *
 * @date 15/12/2021
 *
 * @version 3.0
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

/*******************************************************************************
 * INCLUDES
 ******************************************************************************/

/* Included headers */
#include <scheduler.h>       /* Scheduler constants */
#include <atomic.h>          /* Spinlock API */
#include <kernel_output.h>   /* Kernel outputs */
#include <string.h>          /* Standard memory lib */
#include <kernel_error.h>    /* Kernel errors */
#include <futex.h>           /* Futex API */
#include <sys/process.h>     /* Process and threads management API */
#include <sys/syscall_api.h> /* System calls API */

/* Configuration files */
#include <config.h>
#include <test_bank.h>

/* Header file */
#include <semaphore.h>

/*******************************************************************************
 * CONSTANTS
 ******************************************************************************/

/* None */

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

/* None */

/*******************************************************************************
 * FUNCTIONS
 ******************************************************************************/

OS_RETURN_E sem_init(semaphore_t* sem, const int32_t init_level)
{
    if(sem == NULL)
    {
        return OS_ERR_NULL_POINTER;
    }

    /* Init the semaphore*/
    sem->level   = init_level;
    sem->waiters = 0;
    sem->lock    = SPINLOCK_INIT_VALUE;
    sem->init    = TRUE;

    KERNEL_DEBUG(SEMAPHORE_DEBUG_ENABLED, "Semaphore 0x%p initialized.", sem);

    return OS_NO_ERR;
}

OS_RETURN_E sem_destroy(semaphore_t* sem)
{
    futex_t futex;
    int32_t waiters;

    /* Check if semaphore is initialized */
    if(sem == NULL)
    {
        return OS_ERR_NULL_POINTER;
    }
    if(sem->init == FALSE)
    {
        return OS_ERR_NOT_INITIALIZED;
    }

    SPINLOCK_LOCK(sem->lock);

    /* Destroy the mutex */
    sem->init = FALSE;

    /* Wakeup all threads locked on the semaphore */
    if((waiters = sem->waiters) > 0)
    {
        sem->level = sem->waiters;
        SPINLOCK_UNLOCK(sem->lock);
        futex.val  = waiters;
        futex.addr = (uint32_t*)&sem->level;
        syscall_do(SYSCALL_FUTEX_WAKE, &futex);
        if(futex.error != OS_NO_ERR)
        {
            return futex.error;
        }
    }
    else
    {
        SPINLOCK_UNLOCK(sem->lock);
    }

    KERNEL_DEBUG(SEMAPHORE_DEBUG_ENABLED, "Semaphore 0x%p destroyed.", sem);

    return OS_NO_ERR;
}

OS_RETURN_E sem_pend(semaphore_t* sem)
{
    futex_t     futex;

    /* Check if semaphore is initialized */
    if(sem == NULL)
    {
        return OS_ERR_NULL_POINTER;
    }
    if(sem->init == FALSE)
    {
        return OS_ERR_NOT_INITIALIZED;
    }

    SPINLOCK_LOCK(sem->lock);

    /* Wait to get the semaphore */
    while(sem->level <= 0)
    {
        ++sem->waiters;

        /* Wait on futex until the semaphore is opened */
        futex.addr = (uint32_t*)&sem->level;
        futex.val  = sem->level;

        SPINLOCK_UNLOCK(sem->lock);

        syscall_do(SYSCALL_FUTEX_WAIT, &futex);

        SPINLOCK_LOCK(sem->lock);

        /* We are not waiting anymore */
        --sem->waiters;

        if(futex.error != OS_NO_ERR)
        {
            SPINLOCK_UNLOCK(sem->lock);
            return futex.error;
        }

        /* Check if the semaphore is still initialized */
        if(sem->init == FALSE)
        {
            SPINLOCK_UNLOCK(sem->lock);

            return OS_ERR_NOT_INITIALIZED;
        }
    }

    /* We acquired the semaphore */
    --sem->level;

    SPINLOCK_UNLOCK(sem->lock);

    KERNEL_DEBUG(SEMAPHORE_DEBUG_ENABLED, "Semaphore 0x%p acquired.", sem);

    return OS_NO_ERR;
}

OS_RETURN_E sem_post(semaphore_t* sem)
{
    futex_t futex;

    /* Check if semaphore is initialized */
    if(sem == NULL)
    {
        return OS_ERR_NULL_POINTER;
    }
    if(sem->init == FALSE)
    {
        return OS_ERR_NOT_INITIALIZED;
    }

    SPINLOCK_LOCK(sem->lock);

    /* We released the semaphore */
    ++sem->level;

    /* Check if we should wakeup threads */
    if(sem->level > 0 && sem->waiters > 0)
    {
        futex.addr = (uint32_t*)&sem->level;
        futex.val  = 1;
        syscall_do(SYSCALL_FUTEX_WAKE, &futex);

        if(futex.error != OS_NO_ERR)
        {
            SPINLOCK_UNLOCK(sem->lock);
            return futex.error;
        }
    }

    SPINLOCK_UNLOCK(sem->lock);

    KERNEL_DEBUG(SEMAPHORE_DEBUG_ENABLED, "Semaphore 0x%p released.", sem);

    return OS_NO_ERR;
}

OS_RETURN_E sem_trypend(semaphore_t* sem, int32_t* value)
{
    /* Check if semaphore is initialized */
    if(sem == NULL)
    {
        return OS_ERR_NULL_POINTER;
    }
    if(sem->init == FALSE)
    {
        return OS_ERR_NOT_INITIALIZED;
    }

    SPINLOCK_LOCK(sem->lock);

    /* Check to get the semaphore */
    if(sem->level <= 0)
    {
        if(value != NULL)
        {
            *value = sem->level;
        }
        SPINLOCK_UNLOCK(sem->lock);
        return OS_ERR_UNAUTHORIZED_ACTION;
    }

    /* We acquired the semaphore */
    --sem->level;

    if(value != NULL)
    {
        *value = sem->level;
    }

    SPINLOCK_UNLOCK(sem->lock);

    KERNEL_DEBUG(SEMAPHORE_DEBUG_ENABLED, "Semaphore 0x%p acquired.", sem);

    return OS_NO_ERR;
}

/************************************ EOF *************************************/