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

#include <stdint.h>        /* Generic int types */
#include <queue.h>         /* Queues */
#include <uhashtable.h>    /* Hash tables */
#include <kheap.h>         /* Kernel heap */
#include <scheduler.h>     /* Scheduler API */
#include <panic.h>         /* Kernel panix */
#include <memmgt.h>        /* Memory management API */
#include <kernel_output.h> /* Kernel error output */
#include <string.h>        /* Memory manipualtion */

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

    /** @brief Tells if the futex was released after the owner died. */
    bool_t owner_died;

    /** @brief Contains the resource node in the resource list */
    queue_node_t* resource_node;
};

/**
 * @brief Defines futex_data_t type as a shorcut for struct futex_data.
 */
typedef struct futex_data futex_data_t;

/** @brief Futex thread resource structure, used for cleanup. */
struct futex_resource
{
    /** @brief The futex Id */
    uintptr_t futex_id;

    /** @brief The futex data create when the futex was used. */
    futex_data_t* associated_data;
};

/**
 * @brief Defines futex_data_t type as a shorcut for struct futex_data.
 */
typedef struct futex_resource futex_resource_t;

/** @brief Futex recover data structure, used for cleanup. */
struct recover_data
{
    /** @brief Created futex queue, NULL is none was created. */
    queue_t* created_futex_queue;

    /** @brief Contains the futex table that was modified, NULL is none. */
    uhashtable_t* futex_table;

    /** @brief Contains the node of the locked thread, NULL is not locked. */
    queue_node_t* locked_thread;

    /** @brief Contains the wait node created, NULL if not created. */
    queue_node_t* created_wait_node;

    /** @brief Contains the wait queue, NULL if nothing was pushed. */
    queue_t* pushed_wait_queue;

    /** @brief Contains the res node created, NULL if not created. */
    queue_node_t* created_res_node;
};

/**
 * @brief Defines recover_data_t type as a shorcut for struct recover_data.
 */
typedef struct recover_data recover_data_t;


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

/**
 * @brief Recovers from an error during the manipulation of a futex.
 *
 * @details Recovers from an error during the manipulation of a futex. This
 * function might generate an exception that either results in killing the
 * process or generating a kernel panic.
 *
 * @param[in,out] futex The futex that was modified.
 * @param[in,out] recover_data The recover data used for cleanup.
 * @param[in] error The error that caused the recover.
 */
static void futex_recover(futex_t* futex,
                          recover_data_t* recover_data,
                          const OS_RETURN_E error);

/**
 * @brief Cleans the resources used by a mutex.
 *
 * @details Cleans the resources used by a mutex. It releases the kernel queues
 * and nodes allocated to the resource.
 *
 * @param[in,out] futex_resource The resource sent by the kernel.
 */
static void futex_cleanup(void* futex_resource);

/*******************************************************************************
 * FUNCTIONS
 ******************************************************************************/

#define CHECK_ERROR_STATE(err, condition) {                         \
    if(err != OS_NO_ERR || (condition))                             \
    {                                                               \
        futex_recover(func_params, &recover_data, err);             \
        EXIT_CRITICAL(int_state)                                    \
        return;                                                     \
    }                                                               \
}

#define CHECK_ERROR_RECOVER(err) {  \
    if(err != OS_NO_ERR)            \
    {                               \
        KERNEL_PANIC(err);          \
    }                               \
}

static void futex_recover(futex_t* futex,
                          recover_data_t* recover_data,
                          const OS_RETURN_E error)
{
    OS_RETURN_E err;
    kernel_thread_t* thread;

    /* Sets the error */
    futex->error = error;

    /* Clean the resource. It is important to reverse the order of creation */
    if(recover_data->created_res_node != NULL)
    {
        thread = (kernel_thread_t*)recover_data->locked_thread->data;
        err = sched_thread_remove_resource(thread,
                                           &recover_data->created_res_node);
        CHECK_ERROR_RECOVER(err);
    }

    if(recover_data->pushed_wait_queue != NULL)
    {
        /* Here created wait node should always be non NULL. */
        err = queue_remove(recover_data->pushed_wait_queue,
                           recover_data->created_wait_node);
        CHECK_ERROR_RECOVER(err);
    }

    if(recover_data->created_wait_node != NULL)
    {
        err = queue_delete_node(&recover_data->created_wait_node);
        CHECK_ERROR_RECOVER(err);
    }

    if(recover_data->locked_thread != NULL)
    {
        err = sched_unlock_thread(recover_data->locked_thread,
                                  THREAD_WAIT_TYPE_RESOURCE,
                                  FALSE);
        CHECK_ERROR_RECOVER(err);
    }

    if(recover_data->futex_table != NULL)
    {
        err = uhashtable_remove(recover_data->futex_table,
                                (uintptr_t)futex->addr,
                                NULL);
        CHECK_ERROR_RECOVER(err);
    }

    if(recover_data->created_futex_queue != NULL)
    {
        err = queue_delete_queue(&recover_data->created_futex_queue);
        CHECK_ERROR_RECOVER(err);
    }
}

static void futex_cleanup(void* futex_resource)
{
    OS_RETURN_E      err;
    queue_t*         wait_queue;
    queue_node_t*    wait_node;
    uint32_t         int_state;

    futex_resource_t* resource;

    if(futex_resource == NULL)
    {
        KERNEL_ERROR("Futex cleanup called with null resource\n");
        return;
    }

    /* Initialize data */
    resource = (futex_resource_t*)futex_resource;

    ENTER_CRITICAL(int_state);

    /* Get the futex waiting list */
    err = uhashtable_get(futex_table,
                         resource->futex_id,
                         (void**)(&wait_queue));
    CHECK_ERROR_RECOVER(err);

    /* Get the node */
    wait_node = queue_find(wait_queue, (void*)resource->associated_data, &err);
    CHECK_ERROR_RECOVER(err);

    /* Delete the table node */
    err = queue_remove(wait_queue, wait_node);
    CHECK_ERROR_RECOVER(err);
    err = queue_delete_node(&wait_node);
    CHECK_ERROR_RECOVER(err);

    /* If this was the last entry in the queue, delete the queue */
    if(wait_queue->size == 0)
    {
        /* Remove from the hash table */
        err = uhashtable_remove(futex_table,
                                resource->futex_id,
                                NULL);
        CHECK_ERROR_RECOVER(err);

        /* Delete the queue */
        err = queue_delete_queue(&wait_queue);
        CHECK_ERROR_RECOVER(err);
    }

    EXIT_CRITICAL(int_state);
}

void futex_init(void)
{
    OS_RETURN_E err;

    /* Create the hashtable */
    futex_table = uhashtable_create(UHASHTABLE_ALLOCATOR(kmalloc, kfree), &err);
    if(futex_table == NULL || err != OS_NO_ERR)
    {
        KERNEL_ERROR("Could not initialize futex table: %d\n", err);
        KERNEL_PANIC(err);
    }

    is_init = TRUE;
}

void futex_wait(const SYSCALL_FUNCTION_E func, void* params)
{
    OS_RETURN_E      err;
    queue_t*         wait_queue;
    queue_node_t*    wait_node;
    futex_t*         func_params;
    futex_data_t     data_info;
    futex_resource_t resource;
    uint32_t         int_state;
    recover_data_t   recover_data;
    kernel_thread_t* thread;
    uintptr_t        futex_phys;

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
        return;
    }

    /* Initialize data */
    memset(&recover_data, 0, sizeof(recover_data_t));
    func_params->error = OS_NO_ERR;
    futex_phys         = memory_get_phys_addr((uintptr_t)func_params->addr);

    ENTER_CRITICAL(int_state);

    /* Check if the value has changed */
    if(*func_params->addr != func_params->val)
    {
        EXIT_CRITICAL(int_state);
        return;
    }

    /* Get the futex waiting list */
    err = uhashtable_get(futex_table,
                         futex_phys,
                         (void**)(&wait_queue));
    if(err != OS_NO_ERR)
    {
        /* No futex existed at this address, create it */
        if(err == OS_ERR_NO_SUCH_ID)
        {
            wait_queue = queue_create_queue(QUEUE_ALLOCATOR(kmalloc, kfree),
                                            &err);
            CHECK_ERROR_STATE(err, wait_queue == NULL);
            recover_data.created_futex_queue = wait_queue;

            err = uhashtable_set(futex_table,
                                 futex_phys,
                                 wait_queue);
            CHECK_ERROR_STATE(err, FALSE);
            recover_data.futex_table = futex_table;
        }
        else
        {
            futex_recover(func_params, &recover_data, err);
            return;
        }
    }

    /* Block the thread from scheduling */
    data_info.wait           = func_params->val;
    data_info.owner_died     = FALSE;
    data_info.waiting_thread = sched_lock_thread(THREAD_WAIT_TYPE_RESOURCE);

    CHECK_ERROR_STATE(OS_NO_ERR, data_info.waiting_thread == NULL);
    recover_data.locked_thread = data_info.waiting_thread;

    thread = (kernel_thread_t*)data_info.waiting_thread->data;

    /* Add the current thread to the waiting list */
    wait_node = queue_create_node(&data_info,
                                  QUEUE_ALLOCATOR(kmalloc, kfree),
                                  &err);
    CHECK_ERROR_STATE(err, wait_node == NULL);
    recover_data.created_wait_node = wait_node;

    err = queue_push(wait_node, wait_queue);
    CHECK_ERROR_STATE(err, FALSE);
    recover_data.pushed_wait_queue = wait_queue;

    /* Add the resource to the thread */
    resource.associated_data = &data_info;
    resource.futex_id        = futex_phys;

    err = sched_thread_add_resource(thread,
                                    &resource,
                                    futex_cleanup,
                                    &data_info.resource_node);
    CHECK_ERROR_STATE(err, FALSE);
    recover_data.created_res_node = data_info.resource_node;

    /* Schedule the thread */
    EXIT_CRITICAL(int_state);
    sched_schedule();

    /* We returned from the schedule */
    if(data_info.owner_died)
    {
        func_params->error = OS_ERR_OWNER_DIED;
    }
    else
    {
        func_params->error = OS_NO_ERR;
    }
}

void futex_wake(const SYSCALL_FUNCTION_E func, void* params)
{
    OS_RETURN_E      err;
    futex_t*         func_params;
    queue_t*         wait_queue;
    queue_node_t*    wait_node;
    queue_node_t*    save_node;
    futex_data_t*    data_info;
    uint32_t         int_state;
    size_t           i;
    recover_data_t   recover_data;
    kernel_thread_t* thread;
    uintptr_t        futex_phys;

    func_params = (futex_t*)params;

    if(func_params == NULL)
    {
        return;
    }

    if(func != SYSCALL_FUTEX_WAKE || is_init == FALSE)
    {

        func_params->addr  = NULL;
        func_params->val   = 0;
        func_params->error = is_init ?
                                OS_ERR_UNAUTHORIZED_ACTION :
                                OS_ERR_NOT_INITIALIZED;
        return;
    }

    /* Initialize data */
    memset(&recover_data, 0, sizeof(recover_data_t));
    func_params->error = OS_NO_ERR;
    futex_phys         = memory_get_phys_addr((uintptr_t)func_params->addr);


    ENTER_CRITICAL(int_state);

    /* Get the futex waiting list */
    err = uhashtable_get(futex_table,
                         futex_phys,
                         (void**)(&wait_queue));
    CHECK_ERROR_STATE(err, FALSE);

    /* Wake i threads */
    wait_node = wait_queue->head;
    for(i = 0; i < func_params->val; ++i)
    {
        /* Make the first thread in the list for which the value has change.
        * If the list is empty, clear it */
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

        /* Nothing was found, this returns */
        if(wait_node == NULL)
        {
            break;
        }

        save_node = wait_node;
        wait_node = wait_node->next;

        /* Remove the futex from the thread's resources */
        thread = (kernel_thread_t*)data_info->waiting_thread->data;
        err = sched_thread_remove_resource(thread,
                                           &data_info->resource_node);
        CHECK_ERROR_RECOVER(err);

        /* Put back the thread in the scheduler */
        err = sched_unlock_thread(data_info->waiting_thread,
                                  THREAD_WAIT_TYPE_RESOURCE,
                                  FALSE);
        CHECK_ERROR_RECOVER(err);

        /* Delete the node */
        err = queue_remove(wait_queue, save_node);
        CHECK_ERROR_RECOVER(err);
        err = queue_delete_node(&save_node);
        CHECK_ERROR_RECOVER(err);

        /* If this was the last entry in the queue, delete the queue */
        if(wait_queue->size == 0)
        {
            /* Remove from the hash table */
            err = uhashtable_remove(futex_table,
                                    futex_phys,
                                    NULL);
            CHECK_ERROR_RECOVER(err);

            /* Delete the queue */
            err = queue_delete_queue(&wait_queue);
            CHECK_ERROR_RECOVER(err);

            /* Nothing more to wake up */
            break;
        }
    }

    EXIT_CRITICAL(int_state);
}