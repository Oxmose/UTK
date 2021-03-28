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

#include <stdint.h>       /* Generic int types */
#include <queue.h>        /* Queues lib */
#include <cpu_settings.h> /* CPU structures */
#include <critical.h>     /* Critical sections */

/* UTK configuration file */
#include <config.h>

/*******************************************************************************
 * CONSTANTS
 ******************************************************************************/

#define THREAD_NAME_MAX_LENGTH 32

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
    /** @brief Thread's scheduling state: zombie. */
    THREAD_STATE_ZOMBIE,
    /** @brief Thread's scheduling state: joining. */
    THREAD_STATE_JOINING,
    /** @brief Thread's scheduling state: being copied. */
    THREAD_STATE_COPYING,
};

/**
 * @brief Defines THREAD_STATE_E type as a shorcut for enum THREAD_STATE.
 */
typedef enum THREAD_STATE THREAD_STATE_E;

/** @brief Thread waiting types. */
enum THREAD_WAIT_TYPE
{
    /** @brief The thread is waiting to acquire a resource (e.g mutex, sem). */
    THREAD_WAIT_TYPE_RESOURCE,
    /** @brief The thread is waiting to acquire an IO entry. */
    THREAD_WAIT_TYPE_IO
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
    THREAD_RETURN_STATE_KILLED,
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

/** @brief Kernel process structure. */
struct kernel_process
{
    /** @brief Process identifier. */
    int32_t pid;

    /** @brief Parent process pointer. */
    struct kernel_process* parent_process;

    /** @brief Process main thread. */
    queue_node_t* main_thread;

    /** @brief List of the process threads. */
    queue_t* threads;   

    /** @brief Process children list. */
    queue_t* children;

    /** @brief Process free page table quque. */
    queue_t* free_page_table;

    /** @brief The process page directory pointer. */
    uintptr_t page_dir;

    /** @brief Process's name. */
    char name[THREAD_NAME_MAX_LENGTH];
};

/**
 * @brief Defines kernel_process_t type as a shorcut for struct kernel_process.
 */
typedef struct kernel_process kernel_process_t;

/**
 * @brief Defines THREAD_TYPE_e type as a shorcut for enum THREAD_TYPE.
 */
typedef enum THREAD_TYPE THREAD_TYPE_E;

/** @brief This is the representation of the thread for the kernel. */
struct kernel_thread
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
    queue_node_t* joining_thread;

    /** @brief Thread's start time. */
    uint64_t start_time;

    /** @brief Thread's end time. */
    uint64_t end_time;

    /** @brief Thread's additional storage. */
    uint8_t thread_storage[THREAD_PRIVATE_STORAGE_SIZE];
};

/**
 * @brief Defines kernel_thread_t type as a shorcut for struct kernel_thread.
 */
typedef struct kernel_thread kernel_thread_t;

/*******************************************************************************
 * FUNCTIONS
 ******************************************************************************/

/* None */

#endif /* #ifndef __CORE_CTRL_BLOCK_H_ */