/*******************************************************************************
 * @file futex.c
 *
 * @see futex.h
 *
 * @author Alexy Torres Aurora Dugo
 *
 * @date 06/12/2021
 *
 * @version 1.0
 *
 * @brief Kernel's futex API.
 *
 * @details Kernel's futex API. This module implements the futex system calls
 * and futex management.
 *
 * @copyright Alexy Torres Aurora Dugo
 ******************************************************************************/

#include <stdint.h>     /* Generic int types */
#include <queue.h>      /* Queues */
#include <uhashtable.h> /* Hash tables */
#include <kheap.h>      /* Kernel heap */
#include <scheduler.h>  /* Scheduler API */
#include <panic.h>      /* Kernel panix */

/* UTK configuration file */
#include <config.h>

/* Tests header file */
#ifdef TEST_MODE_ENABLED
#include <test_bank.h>
#endif

/* Header file */
#include <futex.h>

/*******************************************************************************
 * CONSTANTS
 ******************************************************************************/

/* None */

/*******************************************************************************
 * STRUCTURES
 ******************************************************************************/

/** @brief Futex data structure definition. */
struct futex_data
{
    /** @brief Futex waiting value */
    uint32_t wait;

    /** @brief The thread's node waiting on the futex */
    queue_node_t* waiting_thread;
};

/**
 * @brief Defines futex_data_t type as a shorcut for struct futex_data.
 */
typedef struct futex_data futex_data_t;


/*******************************************************************************
 * GLOBAL VARIABLES
 ******************************************************************************/

/** @brief Futex initialization status. */
bool_t is_init = FALSE;

/** @brief Futex hashtable that contains the lists of waiting threads. */
uhashtable_t* futex_table;

/*******************************************************************************
 * STATIC FUNCTIONS DECLARATION
 ******************************************************************************/

/* None */

/*******************************************************************************
 * FUNCTIONS
 ******************************************************************************/

#define CHECK_UNRECOVERABLE_ERROR(err) {    \
    if(err != OS_NO_ERR)                    \
    {                                       \
        EXIT_CRITICAL(int_state);           \
        KERNEL_PANIC(err);                  \
    }                                       \
}

OS_RETURN_E futex_init(void)
{
    OS_RETURN_E err;

    /* Create the hashtable */
    futex_table = uhashtable_create(UHASHTABLE_ALLOCATOR(kmalloc, kfree), &err);
    if(futex_table == NULL || err != OS_NO_ERR)
    {
        return err;
    }
    
    is_init = TRUE;

    return OS_NO_ERR;
}

void futex_wait(const SYSCALL_FUNCTION_E func, void* params)
{
    OS_RETURN_E   err;
    futex_t*      func_params;
    queue_t*      wait_queue;
    queue_node_t* wait_node;
    bool_t        created_queue;
    futex_data_t  data_info;  
    uint32_t      int_state;

    func_params = (futex_t*)params;

    if(func_params == NULL)
    {
        return;
    }

    if(func != SYSCALL_FUTEX_WAIT || is_init == FALSE)
    {

        func_params->addr  = NULL;
        func_params->val   = 0;
        func_params->error = is_init ? 
                                OS_ERR_UNAUTHORIZED_ACTION : 
                                OS_ERR_NOT_INITIALIZED;
    }

    func_params->error = OS_NO_ERR;
    created_queue      = FALSE;

    ENTER_CRITICAL(int_state);

    /* Get the futex waiting list */
    err = uhashtable_get(futex_table, 
                         (uintptr_t)func_params->addr, 
                         (void**)(&wait_queue));
    if(err != OS_NO_ERR)
    {
        /* No futex existed at this address, create it */
        if(err == OS_ERR_NO_SUCH_ID)
        {
            wait_queue = queue_create_queue(QUEUE_ALLOCATOR(kmalloc, kfree), 
                                            &err);
            if(err != OS_NO_ERR)
            {
                func_params->error = err;
                EXIT_CRITICAL(int_state);
                return;
            }

            err = uhashtable_set(futex_table, 
                                 (uintptr_t)func_params->addr,
                                 wait_queue);
            if(err != OS_NO_ERR)
            {
                func_params->error = err;
                err = queue_delete_queue(&wait_queue);
                CHECK_UNRECOVERABLE_ERROR(err);
                EXIT_CRITICAL(int_state);
                return;
            }

            created_queue = TRUE;
        }
        else 
        {
            func_params->error = err;
            EXIT_CRITICAL(int_state);
            return;
        }
    }

    /* Add the current thread to the waiting list */
    data_info.wait = func_params->val;
    data_info.waiting_thread = sched_lock_thread(THREAD_WAIT_TYPE_RESOURCE);
    if(data_info.waiting_thread == NULL)
    {
        if(created_queue == TRUE)
        {
            err = queue_delete_queue(&wait_queue);
            CHECK_UNRECOVERABLE_ERROR(err);
            err = uhashtable_remove(futex_table, 
                                    (uintptr_t)func_params->addr,
                                     NULL);
            CHECK_UNRECOVERABLE_ERROR(err);
        }
        func_params->error = OS_ERR_OUT_OF_BOUND;
        EXIT_CRITICAL(int_state);
        return;
    }

    wait_node = queue_create_node(&data_info, 
                                  QUEUE_ALLOCATOR(kmalloc, kfree), 
                                  &err);
    if(err != OS_NO_ERR)
    {
        err = sched_unlock_thread(data_info.waiting_thread, 
                                  THREAD_WAIT_TYPE_RESOURCE, 
                                  FALSE);
        CHECK_UNRECOVERABLE_ERROR(err);
        if(created_queue == TRUE)
        {
            err = queue_delete_queue(&wait_queue);
            CHECK_UNRECOVERABLE_ERROR(err);
            err = uhashtable_remove(futex_table, 
                                    (uintptr_t)func_params->addr,
                                    NULL);
            CHECK_UNRECOVERABLE_ERROR(err);
        }
        func_params->error = err;
        EXIT_CRITICAL(int_state);
        return;
    }

    err = queue_push(wait_node, wait_queue);
    if(err != OS_NO_ERR)
    {
        err = sched_unlock_thread(data_info.waiting_thread, 
                                  THREAD_WAIT_TYPE_RESOURCE, 
                                  FALSE);
        CHECK_UNRECOVERABLE_ERROR(err);
        err = queue_delete_node(&wait_node);
        CHECK_UNRECOVERABLE_ERROR(err);
        if(created_queue == TRUE)
        {
            err = queue_delete_queue(&wait_queue);
            CHECK_UNRECOVERABLE_ERROR(err);
            err = uhashtable_remove(futex_table, 
                                    (uintptr_t)func_params->addr,
                                    NULL);
            CHECK_UNRECOVERABLE_ERROR(err);
        }
        func_params->error = err;
        EXIT_CRITICAL(int_state);
        return;
    }
    
    EXIT_CRITICAL(int_state);

    /* Schedule the thread */
    sched_schedule();

    /* We returned from the schedule */
    func_params->error = OS_NO_ERR;
}

void futex_wake(const SYSCALL_FUNCTION_E func, void* params)
{
    OS_RETURN_E   err;
    futex_t*      func_params;
    queue_t*      wait_queue;
    queue_node_t* wait_node;
    futex_data_t* data_info;  
    uint32_t      int_state;

    func_params = (futex_t*)params;

    if(func_params == NULL)
    {
        return;
    }

    if(func != SYSCALL_FUTEX_WAIT || is_init == FALSE)
    {

        func_params->addr  = NULL;
        func_params->val   = 0;
        func_params->error = is_init ? 
                                OS_ERR_UNAUTHORIZED_ACTION : 
                                OS_ERR_NOT_INITIALIZED;
    }

    func_params->error = OS_NO_ERR;

    ENTER_CRITICAL(int_state);
    
    /* Get the futex waiting list */
    err = uhashtable_get(futex_table, 
                         (uintptr_t)func_params->addr, 
                         (void**)(&wait_queue));
    if(err != OS_NO_ERR)
    {
        /* In case of OS_ERR_NO_SUCH_ID No futex existed at this address, 
         * nothing to do */
        func_params->error = err;

        EXIT_CRITICAL(int_state);
        return;
    }

    /* Make the first thread in the list for which the value has change. 
     * If the list is empty, clear it */
    wait_node = wait_queue->head;
    while(wait_node != NULL)
    {
        data_info = (futex_data_t*)wait_node->data;
        if(data_info->wait != *func_params->addr)
        {
            /* We found out candidate, the value changed from what it was 
             * waiting from */
            break;
        }
        wait_node = wait_node->next;
    }
    /* Nothing was found */
    if(wait_node == NULL || err != OS_NO_ERR)
    {
        func_params->error = err;

        EXIT_CRITICAL(int_state);
        return;
    }

    /* Put back the thread in the scheduler */
    err = sched_unlock_thread(data_info->waiting_thread, 
                              THREAD_WAIT_TYPE_RESOURCE, 
                              FALSE);
    CHECK_UNRECOVERABLE_ERROR(err);

    /* Delete the node */
    err = queue_remove(wait_queue, wait_node);
    CHECK_UNRECOVERABLE_ERROR(err);
    err = queue_delete_node(&wait_node);
    CHECK_UNRECOVERABLE_ERROR(err);
    
    /* If this was the last entry in the queue, delete the queue */
    if(wait_queue->size == 0)
    {
        /* Delete the queue */
        err = queue_delete_queue(&wait_queue);
        CHECK_UNRECOVERABLE_ERROR(err);

        /* Remove from the hash table */
        err = uhashtable_remove(futex_table, 
                               (uintptr_t)func_params->addr, 
                               NULL);
        CHECK_UNRECOVERABLE_ERROR(err);
    }
}