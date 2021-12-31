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

/*******************************************************************************
 * INCLUDES
 ******************************************************************************/

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

/*******************************************************************************
 * STRUCTURES AND TYPES
 ******************************************************************************/

/* None */

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

/**
 * @brief Initializes the scheduler service.
 *
 * @details Initializes the scheduler features and structures. The idle and
 * init threads are created. Once set, the scheduler starts to schedule the
 * threads.
 *
 * @warning This function will never return if the initialization was successful
 * and the scheduler started.
 */
void sched_init(void);

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
 * @param[in] ret_code The return code for the thread.
 *
 * @details Terminates a thread before its return. The return state of the
 * thread will be KILLED. The termination cause must be set before calling this
 * function.
 */
void sched_thread_terminate_self(void* ret_code);

/**
 * @brief Returns the number of time the scheduler was called.
 *
 * @details Returns the number of time the scheduler was called.
 *
 * @return The number of time the scheduler was called.
 */
uint64_t sched_get_schedule_count(void);

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
 * @brief Returns the current process handler.
 *
 * @details Returns the current process handler. If the system is not yet
 * initialized, the kernel main process handler is returned,
 *
 * @return The current process handler is returned.
 */
kernel_process_t* sched_get_current_process(void);

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
 * @return The success state or the error code. OS_NO_ERR is returned if no
 * error is encountered. Otherwise an error code is set.
 */
OS_RETURN_E sched_join_thread(kernel_thread_t* thread, void** ret_val,
                              THREAD_TERMINATE_CAUSE_E* term_cause);

/**
 * @brief System call handler to wait for a child process to terminate.
 *
 * @details System call handler to wait for a child process to terminate. Waits
 * for the process' main thread to terminate and return its status.
 *
 * @param[in] func The syscall function ID, must correspond to the wait_process
 * call.
 * @param[in, out] params The parameters used by the function, must be of type
 * waitpid_params_t.
 */
void sched_wait_process_pid(const SYSCALL_FUNCTION_E func, void* params);

/**
 * @brief System call handler to exit a process.
 *
 * @details System call handler to exit a process. The function will terminate
 * all the process' threads and clean its resources.
 *
 * @param[in] func The syscall function ID, must correspond to the exit_process
 * call.
 * @param[in] ret_value The proess desired return value.
 */
void sched_exit_process(const SYSCALL_FUNCTION_E func, void* ret_value);

/**
 * @brief System call handler to get the current thread parameters.
 *
 * @details System call handler to get the current thread parameters. This
 * system call fills as sched_param_t sctructure given as parameter.
 *
 * @param[in] func The syscall function ID, must correspond to the get_params
 * call.
 * @param[in, out] params The parameters used by the function, must be of type
 * sched_param_t.
 */
void sched_get_thread_params(const SYSCALL_FUNCTION_E func, void* params);

/**
 * @brief System call handler to set the current thread parameters.
 *
 * @details System call handler to set the current thread parameters. This
 * system call uses as sched_param_t sctructure given as parameter.
 *
 * @param[in] func The syscall function ID, must correspond to the set_params
 * call.
 * @param[in, out] params The parameters used by the function, must be of type
 * sched_param_t.
 */
void sched_set_thread_params(const SYSCALL_FUNCTION_E func, void* params);

/**
 * @brief Locks a thread from being scheduled.
 *
 * @details Removes the active thread from the active threads table, the thread
 * might be contained in an other structure such as a mutex.
 *
 * @param[in] block_type The type of block (mutex, sem, ...)
 *
 * @return The current thread system's node is returned on success. If the call
 * failed, NULL is returned.
 */
kqueue_node_t* sched_lock_thread(const THREAD_WAIT_TYPE_E block_type);

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
OS_RETURN_E sched_unlock_thread(kqueue_node_t* node,
                                const THREAD_WAIT_TYPE_E block_type,
                                const bool_t do_schedule);

/**
 * @brief Adds a resource to the thread's resource queue.
 *
 * @details AAds a resource to the thread's resource queue. The cleanup function
 * will be called with the resource as parameter to release the resource when
 * needed.
 *
 * @param[in,out] thread The thread to give the resource to.
 * @param[in] resource The resoutce to add to the thread.
 * @param[in] cleanup The cleanup fonction to call when the resource needs to be
 * released.
 * @param[out] resource_node The resource node buffer that will be filled with
 * the newly created resource node.
 *
 * @return The error status is returned.
 */
OS_RETURN_E sched_thread_add_resource(kernel_thread_t* thread,
                                      void* resource,
                                      void (*cleanup)(void* resource),
                                      kqueue_node_t** resource_node);

/**
 * @brief Removes a resource from the thread's resource queue.
 *
 * @details Removes a resource from the thread's resource queue. The cleanup
 * function is not called in this case.
 *
 * @param[in,out] thread The thread to remove the resource from.
 * @param[out] resource_node The resource node filled when calling
 * sched_thread_add_resource.
 *
 * @return The error status is returned.
 */
OS_RETURN_E sched_thread_remove_resource(kernel_thread_t* thread,
                                         kqueue_node_t** resource_node);

#endif /* #ifndef __CORE_SCHEDULER_H_ */

/************************************ EOF *************************************/
