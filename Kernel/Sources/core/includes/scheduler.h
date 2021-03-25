/*******************************************************************************
 * @file scheduler.h
 *
 * @see scheduler.c
 *
 * @author Alexy Torres Aurora Dugo
 *
 * @date 13/03/2021
 *
 * @version 4.0
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

#include <stddef.h>     /* Standard definitions */
#include <stdint.h>     /* Generic int types */
#include <ctrl_block.h> /* Threads and processes control block */
#include <syscall.h>    /* System call manager */

/*******************************************************************************
 * CONSTANTS
 ******************************************************************************/

/** @brief Scheduler's thread lowest priority. */
#define KERNEL_LOWEST_PRIORITY  63
/** @brief Scheduler's thread highest priority. */
#define KERNEL_HIGHEST_PRIORITY 0
/** @brief Scheduler's thread initial priority. */
#define KERNEL_INIT_PRIORITY    KERNEL_HIGHEST_PRIORITY
/** @brief Scheduler's idle thread priority. */
#define IDLE_THREAD_PRIORITY    KERNEL_LOWEST_PRIORITY

/** @brief Defines the idle task's stack size in bytes. */
#define SCHEDULER_IDLE_STACK_SIZE 0x1000
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
kernel_thread_t* sched_get_current_thread(void);

/**
 * @brief Returns the process ID of the current executing thread.
 *
 * @details Returns the process ID of the current executing thread.
 *
 * @returns The process ID of the current executing thread.
 */
int32_t sched_get_pid(void);

/**
 * @brief Returns the parent process ID of the current executing thread.
 *
 * @details Returns the parent process ID of the current executing thread.
 *
 * @returns The parent process ID of the current executing thread.
 */
int32_t sched_get_ppid(void);

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
 * @brief Creates a new process.
 * 
 * @details Creates a new process. The process is associatd to a main thread 
 * that always executes in user space.
 *
 * @param[in] name The name of the thread.
 * @param[in] function The thread routine to be executed.
 * @param[in] argc The number of arguments to be used by the process.
 * @param[in] argv The arguments to be used by the process.
 *
 * @return The success state or the error code.
 * - OS_NO_ERR is returned if no error is encountered.
 * - OS_ERR_FORBIDEN_PRIORITY is returned if the desired priority cannot be
 * aplied to the thread.
 * - OS_ERR_MALLOC is returned if the system could not allocate memory for the
 * new process.
 * - OS_ERR_OUT_OF_BOUND if the desired stack size if out of the system's stack
 * size bounds.
 */
OS_RETURN_E sched_create_process_thread(const char* name,
                                        void* (*function)(void*),
                                        int argc,
                                        char** argv);

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
 * @param[in] type The thread's type.
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
OS_RETURN_E sched_create_kernel_thread(kernel_thread_t** thread,
                                       const uint32_t priority,
                                       const char* name,
                                       const size_t stack_size,
                                       const THREAD_TYPE_E type,
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
OS_RETURN_E sched_wait_thread(kernel_thread_t* thread, 
                              void** ret_val,
                              THREAD_TERMINATE_CAUSE_E* term_cause);


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
 * @brief Syscall handler to forks the current process.
 * 
 * @details Forks the current process. A complete copy of the current process
 * will be done and memory will be marked as COW for both new and current 
 * process. Only the calling thread will be copied to the new process.
 * 
 * @param[in] func The system call function id used to fork the current process.
 * @param[out] new_pid This buffer is used to is used to retune the new process 
 * and the PID. -1 is returned on error.
 */
void sched_fork_process(const SYSCALL_FUNCTION_E func, void* new_pid);

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
 * @param[in] type The thread type.
 * @param[in] stack_size The thread's stack size in bytes, must be a multiple of 
 * the system's page size.
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
 * - OS_ERR_UNAUTHORIZED_ACTION is the stack is not a multiple of the system's
 * page size.
 */
OS_RETURN_E sched_create_kernel_thread(kernel_thread_t** thread,
                                       const uint32_t priority,
                                       const char* name,
                                       const THREAD_TYPE_E type,
                                       const size_t stack_size,
                                       void* (*function)(void*),
                                       void* args);

/**
 * @brief Returns the current process handler.
 * 
 * @details Returns the current process handler. If the system is not yet 
 * initialized, the kernel main process handler is returned,
 * 
 * @return The current process handler is returned.
 */
kernel_process_t* sched_get_current_process(void);

#endif /* #ifndef __CORE_SCHEDULER_H_ */