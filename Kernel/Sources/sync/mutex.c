/***************************************************************************//**
 * @file mutex.c
 *
 * @see mutex.h
 *
 * @author Alexy Torres Aurora Dugo
 *
 * @date 10/03/2018
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
#include <sync/mutex.h>

/*******************************************************************************
 * GLOBAL VARIABLES
 ******************************************************************************/

/*******************************************************************************
 * FUNCTIONS
 ******************************************************************************/

OS_RETURN_E mutex_init(mutex_t* mutex, const uint32_t flags,
                       const uint16_t priority)
{
    OS_RETURN_E err;

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

    mutex->state = 1;
    mutex->flags = flags | priority << 8;

#if MAX_CPU_COUNT > 1
    INIT_SPINLOCK(&mutex->lock);
#endif

    mutex->waiting_threads = kernel_queue_create_queue(&err);
    if(err != OS_NO_ERR)
    {
        return err;
    }

    mutex->init = 1;

#if MUTEX_KERNEL_DEBUG == 1
    kernel_serial_debug("Mutex 0x%p initialized\n", mutex);
#endif

    return OS_NO_ERR;
}

OS_RETURN_E mutex_destroy(mutex_t* mutex)
{
    kernel_queue_node_t* node;
    OS_RETURN_E          err;
    uint32_t             int_state;

    /* Check if mutex is initialized */
    if(mutex == NULL)
    {
        return OS_ERR_NULL_POINTER;
    }

#if MAX_CPU_COUNT > 1
    ENTER_CRITICAL(int_state, &mutex->lock);
#else
    ENTER_CRITICAL(int_state);
#endif

    if(mutex->init != 1)
    {
#if MAX_CPU_COUNT > 1
        EXIT_CRITICAL(int_state, &mutex->lock);
#else
        EXIT_CRITICAL(int_state);
#endif
        return OS_ERR_MUTEX_UNINITIALIZED;
    }

    /* Unlock all threads */
    while((node = kernel_queue_pop(mutex->waiting_threads, &err))
        != NULL)
    {
        if(err != OS_NO_ERR)
        {
#if MAX_CPU_COUNT > 1
            EXIT_CRITICAL(int_state, &mutex->lock);
#else
            EXIT_CRITICAL(int_state);
#endif
            kernel_error("Could not dequeue thread from mutex[%d]\n", err);
            kernel_panic(err);
        }

        err = sched_unlock_thread(node, THREAD_WAIT_TYPE_MUTEX, 0);
        if(err != OS_NO_ERR)
        {
#if MAX_CPU_COUNT > 1
            EXIT_CRITICAL(int_state, &mutex->lock);
#else
            EXIT_CRITICAL(int_state);
#endif
            kernel_error("Could not unlock thread from mutex[%d]\n", err);
            kernel_panic(err);
        }

#if MUTEX_KERNEL_DEBUG == 1
        kernel_serial_debug("Mutex 0x%p unlocked thead %d\n",
                            mutex,
                            ((kernel_thread_t*)node->data)->tid);
#endif
    }
    if(err != OS_NO_ERR)
    {
#if MAX_CPU_COUNT > 1
        EXIT_CRITICAL(int_state, &mutex->lock);
#else
        EXIT_CRITICAL(int_state);
#endif
        kernel_error("Could not dequeue thread from mutex[%d]\n", err);
        kernel_panic(err);
    }


    err = kernel_queue_delete_queue(&mutex->waiting_threads);
    mutex->init = 0;

#if MUTEX_KERNEL_DEBUG == 1
    kernel_serial_debug("Mutex 0x%p destroyed\n", mutex);
#endif

#if MAX_CPU_COUNT > 1
    EXIT_CRITICAL(int_state, &mutex->lock);
#else
    EXIT_CRITICAL(int_state);
#endif

    return OS_NO_ERR;
}

OS_RETURN_E mutex_pend(mutex_t* mutex)
{
    OS_RETURN_E err;
    uint32_t    prio;
    uint32_t    int_state;

    /* Check if mutex is initialized */
    if(mutex == NULL)
    {
        return OS_ERR_NULL_POINTER;
    }

#if MAX_CPU_COUNT > 1
    ENTER_CRITICAL(int_state, &mutex->lock);
#else
    ENTER_CRITICAL(int_state);
#endif

    if(mutex->init != 1)
    {
#if MAX_CPU_COUNT > 1
        EXIT_CRITICAL(int_state, &mutex->lock);
#else
        EXIT_CRITICAL(int_state);
#endif
        return OS_ERR_MUTEX_UNINITIALIZED;
    }

    /* Check if we can enter the critical section, also check if the mutex
     * has not been destroyed
     */
    while(mutex->init == 1 &&
          mutex->state != 1)
    {
        kernel_queue_node_t* active_thread;

        /* If the mutex is recursive and the thread acuired the mutex,
         * then don't block the thread
         */
        if((mutex->flags & MUTEX_FLAG_RECURSIVE) != 0 &&
           sched_get_tid() == mutex->locker_tid)
        {
            break;
        }

        active_thread = sched_lock_thread(THREAD_WAIT_TYPE_MUTEX);
        if(active_thread == NULL)
        {
#if MAX_CPU_COUNT > 1
            EXIT_CRITICAL(int_state, &mutex->lock);
#else
            EXIT_CRITICAL(int_state);
#endif
            kernel_error("Could not lock this thread to mutex[%d]\n",
                         OS_ERR_NULL_POINTER);
            kernel_panic(OS_ERR_NULL_POINTER);
        }

        err = kernel_queue_push(active_thread, mutex->waiting_threads);
        if(err != OS_NO_ERR)
        {
#if MAX_CPU_COUNT > 1
            EXIT_CRITICAL(int_state, &mutex->lock);
#else
            EXIT_CRITICAL(int_state);
#endif
            kernel_error("Could not enqueue thread to mutex[%d]\n", err);
            kernel_panic(err);
        }

#if MUTEX_KERNEL_DEBUG == 1
        kernel_serial_debug("Mutex 0x%p locked thead %d\n",
                            mutex,
                            ((kernel_thread_t*)active_thread->data)->tid);
#endif

#if MAX_CPU_COUNT > 1
        EXIT_CRITICAL(int_state, &mutex->lock);
#else
        EXIT_CRITICAL(int_state);
#endif

        sched_schedule();

#if MAX_CPU_COUNT > 1
        ENTER_CRITICAL(int_state, &mutex->lock);
#else
        ENTER_CRITICAL(int_state);
#endif
    }

    if(mutex->init != 1)
    {
#if MAX_CPU_COUNT > 1
        EXIT_CRITICAL(int_state, &mutex->lock);
#else
        EXIT_CRITICAL(int_state);
#endif
        return OS_ERR_MUTEX_UNINITIALIZED;
    }

    /* Set state to busy */
    mutex->state = 0;
    mutex->locker_tid = sched_get_tid();

    /* Set the inherited priority */
    prio = (mutex->flags >> 8) & MUTEX_PRIORITY_ELEVATION_NONE;
    if(prio != MUTEX_PRIORITY_ELEVATION_NONE)
    {
        mutex->acquired_thread_priority = sched_get_priority();

        err = sched_set_priority(prio >> 8);
        if(err != OS_NO_ERR)
        {
#if MAX_CPU_COUNT > 1
            EXIT_CRITICAL(int_state, &mutex->lock);
#else
            EXIT_CRITICAL(int_state);
#endif
            kernel_error("Could not elevate priority mutex[%d]\n", err);
            kernel_panic(err);
        }
    }
#if MUTEX_KERNEL_DEBUG == 1
    kernel_serial_debug("Mutex 0x%p aquired by thread %d\n",
                        mutex,
                        sched_get_tid());
#endif

#if MAX_CPU_COUNT > 1
    EXIT_CRITICAL(int_state, &mutex->lock);
#else
    EXIT_CRITICAL(int_state);
#endif

    return OS_NO_ERR;
}

OS_RETURN_E mutex_post(mutex_t* mutex)
{
    kernel_queue_node_t* node;
    OS_RETURN_E          err;
    uint32_t              do_sched;
    uint32_t             prio;
    uint32_t             int_state;

    /* Check if mutex is initialized */
    if(mutex == NULL)
    {
        return OS_ERR_NULL_POINTER;
    }

#if MAX_CPU_COUNT > 1
    ENTER_CRITICAL(int_state, &mutex->lock);
#else
    ENTER_CRITICAL(int_state);
#endif

    if(mutex->init != 1)
    {
#if MAX_CPU_COUNT > 1
        EXIT_CRITICAL(int_state, &mutex->lock);
#else
        EXIT_CRITICAL(int_state);
#endif
        return OS_ERR_MUTEX_UNINITIALIZED;
    }

    /* Increment mutex level */
    mutex->state = 1;

    /* Unset the inherited priority */
    do_sched = 0;
    prio = (mutex->flags >> 8) & MUTEX_PRIORITY_ELEVATION_NONE;
    if(prio != MUTEX_PRIORITY_ELEVATION_NONE)
    {
        err = sched_set_priority(mutex->acquired_thread_priority);
        if(err != OS_NO_ERR)
        {
#if MAX_CPU_COUNT > 1
            EXIT_CRITICAL(int_state, &mutex->lock);
#else
            EXIT_CRITICAL(int_state);
#endif
            kernel_error("Could release priority mmutex[%d]\n", err);
            kernel_panic(err);
        }
        mutex->acquired_thread_priority = KERNEL_LOWEST_PRIORITY;

        do_sched = 1;
    }

    /* Check if we can unlock a blocked thread on the mutex */
    if((node = kernel_queue_pop(mutex->waiting_threads, &err)) != NULL)
    {
        if(err != OS_NO_ERR)
        {
#if MAX_CPU_COUNT > 1
            EXIT_CRITICAL(int_state, &mutex->lock);
#else
            EXIT_CRITICAL(int_state);
#endif
            kernel_error("Could not dequeue thread from mutex[%d]\n", err);
            kernel_panic(err);
        }

#if MUTEX_KERNEL_DEBUG == 1
        kernel_serial_debug("Mutex 0x%p unlocked thead %d\n",
                            mutex,
                            ((kernel_thread_t*)node->data)->tid);
#endif

#if MAX_CPU_COUNT > 1
        EXIT_CRITICAL(int_state, &mutex->lock);
#else
        EXIT_CRITICAL(int_state);
#endif

        err = sched_unlock_thread(node, THREAD_WAIT_TYPE_MUTEX, 1);
        if(err != OS_NO_ERR)
        {
            kernel_error("Could not unlock thread from mutex[%d]\n", err);
            kernel_panic(err);
        }

#if MUTEX_KERNEL_DEBUG == 1
        kernel_serial_debug("Mutex 0x%p released by thead %d\n",
                            mutex,
                            sched_get_tid());
#endif

        return OS_NO_ERR;
    }
    if(err != OS_NO_ERR)
    {
#if MAX_CPU_COUNT > 1
        EXIT_CRITICAL(int_state, &mutex->lock);
#else
        EXIT_CRITICAL(int_state);
#endif
        kernel_error("Could not dequeue thread from mutex[%d]\n", err);
        kernel_panic(err);
    }

#if MUTEX_KERNEL_DEBUG == 1
    kernel_serial_debug("Mutex 0x%p released by thead %d\n",
                        mutex,
                        sched_get_tid());
#endif

    /* If here, we did not find any waiting process */
#if MAX_CPU_COUNT > 1
    EXIT_CRITICAL(int_state, &mutex->lock);
#else
    EXIT_CRITICAL(int_state);
#endif

    if(do_sched)
    {
        sched_schedule();
    }

    return OS_NO_ERR;
}

OS_RETURN_E mutex_try_pend(mutex_t* mutex, int32_t* value)
{
    uint32_t int_state;

    /* Check if mutex is initialized */
    if(mutex == NULL || value == NULL)
    {
        return OS_ERR_NULL_POINTER;
    }

#if MAX_CPU_COUNT > 1
    ENTER_CRITICAL(int_state, &mutex->lock);
#else
    ENTER_CRITICAL(int_state);
#endif

    if(mutex->init != 1)
    {
#if MAX_CPU_COUNT > 1
        EXIT_CRITICAL(int_state, &mutex->lock);
#else
        EXIT_CRITICAL(int_state);
#endif
        return OS_ERR_MUTEX_UNINITIALIZED;
    }

    /* Check if we can enter the critical section, also check if the mutex
     * has not been destroyed
     */
    if(mutex != NULL &&
       mutex->state != 1)
    {
        *value = mutex->state;

#if MUTEX_KERNEL_DEBUG == 1
        kernel_serial_debug("Locked mutex 0x%p try pend by thead %d\n",
                            mutex,
                            sched_get_tid());
#endif

#if MAX_CPU_COUNT > 1
        EXIT_CRITICAL(int_state, &mutex->lock);
#else
        EXIT_CRITICAL(int_state);
#endif
        return OS_MUTEX_LOCKED;
    }
    else if(mutex != NULL &&mutex->init == 1)
    {
        mutex->state = 0;
    }
    else
    {
#if MAX_CPU_COUNT > 1
        EXIT_CRITICAL(int_state, &mutex->lock);
#else
        EXIT_CRITICAL(int_state);
#endif
        return OS_ERR_MUTEX_UNINITIALIZED;
    }

#if MUTEX_KERNEL_DEBUG == 1
    kernel_serial_debug("Unlocked mutex 0x%p try pend and aquired by thead"
                        " %d\n",
                        mutex,
                        sched_get_tid());
#endif

#if MAX_CPU_COUNT > 1
    EXIT_CRITICAL(int_state, &mutex->lock);
#else
    EXIT_CRITICAL(int_state);
#endif

    return OS_NO_ERR;
}