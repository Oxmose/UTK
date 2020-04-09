/*******************************************************************************
 * @file thread.h
 *
 * @author Alexy Torres Aurora Dugo
 *
 * @date 03/10/2017
 *
 * @version 2.0
 *
 * @brief Thread's structures definitions.
 *
 * @details Thread's structures definitions. The files contains all the data
 * relative to the thread's management in the system (thread structure, thread
 * state).
 *
 * @copyright Alexy Torres Aurora Dugo
 ******************************************************************************/

#ifndef __CORE_THREAD_H_
#define __CORE_THREAD_H_

#include <lib/stdint.h>        /* Generic int types */
#include <core/kernel_queue.h> /* Kernel queues */
#include <cpu_structs.h>       /* CPU structures */
#include <sync/critical.h>     /* Critical sections */

/* UTK configuration file */
#include <config.h>

/*******************************************************************************
 * CONSTANTS
 ******************************************************************************/

/* None */

/*******************************************************************************
 * STRUCTURES
 ******************************************************************************/

/** @brief Thread's scheduling state. */
enum THREAD_STATE
{
    /** @brief Thread's scheduling state: running. */
    THREAD_STATE_RUNNING,
    /** @brief Thread's scheduling state: running to be elected. */
    THREAD_STATE_READY,
    /** @brief Thread's scheduling state: sleeping. */
    THREAD_STATE_SLEEPING,
    /** @brief Thread's scheduling state: dead. */
    THREAD_STATE_DEAD,
    /** @brief Thread's scheduling state: waiting to be joined. */
    THREAD_STATE_ZOMBIE,
    /** @brief Thread's scheduling state: joining a thread. */
    THREAD_STATE_JOINING,
    /** @brief Thread's scheduling state: waiting on an condition. */
    THREAD_STATE_WAITING
};

/**
 * @brief Defines THREAD_STATE_E type as a shorcut for enum THREAD_STATE.
 */
typedef enum THREAD_STATE THREAD_STATE_E;

/** @brief Thread waiting types. */
enum THREAD_WAIT_TYPE
{
    /** @brief The thread is waiting to acquire a semaphore. */
    THREAD_WAIT_TYPE_SEM,
    /** @brief The thread is waiting to acquire a mutex. */
    THREAD_WAIT_TYPE_MUTEX,
    /** @brief The thread is waiting to acquire a keyboard entry. */
    THREAD_WAIT_TYPE_IO_KEYBOARD
};

/**
 * @brief Defines THREAD_WAIT_TYPE_E type as a shorcut for enum
 * THREAD_WAIT_TYPE.
 */
typedef enum THREAD_WAIT_TYPE THREAD_WAIT_TYPE_E;

/** @brief Defines the possitble return state of a thread. */
enum THREAD_RETURN_STATE
{
    /** @brief The thread returned normally. */
    THREAD_RETURN_STATE_RETURNED,
    /** @brief The thread was killed before exiting normally. */
    THREAD_RETURN_STATE_KILLED
};

/**
 * @brief Defines THREAD_RETURN_STATE_E type as a shorcut for enum
 * THREAD_RETURN_STATE.
 */
typedef enum THREAD_RETURN_STATE THREAD_RETURN_STATE_E;

/** @brief Thread's abnomarl exit cause. */
enum THREAD_TERMINATE_CAUSE
{
    /** @brief The thread returned normally.  */
    THREAD_TERMINATE_CORRECTLY,
    /** @brief The thread was killed because of a division by zero. */
    THREAD_TERMINATE_CAUSE_DIV_BY_ZERO,
    /** @brief The thread was killed by a panic condition. */
    THREAD_TERMINATE_CAUSE_PANIC
};

/**
 * @brief Defines THREAD_TERMINATE_CAUSE_E type as a shorcut for enum
 * THREAD_TERMINATE_CAUSE.
 */
typedef enum THREAD_TERMINATE_CAUSE THREAD_TERMINATE_CAUSE_E;

/**
 * @brief Define the thread's types in the kernel.
 */
enum THREAD_TYPE
{
    /** @brief Kernel thread type, create by and for the kernel. */
    THREAD_TYPE_KERNEL,

    /** @brief User thread type, created by the kernel for the user. */
    THREAD_TYPE_USER
};

/**
 * @brief Defines THREAD_TYPE_e type as a shorcut for enum THREAD_TYPE.
 */
typedef enum THREAD_TYPE THREAD_TYPE_E;


/** @brief This is the representation of the thread for the kernel. */
struct kernel_thread
{
    /** @brief Thread's additional storage. */
    uint8_t thread_storage[1024];

    /** @brief Thread's identifier. */
    int32_t tid;
    /** @brief Thread's parent identifier. */
    int32_t ptid;
    /** @brief Thread's name. */
    char    name[THREAD_MAX_NAME_LENGTH];

    /** @brief Thread's type. */
    THREAD_TYPE_E type;

    /** @brief Thread's priority assigned at creation. */
    uint32_t init_prio;
    /** @brief Thread's current priority. */
    uint32_t priority;

    /** @brief Thread's current state. */
    THREAD_STATE_E           state;
    /** @brief Thread's wait type. This is inly relevant when the thread's state
     * is THREAD_STATE_WAITING.
     */
    THREAD_WAIT_TYPE_E       block_type;
    /** @brief Thread's return state. This is only relevant when the thread
     * returned.
     */
    THREAD_RETURN_STATE_E    return_state;
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

    virtual_cpu_context_t cpu_context;

    /** @brief Thread's stack. */
    uint32_t* stack;

    /** @brief Thread's stack size. */
    uint32_t stack_size;

    /** @brief Thread's free page table address. */
    uintptr_t free_page_table;

    /** @brief Wake up time limit for the sleeping thread. */
    uint64_t wakeup_time;

    /** @brief Pointer to the joining thread's node in the threads list. */
    kernel_queue_node_t* joining_thread;

    /** @brief Thread's children list. */
    kernel_queue_t* children;

    /** @brief Thread's start time. */
    uint64_t start_time;
    /** @brief Thread's end time. */
    uint64_t end_time;

    /** @brief Thread's CPU affinity. */
    uint32_t cpu_affinity;

#if MAX_CPU_COUNT > 1
    /** @brief Thread's concurency lock. */
    spinlock_t lock;
#endif
};

/**
 * @brief Defines kernel_thread_t type as a shorcut for struct kernel_thread_t.
 */
typedef struct kernel_thread kernel_thread_t;

/**
 * @brief Defines the user's thread type.
 */
typedef kernel_thread_t* thread_t;

/*******************************************************************************
 * FUNCTIONS
 ******************************************************************************/

/* None */

#endif /* #ifndef __CORE_THREAD_H_ */