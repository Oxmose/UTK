/*******************************************************************************
 * @file ctrl_block.h
 *
 * @author Alexy Torres Aurora Dugo
 *
 * @date 13/03/2021
 *
 * @version 3.0
 *
 * @brief Thread and process structures definitions.
 *
 * @details hread and process structures definitions. The files contains all the
 * data relative to the thread's management in the system (thread structure,
 * thread state) and processes.
 *
 * @copyright Alexy Torres Aurora Dugo
 ******************************************************************************/

#ifndef __CORE_CTRL_BLOCK_H_
#define __CORE_CTRL_BLOCK_H_

/*******************************************************************************
 * INCLUDES
 ******************************************************************************/

#include <stdint.h>       /* Generic int types */
#include <kqueue.h>       /* Kernel queues lib */
#include <cpu_settings.h> /* CPU structures */
#include <critical.h>     /* Critical sections */

/*******************************************************************************
 * CONSTANTS
 ******************************************************************************/

/** @brief Maximal thead's name length. */
#define THREAD_NAME_MAX_LENGTH 32

/*******************************************************************************
 * STRUCTURES AND TYPES
 ******************************************************************************/

/** @brief Thread's scheduling state. */
typedef enum
{
    /** @brief Thread's scheduling state: running. */
    THREAD_STATE_RUNNING,
    /** @brief Thread's scheduling state: running to be elected. */
    THREAD_STATE_READY,
    /** @brief Thread's scheduling state: sleeping. */
    THREAD_STATE_SLEEPING,
    /** @brief Thread's scheduling state: zombie. */
    THREAD_STATE_ZOMBIE,
    /** @brief Thread's scheduling state: joining. */
    THREAD_STATE_JOINING,
    /** @brief Thread's scheduling state: being copied. */
    THREAD_STATE_COPYING,
    /** @brief Thread's scheduling state: waiting. */
    THREAD_STATE_WAITING,
} THREAD_STATE_E;

/** @brief Thread waiting types. */
typedef enum
{
    /** @brief The thread is waiting to acquire a resource (e.g mutex, sem). */
    THREAD_WAIT_TYPE_RESOURCE,
    /** @brief The thread is waiting to acquire an IO entry. */
    THREAD_WAIT_TYPE_IO
} THREAD_WAIT_TYPE_E;

/** @brief Defines the possitble return state of a thread. */
typedef enum
{
    /** @brief The thread returned normally. */
    THREAD_RETURN_STATE_RETURNED,
    /** @brief The thread was killed before exiting normally. */
    THREAD_RETURN_STATE_KILLED,
} THREAD_RETURN_STATE_E;

/** @brief Thread's abnomarl exit cause. */
typedef enum
{
    /** @brief The thread returned normally.  */
    THREAD_TERMINATE_CORRECTLY,
    /** @brief The thread was killed because of a division by zero. */
    THREAD_TERMINATE_CAUSE_DIV_BY_ZERO,
    /** @brief The thread was killed by a panic condition. */
    THREAD_TERMINATE_CAUSE_PANIC
} THREAD_TERMINATE_CAUSE_E;

/**
 * @brief Define the thread's types in the kernel.
 */
typedef enum
{
    /** @brief Kernel thread type, create by and for the kernel. */
    THREAD_TYPE_KERNEL,
    /** @brief User thread type, created by the kernel for the user. */
    THREAD_TYPE_USER
} THREAD_TYPE_E;

/** @brief Kernel process structure. */
typedef struct kernel_process
{
    /** @brief Process identifier. */
    int32_t pid;

    /** @brief Process' return value. */
    int32_t return_val;

    /** @brief Parent process pointer. */
    struct kernel_process* parent_process;

    /** @brief Process main thread. */
    kqueue_node_t* main_thread;

    /** @brief List of the process threads. */
    kqueue_t* threads;

    /** @brief Process children list. */
    kqueue_t* children;

    /** @brief Process dead children list. */
    kqueue_t* dead_children;

    /** @brief Process free page table queue. */
    kqueue_t* free_page_table;

    /** @brief The process page directory pointer. */
    uintptr_t page_dir;

    /** @brief Process's name. */
    char name[THREAD_NAME_MAX_LENGTH];
} kernel_process_t;

/** @brief This is the representation of the thread for the kernel. */
typedef struct
{
    /** @brief Thread's identifier. */
    int32_t tid;

    /** @brief Parernt process */
    kernel_process_t* process;

    /** @brief Thread's name. */
    char name[THREAD_NAME_MAX_LENGTH];

    /** @brief Thread's type. */
    THREAD_TYPE_E type;

    /** @brief Thread's current priority. */
    uint8_t priority;

    /** @brief Thread's current state. */
    THREAD_STATE_E state;

    /** @brief Thread's wait type. This is inly relevant when the thread's state
     * is THREAD_STATE_WAITING.
     */
    THREAD_WAIT_TYPE_E block_type;

    /** @brief Thread's return state. This is only relevant when the thread
     * returned.
     */
    THREAD_RETURN_STATE_E return_state;

    /** @brief Thread's return state. This is only relevant when when
     * return state is not THREAD_RETURN_STATE_RETURNED.
     */
    THREAD_TERMINATE_CAUSE_E return_cause;

    /** @brief Thread's start arguments. */
    void* args;

    /** @brief Thread's routine. */
    void* (*function)(void*);

    /** @brief Thread's return value. */
    void* ret_val;

    /** @brief Thread's virtual CPU context */
    virtual_cpu_context_t cpu_context;

    /** @brief Thread's stack. */
    uintptr_t stack;

    /** @brief Thread's stack size. */
    uint32_t stack_size;

    /** @brief Thread's interrupt stack. */
    uintptr_t kstack;

    /** @brief Thread's interrupt stack size. */
    uint32_t kstack_size;

    /** @brief Wake up time limit for the sleeping thread. */
    uint64_t wakeup_time;

    /** @brief Pointer to the joining thread's node in the threads list. */
    kqueue_node_t* joining_thread;

    /** @brief Thread's start time. */
    uint64_t start_time;

    /** @brief Thread's end time. */
    uint64_t end_time;

    /** @brief Thread's resource queue. */
    kqueue_t* resources;
} kernel_thread_t;

/** @brief This is the representation of a thread's resource. */
typedef struct
{
    /** @brief The data used to represent the resource, can vary depending on
     * the resource. */
    void* data;

    /**
     * @brief The data used to represent the resource, can vary depending on
     * the resource. *
     *
     * @param[in, out] data The data used for the cleanup, if data was allocated
     * dynamically, it should be freed by this function.
     */
    void (*cleanup)(void* data);
} thread_resource_t;

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
 * FUNCTIONS
 ******************************************************************************/

/* None */

#endif /* #ifndef __CORE_CTRL_BLOCK_H_ */

/************************************ EOF *************************************/