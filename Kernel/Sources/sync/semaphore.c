/***************************************************************************//**
 * @file semaphore.c
 *
 * @see semaphore.h
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

#include <lib/stddef.h>        /* Standard definitions */
#include <lib/stdint.h>        /* Generic int types */
#include <lib/string.h>        /* String manipulation */
#include <core/kernel_queue.h> /* Kernel queues */
#include <core/scheduler.h>    /* Kernel scheduler */
#include <io/kernel_output.h>  /* Kernel output methods */
#include <core/panic.h>        /* Kernel panic */
#include <sync/critical.h>     /* Critical sections */

/* UTK configuration file */
#include <config.h>

/* Header file */
#include <sync/semaphore.h>

/*******************************************************************************
 * GLOBAL VARIABLES
 ******************************************************************************/

/*******************************************************************************
 * FUNCTIONS
 ******************************************************************************/

OS_RETURN_E sem_init(semaphore_t* sem, const int32_t init_level)
{
    OS_RETURN_E err;
    if(sem == NULL)
    {
        return OS_ERR_NULL_POINTER;
    }

    /* Init the semaphore*/
    memset(sem, 0, sizeof(semaphore_t));

    sem->sem_level = init_level;

#if MAX_CPU_COUNT > 1
    INIT_SPINLOCK(&sem->lock);
#endif

    sem->waiting_threads = kernel_queue_create_queue(&err);
    if(err != OS_NO_ERR)
    {
        return err;
    }

    sem->init = 1;

#if SEMAPHORE_KERNEL_DEBUG == 1
    kernel_serial_debug("Semaphore 0x%p initialized\n", sem);
#endif

    return OS_NO_ERR;
}

OS_RETURN_E sem_destroy(semaphore_t* sem)
{
    kernel_queue_node_t* node;
    OS_RETURN_E          err;
    uint32_t             int_state;

    /* Check if semaphore is initialized */
    if(sem == NULL)
    {
        return OS_ERR_NULL_POINTER;
    }

#if MAX_CPU_COUNT > 1
    ENTER_CRITICAL(int_state, &sem->lock);
#else
    ENTER_CRITICAL(int_state);
#endif

    if(sem->init != 1)
    {
#if MAX_CPU_COUNT > 1
        EXIT_CRITICAL(int_state, &sem->lock);
#else
        EXIT_CRITICAL(int_state);
#endif
        return OS_ERR_SEM_UNINITIALIZED;
    }

    sem->init = 0;

    /* Unlock all threead*/
    while((node = kernel_queue_pop(sem->waiting_threads, &err)) != NULL)
    {
        if(err != OS_NO_ERR)
        {
#if MAX_CPU_COUNT > 1
            EXIT_CRITICAL(int_state, &sem->lock);
#else
            EXIT_CRITICAL(int_state);
#endif
            kernel_error("Could not dequeue thread from semaphore[%d]\n", err);
            kernel_panic(err);
        }
        err = sched_unlock_thread(node, THREAD_WAIT_TYPE_SEM, 0);
        if(err != OS_NO_ERR)
        {
#if MAX_CPU_COUNT > 1
            EXIT_CRITICAL(int_state, &sem->lock);
#else
            EXIT_CRITICAL(int_state);
#endif
            kernel_error("Could not unlock thread from semaphore[%d]\n", err);
            kernel_panic(err);
        }

#if SEMAPHORE_KERNEL_DEBUG == 1
        kernel_serial_debug("Semaphore 0x%p unlocked thead %d\n",
                            sem,
                            ((kernel_thread_t*)node->data)->tid);
#endif
    }
    if(err != OS_NO_ERR)
    {
#if MAX_CPU_COUNT > 1
        EXIT_CRITICAL(int_state, &sem->lock);
#else
        EXIT_CRITICAL(int_state);
#endif
        kernel_error("Could not dequeue thread from semaphore[%d]\n", err);
        kernel_panic(err);
    }

#if SEMAPHORE_KERNEL_DEBUG == 1

    kernel_serial_debug("Semaphore 0x%p destroyed\n", sem);
#endif

#if MAX_CPU_COUNT > 1
    EXIT_CRITICAL(int_state, &sem->lock);
#else
    EXIT_CRITICAL(int_state);
#endif

    return OS_NO_ERR;
}

OS_RETURN_E sem_pend(semaphore_t* sem)
{
    OS_RETURN_E err;
    uint32_t    int_state;

    /* Check if semaphore is initialized */
    if(sem == NULL)
    {
        return OS_ERR_NULL_POINTER;
    }

#if MAX_CPU_COUNT > 1
    ENTER_CRITICAL(int_state, &sem->lock);
#else
    ENTER_CRITICAL(int_state);
#endif

    if(sem->init != 1)
    {
#if MAX_CPU_COUNT > 1
        EXIT_CRITICAL(int_state, &sem->lock);
#else
        EXIT_CRITICAL(int_state);
#endif
        return OS_ERR_SEM_UNINITIALIZED;
    }

    /* Check if we can enter the critical section, also check if the semaphore
     * has not been destroyed
     */
    while(sem->init == 1 &&
          sem->sem_level < 1)
    {
        kernel_queue_node_t* active_thread = sched_lock_thread(THREAD_WAIT_TYPE_SEM);

        if(active_thread == NULL)
        {
#if MAX_CPU_COUNT > 1
            EXIT_CRITICAL(int_state, &sem->lock);
#else
            EXIT_CRITICAL(int_state);
#endif
            kernel_error("Could not lock this thread to semaphore[%d]\n",
                         OS_ERR_NULL_POINTER);
            kernel_panic(OS_ERR_NULL_POINTER);
        }

        err = kernel_queue_push(active_thread, sem->waiting_threads);

        if(err != OS_NO_ERR)
        {
#if MAX_CPU_COUNT > 1
            EXIT_CRITICAL(int_state, &sem->lock);
#else
            EXIT_CRITICAL(int_state);
#endif
            kernel_error("Could not enqueue thread from semaphore[%d]\n", err);
            kernel_panic(err);
        }

#if SEMAPHORE_KERNEL_DEBUG == 1
        kernel_serial_debug("Semaphore 0x%p locked thead %d\n",
                            sem,
                            ((kernel_thread_t*)active_thread->data)->tid);
#endif

#if MAX_CPU_COUNT > 1
        EXIT_CRITICAL(int_state, &sem->lock);
#else
        EXIT_CRITICAL(int_state);
#endif

        sched_schedule();

#if MAX_CPU_COUNT > 1
        ENTER_CRITICAL(int_state, &sem->lock);
#else
        ENTER_CRITICAL(int_state);
#endif
    }

    if(sem->init != 1)
    {
#if MAX_CPU_COUNT > 1
        EXIT_CRITICAL(int_state, &sem->lock);
#else
        EXIT_CRITICAL(int_state);
#endif
        return OS_ERR_SEM_UNINITIALIZED;
    }

    /* Decrement sem level */
    --(sem->sem_level);

#if SEMAPHORE_KERNEL_DEBUG == 1
    kernel_serial_debug("Semaphore 0x%p aquired by thead %d\n",
                        sem,
                        sched_get_tid());
#endif

#if MAX_CPU_COUNT > 1
    EXIT_CRITICAL(int_state, &sem->lock);
#else
    EXIT_CRITICAL(int_state);
#endif

    return OS_NO_ERR;
}

OS_RETURN_E sem_post(semaphore_t* sem)
{
    OS_RETURN_E err;
    uint32_t    int_state;

    /* Check if semaphore is initialized */
    if(sem == NULL)
    {
        return OS_ERR_NULL_POINTER;
    }

#if MAX_CPU_COUNT > 1
    ENTER_CRITICAL(int_state, &sem->lock);
#else
    ENTER_CRITICAL(int_state);
#endif

    if(sem->init != 1)
    {
#if MAX_CPU_COUNT > 1
        EXIT_CRITICAL(int_state, &sem->lock);
#else
        EXIT_CRITICAL(int_state);
#endif
        return OS_ERR_SEM_UNINITIALIZED;
    }

    /* Increment sem level */
    ++sem->sem_level;

    /* Check if we can unlock a blocked thread on the semaphore */
    if(sem->sem_level > 0)
    {
        kernel_queue_node_t* node;

        if((node = kernel_queue_pop(sem->waiting_threads, &err))
            != NULL)
        {
            if(err != OS_NO_ERR)
            {
#if MAX_CPU_COUNT > 1
                EXIT_CRITICAL(int_state, &sem->lock);
#else
                EXIT_CRITICAL(int_state);
#endif
                kernel_error("Could not dequeue thread from semaphore[%d]\n",
                             err);
                kernel_panic(err);
            }

#if SEMAPHORE_KERNEL_DEBUG == 1
            kernel_serial_debug("Semaphore 0x%p unlocked thead %d\n",
                                sem,
                                ((kernel_thread_t*)node->data)->tid);
#endif

#if MAX_CPU_COUNT > 1
            EXIT_CRITICAL(int_state, &sem->lock);
#else
            EXIT_CRITICAL(int_state);
#endif

            /* Do not schedule in interrupt handlers */
            if(kernel_interrupt_get_state() > 0)
            {
                err = sched_unlock_thread(node, THREAD_WAIT_TYPE_SEM, 0);
            }
            else
            {
                err = sched_unlock_thread(node, THREAD_WAIT_TYPE_SEM, 1);
            }

            if(err != OS_NO_ERR)
            {
#if MAX_CPU_COUNT > 1
                EXIT_CRITICAL(int_state, &sem->lock);
#else
                EXIT_CRITICAL(int_state);
#endif
                kernel_error("Could not unlock thread from semaphore[%d]\n",
                             err);
                kernel_panic(err);
            }

#if SEMAPHORE_KERNEL_DEBUG == 1
            kernel_serial_debug("Semaphore 0x%p released by thead %d\n",
                                sem,
                                sched_get_tid());
#endif

            return OS_NO_ERR;
        }
        if(err != OS_NO_ERR)
        {
#if MAX_CPU_COUNT > 1
            EXIT_CRITICAL(int_state, &sem->lock);
#else
            EXIT_CRITICAL(int_state);
#endif
            kernel_error("Could not dequeue thread from semaphore[%d]\n", err);
            kernel_panic(err);
        }
    }

#if SEMAPHORE_KERNEL_DEBUG == 1
    kernel_serial_debug("Semaphore 0x%p released by thead %d\n",
                        sem,
                        sched_get_tid());
#endif

    /* If here, we did not find any waiting process */
#if MAX_CPU_COUNT > 1
    EXIT_CRITICAL(int_state, &sem->lock);
#else
    EXIT_CRITICAL(int_state);
#endif

    return OS_NO_ERR;
}

OS_RETURN_E sem_try_pend(semaphore_t* sem, int32_t* value)
{
    uint32_t int_state;

    /* Check if semaphore is initialized */
    if(sem == NULL || value == NULL)
    {
        return OS_ERR_NULL_POINTER;
    }

#if MAX_CPU_COUNT > 1
    ENTER_CRITICAL(int_state, &sem->lock);
#else
    ENTER_CRITICAL(int_state);
#endif

    if(sem->init != 1)
    {
#if MAX_CPU_COUNT > 1
        EXIT_CRITICAL(int_state, &sem->lock);
#else
        EXIT_CRITICAL(int_state);
#endif
        return OS_ERR_SEM_UNINITIALIZED;
    }

    /* Check if we can enter the critical section, also check if the semaphore
     * has not been destroyed
     */
    if(sem != NULL &&
       sem->sem_level < 1)
    {
        *value = sem->sem_level;

#if SEMAPHORE_KERNEL_DEBUG == 1
        kernel_serial_debug("Locked semaphore 0x%p try pend by thead %d\n",
                            sem,
                            sched_get_tid());
#endif

#if MAX_CPU_COUNT > 1
        EXIT_CRITICAL(int_state, &sem->lock);
#else
        EXIT_CRITICAL(int_state);
#endif
        return OS_SEM_LOCKED;
    }
    else if(sem != NULL && sem->init == 1)
    {
        *value = --sem->sem_level;
    }
    else
    {
#if MAX_CPU_COUNT > 1
        EXIT_CRITICAL(int_state, &sem->lock);
#else
        EXIT_CRITICAL(int_state);
#endif
        return OS_ERR_SEM_UNINITIALIZED;
    }

#if SEMAPHORE_KERNEL_DEBUG == 1
    kernel_serial_debug("Unlocked semaphore 0x%p try pend and aquired by "
                        "thead %d\n",
                        sem,
                        sched_get_tid());
#endif

#if MAX_CPU_COUNT > 1
    EXIT_CRITICAL(int_state, &sem->lock);
#else
    EXIT_CRITICAL(int_state);
#endif

    return OS_NO_ERR;
}