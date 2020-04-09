/*******************************************************************************
 * @file scheduler.h
 *
 * @see scheduler.c
 *
 * @author Alexy Torres Aurora Dugo
 *
 * @date 16/12/2017
 *
 * @version 3.0
 *
 * @brief Kernel's thread scheduler.
 *
 * @details Kernel's thread scheduler. Thread creation and management functions
 * are located in this file.
 *
 * @copyright Alexy Torres Aurora Dugo
 ******************************************************************************/

#ifndef __CORE_SCHEDULER_H_
#define __CORE_SCHEDULER_H_

#include <lib/stddef.h>  /* Standard definitions */
#include <lib/stdint.h>  /* Generic int types */
#include <core/thread.h> /* Kernel thread */

/*******************************************************************************
 * CONSTANTS
 ******************************************************************************/

/** @brief Scheduler's thread lowest priority. */
#define KERNEL_LOWEST_PRIORITY  63
/** @brief Scheduler's thread highest priority. */
#define KERNEL_HIGHEST_PRIORITY 0
/** @brief Scheduler's idle thread priority. */
#define IDLE_THREAD_PRIORITY    KERNEL_LOWEST_PRIORITY

/** @brief Defines the idle task's stack size in bytes. */
#define SCHEDULER_IDLE_STACK_SIZE 0x1000
/** @brief Defines the init task's stack size in bytes. */
#define SCHEDULER_INIT_STACK_SIZE 0x2000
/** @brief Defines the main task's stack size in bytes. */
#define SCHEDULER_MAIN_STACK_SIZE KERNEL_STACK_SIZE

/*******************************************************************************
 * STRUCTURES
 ******************************************************************************/

/** @brief System's state enumeration. */
enum SYSTEM_STATE
{
    /** @brief System is booting. */
    SYSTEM_STATE_BOOTING,
    /** @brief System is running. */
    SYSTEM_STATE_RUNNING,
    /** @brief System is halted. */
    SYSTEM_STATE_HALTED
};

/**
 * @brief Defines SYSTEM_STATE_E type as a shorcut for enum SYSTEM_STATE.
 */
typedef enum SYSTEM_STATE SYSTEM_STATE_E;

/** @brief Thread information structure use to return thread information to the
 * user.
 */
struct thread_info
{
    /** @brief Thread's identifier. */
    int32_t tid;
    /** @brief Thread's parent identifier. */
    int32_t ptid;
    /** @brief Thread's name. */
    char    name[THREAD_MAX_NAME_LENGTH];

    /** @brief Thread's priority assigned at creation. */
    uint32_t init_prio;
    /** @brief Thread's current priority. */
    uint32_t priority;

    /** @brief Thread's CPU affinity. */
    uint32_t assigned_cpu;

    /** @brief Thread's current state. */
    THREAD_STATE_E state;

    /** @brief Wake up time limit for the sleeping thread. */
    uint32_t wakeup_time;

    /** @brief Thread's start time. */
    uint32_t start_time;
    /** @brief Thread's end time. */
    uint32_t end_time;
};
/**
 * @brief Defines thread_info_t type as a shorcut for struct thread_info.
 */
typedef struct thread_info thread_info_t;

/*******************************************************************************
 * FUNCTIONS
 ******************************************************************************/

/**
 * @brief Returns the current system's state.
 *
 * @details Returns the current system's state. See SYSTEM_STATE_E to know the
 * different system states.
 *
 * @returns The current system's state. See SYSTEM_STATE_E to know the
 * different system states.
 */
SYSTEM_STATE_E sched_get_system_state(void);

/**
 * @brief Initializes the scheduler service.
 *
 * @details Initializes the scheduler features and structures. The idle and
 * init threads are created. Once set, the scheduler starts to schedule the
 * threads.
 *
 * @warning This function will never return if the initialization was successful
 * and the scheduler started.
 *
 * @return The function inly returns in case of error.
 * - OS_ERR_UNAUTHORIZED_ACTION is returned if the initialization failed.
 * - Other types of value can be returned depending on the functions called at
 *   scheduler initialization.
 */
OS_RETURN_E sched_init(void);

/**
 * @brief Initializes the scheduler service for AP cores.
 *
 * @details Initializes the scheduler features and structures. The idle thread
 * for the AP core is created. Once set, the scheduler starts to schedule the
 * threads.
 *
 * @warning This function will never return if the initialization was successful
 * and the scheduler started.
 *
 * @return The function inly returns in case of error.
 * - OS_ERR_UNAUTHORIZED_ACTION is returned if the initialization failed.
 * - Other types of value can be returned depending on the functions called at
 *   scheduler initialization.
 */
OS_RETURN_E sched_init_ap(void);

/**
 * @brief Calls the scheduler dispatch function.
 *
 * @details Calls the scheduler. This will raise an interrupt since we should
 * never call the scheduler routine outside of an interrupt context.
 */
void sched_schedule(void);

/**
 * @brief Puts the calling thread to sleep.
 *
 * @details Puts the calling thread to sleep for at least time_ms microseconds.
 * The sleeping time can be greater depending on the time granularity and the
 * system's load.
 *
 * @param[in] time_ms The number of milliseconds to wait.
 *
 * @return The success state or the error code.
 * - OS_NO_ERR is returned if no error is encountered.
 * - OS_ERR_UNAUTHORIZED_ACTION is returned if sleep was called by the idle
 * thread.
 */
OS_RETURN_E sched_sleep(const uint32_t time_ms);

/**
 * @brief Returns the number of non dead threads.
 *
 * @details Returns the number of non dead threads.
 *
 * @returns The number of non dead threads.
 */
uint32_t sched_get_thread_count(void);

/**
 * @brief Returns the TID of the current executing thread.
 *
 * @details Returns the TID of the current executing thread.
 *
 * @returns The TID of the current executing thread.
 */
int32_t sched_get_tid(void);

/**
 * @brief Returns the thread structure of the current executing thread.
 *
 * @details Returns the thread structure of the current executing thread.
 *
 * @returns The thread structure of the current executing thread.
 */
kernel_thread_t* sched_get_self(void);

/**
 * @brief Returns the parent TID of the current executing thread.
 *
 * @details Returns the parent TID of the current executing thread.
 *
 * @returns The parent TID of the current executing thread.
 */
int32_t sched_get_ptid(void);

/**
 * @brief Returns the priority of the current executing thread.
 *
 * @details Returns the priority of the current executing thread.
 *
 * @returns The priority of the current executing thread.
 */
uint32_t sched_get_priority(void);

/**
 * @brief Sets the new priority of the current executing thread.
 *
 * @details Set the new priority of the current executing thread. The value of
 * the new priority is chekced and set if not error is encountered.
 *
 * @param[in] priority The desired priority of the thread.
 *
 * @return The success state or the error code.
 * - OS_NO_ERR is returned if no error is encountered.
 * - OS_ERR_FORBIDEN_PRIORITY is returned if the desired priority cannot be
 * aplied to the thread.
 */
OS_RETURN_E sched_set_priority(const uint32_t priority);

/**
 * @brief Creates a new kernel thread in the thread table.
 *
 * @details Creates a new thread added in the ready threads table. The thread
 * might not be directly scheduled depending on its priority and the current
 * system's load.
 * A handle to the thread is given as parameter and set on success.
 *
 * @warning These are kernel threads, sharing the kernel memory space and using
 * the kernel memory map and heap.
 *
 * @param[out] thread The pointer to the thread structure. This is the handle of
 * the thread for the user.
 * @param[in] priority The priority of the thread.
 * @param[in] name The name of the thread.
 * @param[in] stack_size The thread's stack size in bytes.
 * @param[in] cpu_affinity The CPU id on which the thread should execute.
 * @param[in] function The thread routine to be executed.
 * @param[in] args The arguments to be used by the thread.
 *
 * @return The success state or the error code.
 * - OS_NO_ERR is returned if no error is encountered.
 * - OS_ERR_FORBIDEN_PRIORITY is returned if the desired priority cannot be
 * aplied to the thread.
 * - OS_ERR_MALLOC is returned if the system could not allocate memory for the
 * new thread.
 * - OS_ERR_OUT_OF_BOUND if the desired stack size if out of the system's stack
 * size bounds.
 */
OS_RETURN_E sched_create_kernel_thread(thread_t* thread,
                                       const uint32_t priority,
                                       const char* name,
                                       const size_t stack_size,
                                       const uint32_t cpu_affinity,
                                       void* (*function)(void*),
                                       void* args);

/**
 * @brief Remove a thread from the threads table.
 *
 * @details Removes a thread from the threads table. The function will wait for
 * the thread to finish before removing it.
 *
 * @param[in] thread The handle of thread structure.
 * @param[out] ret_val The buffer to store the value returned by the thread's
 * routine.
 * @param[out] term_cause The buffer to store the termination cause of the
 * thread.
 *
 * @return The success state or the error code.
 * - OS_NO_ERR is returned if no error is encountered.
 * - OS_ERR_NULL_POINTER is returned if the thread handle is NULL.
 * - OS_ERR_NO_SUCH_ID is returned if thread cannot be found in the system.
 */
OS_RETURN_E sched_wait_thread(thread_t thread, void** ret_val,
                              THREAD_TERMINATE_CAUSE_E* term_cause);


/* Remove the active thread from the active threads table, the thread might be
 * contained in an other structure such as a mutex. The caller of this function
 * must call schedule() after.
 *
 * @param block_type The type of block (mutex, sem, ...)
 * @returns The node to the thread that has been locked. NULL is returned if the
 * current thread cannot be locked (idle).
 */


/**
 * @brief Locks a thread from behing scheduled.
 *
 * @details Removes the active thread from the active threads table, the thread
 * might be contained in an other structure such as a mutex. The caller of this
 * function must call schedule() after.
 *
 * @param block_type The type of block (mutex, sem, ...)
 *
 * @return The current thread system's node is returned on success. If the call
 * failed, NULL is returned.
 */
kernel_queue_node_t* sched_lock_thread(const THREAD_WAIT_TYPE_E block_type);


/**
 * @brief Unlocks a thread from behing scheduled.
 *
 * @details Unlocks a thread from behing scheduled. Adds a thread to the active
 * threads table, the thread might be contained in an other structure such as a
 * mutex.
 *
 * @param[in] node The node containing the thread to unlock.
 * @param[in] block_type The type of block (mutex, sem, ...)
 * @param[in] do_schedule Set to 1 the thread should be immediatly scheduled.
 *
 * @return The success state or the error code.
 * - OS_NO_ERR is returned if no error is encountered.
 * - OS_ERR_NULL_POINTER is returned if the thread handle is NULL.
 * - OS_ERR_NO_SUCH_ID is returned if thread cannot be found in the system.
 */
OS_RETURN_E sched_unlock_thread(kernel_queue_node_t* node,
                                const THREAD_WAIT_TYPE_E block_type,
                                const uint32_t do_schedule);

/**
 * @brief Gets all the system threads information.
 *
 * @details Gets all the system threads information. The function will fill the
 * structure given as parameter until there is no more thread to gather
 * information from or the function already gathered size threads. If size is
 * greater than the current threads count in the system then it will be modified
 * to the current threads count.
 *
 * @param[out] threads The array in wich we want to store the threads
 * information.
 * @param[in] size The size of the array given as parameter. If size is greater
 * than the current threads count in the system then it will be modified to the
 * current threads count.
 *
 * @return The success state or the error code.
 * - OS_NO_ERR is returned if no error is encountered.
 * - OS_ERR_NULL_POINTER is returned if the buffer or the size parameter are
 *   NULL.
 */
OS_RETURN_E sched_get_threads_info(thread_info_t* threads, size_t* size);

/**
 * @brief Set the current thread termination cause.
 *
 * @details Set the current thread termination cause in case of abnormal
 * termination.
 *
 * @param[in] term_cause The cause of the thread's termination.
 */
void sched_set_thread_termination_cause(const THREAD_TERMINATE_CAUSE_E
                                        term_cause);

/**
 * @brief Terninates a thread before it's normal termination.
 *
 * @details Terminates a thread before its return. The return state of the
 * thread will be KILLED. The termination cause must be set before calling this
 * function.
 */
void sched_terminate_thread(void);

/**
 * @brief Returns the number of time the scheduler was called.
 *
 * @details Returns the number of time the scheduler was called.
 *
 * @return The number of time the scheduler was called.
 */
uint64_t sched_get_schedule_count(void);

/**
 * @brief Returns the number of time the idle thread was schedulled.
 *
 * @details Returns the number of time the idle thread was schedulled.
 *
 * @return The number of time the idle thread was schedulled.
 */
uint64_t sched_get_idle_schedule_count(void);

/**
 * @brief Returns the address of the current thread's free page table.
 *
 * @details Returns the address of the current thread's free page table.
 *
 * @return The address of the current thread's free page table.
 */
uintptr_t sched_get_thread_free_page_table(void);

/**
 * @brief Returns the physical address of the current thread's page directory.
 *
 * @details Returns the physical address of the current thread's page directory.
 *
 * @return The address physical of the current thread's page directory.
 */
uintptr_t sched_get_thread_phys_pgdir(void);

#endif /* #ifndef __CORE_SCHEDULER_H_ */