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

/*******************************************************************************
 * INCLUDES
 ******************************************************************************/

/* Included headers */
/* None */

/* Configuration files */
#include <stdint.h>        /* Generic int types */
#include <kqueue.h>        /* Kernel queues lib */
#include <uhashtable.h>    /* Hash tables */
#include <kheap.h>         /* Kernel heap */
#include <scheduler.h>     /* Scheduler API */
#include <panic.h>         /* Kernel panix */
#include <memmgt.h>        /* Memory management API */
#include <kernel_output.h> /* Kernel error output */
#include <string.h>        /* Memory manipualtion */

/* Header file */
#include <futex.h>

/*******************************************************************************
 * CONSTANTS
 ******************************************************************************/

/* None */

/*******************************************************************************
 * STRUCTURES AND TYPES
 ******************************************************************************/

/** @brief Futex data structure definition. */
typedef struct
{
    /** @brief Futex waiting value */
    uint32_t wait;

    /** @brief The thread's node waiting on the futex */
    kqueue_node_t* waiting_thread;

    /** @brief Tells if the futex was released after the owner died. */
    bool_t owner_died;

    /** @brief Contains the resource node in the resource list */
    kqueue_node_t* resource_node;
} futex_data_t;

/** @brief Futex thread resource structure, used for cleanup. */
typedef struct
{
    /** @brief The futex Id */
    uintptr_t futex_id;

    /** @brief The futex data create when the futex was used. */
    futex_data_t* associated_data;
} futex_resource_t;

/** @brief Futex recover data structure, used for cleanup. */
typedef struct
{
    /** @brief Created futex queue, NULL is none was created. */
    kqueue_t* created_futex_queue;

    /** @brief Contains the futex table that was modified, NULL is none. */
    uhashtable_t* futex_table;

    /** @brief Contains the node of the locked thread, NULL is not locked. */
    kqueue_node_t* locked_thread;

    /** @brief Contains the wait node created, NULL if not created. */
    kqueue_node_t* created_wait_node;

    /** @brief Contains the wait queue, NULL if nothing was pushed. */
    kqueue_t* pushed_wait_queue;

    /** @brief Contains the res node created, NULL if not created. */
    kqueue_node_t* created_res_node;
} recover_data_t;

/*******************************************************************************
 * MACROS
 ******************************************************************************/

/**
 * @brief Assert macro used by the futex to ensure correctness of
 * execution.
 *
 * @details Assert macro used by the futex to ensure correctness of
 * execution. Due to the critical nature of the futex, any error
 * generates a kernel panic.
 *
 * @param[in] COND The condition that should be true.
 * @param[in] MSG The message to display in case of kernel panic.
 * @param[in] ERROR The error code to use in case of kernel panic.
 */
#define FUTEX_ASSERT(COND, MSG, ERROR) {                    \
    if((COND) == FALSE)                                     \
    {                                                       \
        PANIC(ERROR, "FUTEX", MSG, TRUE);                   \
    }                                                       \
}

/**
 * @brief Checks the futex state and recover in case of error.
 *
 * @details Checks the futex state and recover in case of error. This macro
 * makes the function return after executing futex recovery.
 *
 * @param[in] COND The condition that should be true.
 * @param[in] ERR The error value to check.
 */
#define CHECK_ERROR_STATE(ERR, COND) {                              \
    if(ERR != OS_NO_ERR || (COND))                                  \
    {                                                               \
        futex_recover(func_params, &recover_data, ERR);             \
        EXIT_CRITICAL(int_state)                                    \
        return;                                                     \
    }                                                               \
}

/*******************************************************************************
 * GLOBAL VARIABLES
 ******************************************************************************/

/************************* Imported global variables **************************/
/* None */

/************************* Exported global variables **************************/
/* None */

/************************** Static global variables ***************************/
/** @brief Futex initialization status. */
static bool_t is_init = FALSE;

/** @brief Futex hashtable that contains the lists of waiting threads. */
static uhashtable_t* futex_table;

/*******************************************************************************
 * STATIC FUNCTIONS DECLARATIONS
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
 * @brief Cleans the resources used by a futex.
 *
 * @details Cleans the resources used by a futex. It releases the kernel queues
 * and nodes allocated to the resource.
 *
 * @param[in,out] futex_resource The resource sent by the kernel.
 */
static void futex_cleanup(void* futex_resource);

/*******************************************************************************
 * FUNCTIONS
 ******************************************************************************/

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
        FUTEX_ASSERT(err == OS_NO_ERR,
                     "Could not recover from failed futex",
                     err);
    }

    if(recover_data->pushed_wait_queue != NULL)
    {
        /* Here created wait node should always be non NULL. */
        kqueue_remove(recover_data->pushed_wait_queue,
                      recover_data->created_wait_node,
                      TRUE);
    }

    if(recover_data->created_wait_node != NULL)
    {
        kqueue_delete_node(&recover_data->created_wait_node);
    }

    if(recover_data->locked_thread != NULL)
    {
        err = sched_unlock_thread(recover_data->locked_thread,
                                  THREAD_WAIT_TYPE_RESOURCE,
                                  FALSE);
        FUTEX_ASSERT(err == OS_NO_ERR,
                     "Could not recover from failed futex",
                     err);
    }

    if(recover_data->futex_table != NULL)
    {
        err = uhashtable_remove(recover_data->futex_table,
                                (uintptr_t)futex->addr,
                                NULL);
        FUTEX_ASSERT(err == OS_NO_ERR,
                     "Could not recover from failed futex",
                     err);
    }

    if(recover_data->created_futex_queue != NULL)
    {
        kqueue_delete_queue(&recover_data->created_futex_queue);
    }
}

static void futex_cleanup(void* futex_resource)
{
    OS_RETURN_E    err;
    kqueue_t*      wait_queue;
    kqueue_node_t* wait_node;
    uint32_t       int_state;

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
    FUTEX_ASSERT(err == OS_NO_ERR, "Could not cleanup futex", err);

    /* Get the node */
    wait_node = kqueue_find(wait_queue, (void*)resource->associated_data);
    FUTEX_ASSERT(wait_node != NULL, "Could not cleanup futex", err);

    /* Delete the table node */
    kqueue_remove(wait_queue, wait_node, TRUE);
    kqueue_delete_node(&wait_node);

    /* If this was the last entry in the queue, delete the queue */
    if(wait_queue->size == 0)
    {
        /* Remove from the hash table */
        err = uhashtable_remove(futex_table,
                                resource->futex_id,
                                NULL);
        FUTEX_ASSERT(err == OS_NO_ERR, "Could not cleanup futex", err);

        /* Delete the queue */
        kqueue_delete_queue(&wait_queue);
    }

    EXIT_CRITICAL(int_state);
}

void futex_init(void)
{
    OS_RETURN_E err;

    /* Create the hashtable */
    futex_table = uhashtable_create(UHASHTABLE_ALLOCATOR(kmalloc, kfree), &err);
    FUTEX_ASSERT(err == OS_NO_ERR, "Could not initialize futex table", err);

    is_init = TRUE;
}

void futex_wait(const SYSCALL_FUNCTION_E func, void* params)
{
    OS_RETURN_E      err;
    kqueue_t*        wait_queue;
    kqueue_node_t*   wait_node;
    futex_t*         func_params;
    futex_data_t     data_info;
    futex_resource_t resource;
    uint32_t         int_state;
    recover_data_t   recover_data;
    kernel_thread_t* thread;
    uintptr_t        futex_phys;

    func_params = (futex_t*)params;

    FUTEX_ASSERT(func == SYSCALL_FUTEX_WAIT,
                 "Wrong system call invocated", OS_ERR_INCORRECT_VALUE);

    FUTEX_ASSERT(func_params != NULL,
                 "NULL system call parameters", OS_ERR_NULL_POINTER);

    FUTEX_ASSERT(is_init != FALSE,
                 "Futex have not been initialized", OS_ERR_NOT_INITIALIZED);

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
            wait_queue = kqueue_create_queue();
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
    wait_node = kqueue_create_node(&data_info);
    recover_data.created_wait_node = wait_node;

    kqueue_push(wait_node, wait_queue);
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
    OS_RETURN_E       err;
    futex_t*          func_params;
    kqueue_t*         wait_queue;
    kqueue_node_t*    wait_node;
    kqueue_node_t*    save_node;
    futex_data_t*     data_info;
    uint32_t          int_state;
    size_t            i;
    recover_data_t    recover_data;
    kernel_thread_t*  thread;
    uintptr_t         futex_phys;

    func_params = (futex_t*)params;

    FUTEX_ASSERT(func == SYSCALL_FUTEX_WAKE,
                 "Wrong system call invocated", OS_ERR_INCORRECT_VALUE);

    FUTEX_ASSERT(func_params != NULL,
                 "NULL system call parameters", OS_ERR_NULL_POINTER);

    FUTEX_ASSERT(is_init != FALSE,
                 "Futex have not been initialized", OS_ERR_NOT_INITIALIZED);

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
        FUTEX_ASSERT(err == OS_NO_ERR, "Could not remove futex resource", err);

        /* Put back the thread in the scheduler */
        err = sched_unlock_thread(data_info->waiting_thread,
                                  THREAD_WAIT_TYPE_RESOURCE,
                                  FALSE);
        FUTEX_ASSERT(err == OS_NO_ERR, "Unlock futex thread", err);

        /* Delete the node */
        kqueue_remove(wait_queue, save_node, TRUE);
        kqueue_delete_node(&save_node);

        /* If this was the last entry in the queue, delete the queue */
        if(wait_queue->size == 0)
        {
            /* Remove from the hash table */
            err = uhashtable_remove(futex_table,
                                    futex_phys,
                                    NULL);
            FUTEX_ASSERT(err == OS_NO_ERR, "Could not remove futex", err);

            /* Delete the queue */
            kqueue_delete_queue(&wait_queue);

            /* Nothing more to wake up */
            break;
        }
    }

    EXIT_CRITICAL(int_state);
}

/************************************ EOF *************************************/