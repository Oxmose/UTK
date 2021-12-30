/*******************************************************************************
 * @file scheduler.c
 *
 * @see scheduler.h
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

/*******************************************************************************
 * INCLUDES
 ******************************************************************************/

/* Included headers */
#include <stdint.h>             /* Generic int types */
#include <stddef.h>             /* Standard definitions */
#include <string.h>             /* String manipulation */
#include <stdlib.h>             /* Standard library */
#include <kheap.h>              /* Kernel heap */
#include <memmgt.h>             /* Memory management*/
#include <cpu_api.h>            /* CPU management */
#include <panic.h>              /* Kernel panic */
#include <interrupts.h>         /* Interrupt management */
#include <interrupt_settings.h> /* Interrupt settings */
#include <kernel_output.h>      /* Kernel output methods */
#include <graphic.h>            /* Graphic API */
#include <kqueue.h>             /* Kernel queues lib */
#include <critical.h>           /* Critical sections */
#include <time_management.h>    /* Timers factory */
#include <ctrl_block.h>         /* Threads and processes control block */
#include <init.h>               /* Init thread */
#include <syscall.h>            /* System call manager */
#include <kernel_error.h>       /* Kernel error codes */

/* Configuration files */
#include <config.h>
#include <test_bank.h>

/* Header file */
#include <scheduler.h>

/*******************************************************************************
 * CONSTANTS
 ******************************************************************************/

/** @brief Scheduler's thread initial priority. */
#define KERNEL_INIT_PRIORITY KERNEL_HIGHEST_PRIORITY
/** @brief Scheduler's idle thread priority. */
#define IDLE_THREAD_PRIORITY KERNEL_LOWEST_PRIORITY

/** @brief Defines the idle task's stack size in bytes. */
#define SCHEDULER_IDLE_STACK_SIZE 0x1000
/** @brief Defines the main task's stack size in bytes. */
#define SCHEDULER_MAIN_STACK_SIZE KERNEL_STACK_SIZE

/*******************************************************************************
 * STRUCTURES AND TYPES
 ******************************************************************************/

/** @brief Resource structure used by the scheduler to store the threads'
 * resources.
 */
typedef struct
{
    /** @brief The data representing the resource. Its type depends on the
     * manager that uses the resource.
     */
    void* data;

    /**
     * @brief The cleanup function used when the resource needs to be released.
     *
     * @details The cleanup function used when the resource needs to be
     * released. Its implementation depends on the resource manager.
     *
     * @param[in,out] data The data that represents the resource is passed as
     * parameter to the cleanup function.
     */
    void (*cleanup)(void* data);
} sched_resource_t;

/*******************************************************************************
 * MACROS
 ******************************************************************************/

/**
 * @brief Assert macro used by the scheduler to ensure correctness of
 * execution.
 *
 * @details Assert macro used by the scheduler to ensure correctness of
 * execution. Due to the critical nature of the scheduler, any error
 * generates a kernel panic.
 *
 * @param[in] COND The condition that should be true.
 * @param[in] MSG The message to display in case of kernel panic.
 * @param[in] ERROR The error code to use in case of kernel panic.
 */
#define SCHED_ASSERT(COND, MSG, ERROR) {                    \
    if((COND) == FALSE)                                     \
    {                                                       \
        PANIC(ERROR, "SCHED", MSG, TRUE);                   \
    }                                                       \
}

/*******************************************************************************
 * GLOBAL VARIABLES
 ******************************************************************************/

/************************* Imported global variables **************************/
/* None */

/************************* Exported global variables **************************/
/* None */

/************************** Static global variables ***************************/
/** @brief The last TID given by the kernel. */
static volatile uint32_t last_given_tid;

/** @brief The number of thread in the system (dead threads are not accounted).
 */
static volatile uint32_t thread_count;

/** @brief The last PID given by the kernel. */
static volatile uint32_t last_given_pid;

/** @brief The number of process in the system.
 */
static volatile uint32_t process_count;

/** @brief Idle thread handle. */
static kernel_thread_t* idle_thread;

/** @brief Kernel main process. */
static kernel_process_t* main_kprocess = NULL;

/** @brief Current active thread handle. */
static kernel_thread_t* active_thread = NULL;

/** @brief Current active thread queue node. */
static kqueue_node_t* active_thread_node = NULL;

/** @brief Current active process handler. */
static kernel_process_t* active_process = NULL;

/** @brief Count of the number of times the scheduler was called. */
static volatile uint64_t schedule_count;

/*******************************************************
 * THREAD TABLES
 * FIFO:
 *     - active_threads_table
 * Sorted by priority:
 *     - sleeping_threads: thread wakeup time
 *******************************************************/
/** @brief Active threads tables. The array is sorted by priority. */
static kqueue_t* active_threads_table[KERNEL_LOWEST_PRIORITY + 1];

/** @brief Sleeping threads table. The threads are sorted by their wakeup time
 * value.
 */
static kqueue_t* sleeping_threads_table;

/*******************************************************************************
 * STATIC FUNCTIONS DECLARATIONS
 ******************************************************************************/

/**
 * @brief Thread's exit point.
 *
 * @details Exit point of a thread. The function will release the resources of
 * the thread and manage its children. Put the thread
 * in a THREAD_STATE_ZOMBIE state. If an other thread is already joining the
 * active thread, then the joining thread will switch from blocked to ready
 * state.
 *
 * @param[in] cause The thread termination cause.
 * @param[in] ret_state The thread return state.
 * @param[in] ret_val The thread return value.
 */
static void thread_exit(const THREAD_TERMINATE_CAUSE_E cause,
                        const THREAD_RETURN_STATE_E ret_state,
                        void* ret_val);

/**
 * @brief Cleans a thread memory and resources.
 *
 * @details Cleans a thread memory and resources. The thread will be removed
 * from the memory. Before calling this function, the user must ensure the
 * thread is not used in any place in the system.
 *
 * @param[in] thread The thread to clean.
 */
static void sched_clean_thread(kernel_thread_t* thread);

/**
 * @brief Cleans the resources used by a thread.
 *
 * @details Cleans the resources used by a thread. The thread's resource queue
 * is walked and the resource cleanup function called for each resource. The
 * resources are then removed from the resource list.
 *
 * @param[in,out] thread The thread to clean.
 */
static void sched_clean_thread_resources(kernel_thread_t* thread);

/**
 * @brief Cleans a process memory and resources.
 *
 * @details Cleans a process memory and resources. The process will be removed
 * from the memory. Before calling this function, the user must ensure the
 * process is not used in any place in the system.
 *
 * @param[in] process The process to clean.
 */
static void sched_clean_process(kernel_process_t* process);

/**
 * @brief Copy the current thread to another kernel thread.
 *
 * @details Copy the current thread to another kernel thread. The copied thread
 * will be given a new tid, its state will be set to READY and the new thread
 * will not inherit the joining thread of the source thread.
 *
 * @param[out] dst_thread The thread that will receive the copy.
 *
 * @return The error status is returned.
 */
static OS_RETURN_E sched_copy_kernel_thread(kernel_thread_t* dst_thread);

/**
 * @brief Thread routine wrapper.
 *
 * @details Thread launch routine. Wrapper for the actual thread routine. The
 * wrapper will call the thread routine, pass its arguments and gather the
 * return value of the thread function to allow the joining thread to retreive
 * it. Some statistics about the thread might be added in this function.
 */
static void thread_wrapper(void);

/**
 * @brief Creates the main kernel process.
 *
 * @details Creates the main kernel process.
 */
static void create_main_kprocess(void);

/**
 * @brief Creates the IDLE thread.
 *
 * @details Creates the IDLE thread for the scheduler.
 *
 * @return On success OS_NO_ERR is returned. Otherwise an error code is given.
 */
static void create_idle(void);

/**
 * @brief Creates the INIT thread.
 *
 * @details Creates the INIT thread for the scheduler.
 *
 * @return On success OS_NO_ERR is returned. Otherwise an error code is given.
 */
static void create_init(void);

/**
 * @brief Selects the next thread to be scheduled.
 *
 * @details Selects the next thread to be scheduled. Sets the prev_thread and
 * active_thread pointers. The function will select the next most prioritary
 * thread to be executed. This function also wake up sleeping thread which
 * wake-up time has been reached
 */
static void select_thread(void);

/**
 * @brief Scheduler interrupt handler, executes the conetxt switch.
 *
 * @details Scheduling function. The function will call the select_thread
 * function and then set the CPU registers with the values on the
 * new active_thread stack.
 *
 * @warning THIS FUNCTION SHOULD NEVER BE CALLED OUTSIDE OF AN INTERRUPT.
 *
 * @param[in, out] cpu_state The pre interrupt CPU state.
 * @param[in] int_id The interrupt id when calling this function.
 * @param[in] stack_state The pre interrupt stack state.
 */
static void schedule_int(cpu_state_t* cpu_state,
                         uintptr_t int_id,
                         stack_state_t* stack_state);

/*******************************************************************************
 * FUNCTIONS
 ******************************************************************************/

static void thread_wrapper(void)
{
    void* ret_val;

    SCHED_ASSERT(active_thread->function != NULL,
                 "Thread routine cannot be NULL",
                 OS_ERR_NULL_POINTER);

    active_thread->start_time = time_get_current_uptime();

    /* Call thread's routine */
    ret_val = active_thread->function(active_thread->args);

    /* Exit thread properly */
    thread_exit(THREAD_TERMINATE_CORRECTLY,
                THREAD_RETURN_STATE_RETURNED,
                ret_val);
}

static void create_main_kprocess(void)
{
    main_kprocess = kmalloc(sizeof(kernel_process_t));

    SCHED_ASSERT(main_kprocess != NULL,
                 "Could not allocate kernel main process",
                 OS_ERR_MALLOC);

    main_kprocess->parent_process  = NULL;
    main_kprocess->pid             = last_given_pid++;
    main_kprocess->children        = kqueue_create_queue();
    main_kprocess->threads         = kqueue_create_queue();
    main_kprocess->free_page_table = memory_create_free_page_table();

    main_kprocess->page_dir = cpu_get_current_pgdir();
    strncpy(main_kprocess->name, "UTK-Kernel\0", 11);

    ++process_count;

    active_process = main_kprocess;
}

static void create_idle(void)
{
    OS_RETURN_E err;

    err = sched_create_kernel_thread(&idle_thread,
                                     IDLE_THREAD_PRIORITY,
                                     "IDLE",
                                     THREAD_TYPE_KERNEL,
                                     SCHEDULER_IDLE_STACK_SIZE,
                                     idle_sys,
                                     NULL);
    SCHED_ASSERT(err == OS_NO_ERR,
                 "Could not create IDLE thread",
                 err);

    /* Initializes the scheduler active thread */
    idle_thread->state = THREAD_STATE_READY;
    active_thread      = idle_thread;
    active_thread_node = kqueue_find(
                                    active_threads_table[idle_thread->priority],
                                    idle_thread);

    SCHED_ASSERT(active_thread_node != NULL,
                 "Could not create IDLE thread",
                 err);
}

static void create_init(void)
{
    OS_RETURN_E err;

    kernel_thread_t* init_thread;

    err = sched_create_kernel_thread(&init_thread,
                                     KERNEL_HIGHEST_PRIORITY,
                                     "INIT",
                                     THREAD_TYPE_KERNEL,
                                     SCHEDULER_MAIN_STACK_SIZE,
                                     init_sys,
                                     NULL);

    SCHED_ASSERT(err == OS_NO_ERR,
                 "Could not create INIT thread",
                 err);
}

static void select_thread(void)
{
    kqueue_node_t*   sleeping_node;
    uint32_t         i;
    uint64_t         current_time;
    kernel_thread_t* sleeping;

    current_time = time_get_current_uptime();

    /* If the thread was not locked, we put it in its queue */
    if(active_thread->state == THREAD_STATE_RUNNING)
    {
        active_thread->state = THREAD_STATE_READY;
        kqueue_push(active_thread_node,
                    active_threads_table[active_thread->priority]);
    }
    else if(active_thread->state == THREAD_STATE_SLEEPING)
    {
        kqueue_push_prio(active_thread_node,
                         sleeping_threads_table,
                         active_thread->wakeup_time);
    }

    /* Wake up the sleeping threads */
    do
    {
        KERNEL_DEBUG(SCHED_ELECT_DEBUG_ENABLED,
                     "[SCHED] Checking threads to wakeup");

        sleeping_node = kqueue_pop(sleeping_threads_table);

        /* If nothing to wakeup */
        if(sleeping_node == NULL)
        {
            break;
        }

        sleeping = (kernel_thread_t*)sleeping_node->data;

        /* If we should wakeup the thread */
        if(sleeping != NULL && sleeping->wakeup_time < current_time)
        {
            KERNEL_DEBUG(SCHED_ELECT_DEBUG_ENABLED,
                         "[SCHED] Waking up %d",
                         sleeping->tid);

            sleeping->state = THREAD_STATE_READY;
            kqueue_push(sleeping_node,
                        active_threads_table[sleeping->priority]);
        }
        else if(sleeping != NULL)
        {
            kqueue_push_prio(sleeping_node,
                             sleeping_threads_table,
                             sleeping->wakeup_time);
            break;
        }
    }while(sleeping_node != NULL);

    /* Get the new thread */
    active_thread_node = NULL;
    for(i = 0; i <= KERNEL_LOWEST_PRIORITY && active_thread_node == NULL; ++i)
    {
        active_thread_node = kqueue_pop(active_threads_table[i]);
    }
    SCHED_ASSERT(active_thread_node != NULL,
                 "Could not dequeue valid next thread",
                 OS_ERR_NULL_POINTER);

    active_thread = (kernel_thread_t*)active_thread_node->data;
    SCHED_ASSERT(active_thread != NULL,
                 "Could not dequeue valid next thread",
                 OS_ERR_NULL_POINTER);

    active_process       = active_thread->process;
    active_thread->state = THREAD_STATE_RUNNING;

    KERNEL_DEBUG(SCHED_ELECT_DEBUG_ENABLED,
                 "[SCHED] Elected new thread: %d",
                 active_thread->tid);
}

static void schedule_int(cpu_state_t* cpu_state,
                         uintptr_t int_id,
                         stack_state_t* stack_state)
{
    (void) int_id;

#if KERNEL_LOG_LEVEL >= DEBUG_LOG_LEVEL
    int32_t old_tid;

    old_tid = active_thread->tid;
#endif

    /* Search for next thread */
    select_thread();

    ++schedule_count;

#if KERNEL_LOG_LEVEL >= DEBUG_LOG_LEVEL
    if(old_tid != active_thread->tid)
    {
        KERNEL_DEBUG(SCHED_SWITCH_DEBUG_ENABLED,
                    "[SCHED] CPU Sched %d -> %d",
                    old_tid,
                    active_thread->tid);
    }
#endif

     /* Restore thread context, we should never return from here  */
    cpu_restore_context(cpu_state, stack_state, active_thread);

    SCHED_ASSERT(FALSE,
                 "Returned from context restore",
                 OS_ERR_UNAUTHORIZED_ACTION);
}

void sched_init(void)
{
    OS_RETURN_E err;
    uint32_t    i;

    /* Init scheduler settings */
    last_given_tid = 0;
    last_given_pid = 0;
    process_count  = 0;
    thread_count   = 0;
    schedule_count = 0;

    /* Init thread tables */
    for(i = 0; i < KERNEL_LOWEST_PRIORITY + 1; ++i)
    {
        active_threads_table[i] = kqueue_create_queue();
    }
    sleeping_threads_table = kqueue_create_queue();

    /* Create main kernel process */
    create_main_kprocess();

    /* Create idle and init thread */
    create_idle();
    create_init();

    /* Set main thread of main pross as the new active thread */
    main_kprocess->main_thread = active_thread_node;

    /* Register SW interrupt scheduling */
    err = kernel_interrupt_register_int_handler(SCHEDULER_SW_INT_LINE,
                                                schedule_int);
    SCHED_ASSERT(err == OS_NO_ERR,
                 "Could not set scheduler interrupt",
                 err);

    /* Register the scheduler on the main system timer. */
    err = time_register_scheduler(schedule_int);
    SCHED_ASSERT(err == OS_NO_ERR,
                 "Could not register scheduler interrupt",
                 err);

    KERNEL_DEBUG(SCHED_DEBUG_ENABLED, "[SCHED] Init scheduler");

    cpu_restore_context(NULL, NULL, idle_thread);
}

void sched_schedule(void)
{
    OS_RETURN_E err;

    /* Raise scheduling interrupt */
    err = cpu_raise_interrupt(SCHEDULER_SW_INT_LINE);
    SCHED_ASSERT(err == OS_NO_ERR,
                 "Could not raise schedule interrupt",
                 err);
}

OS_RETURN_E sched_sleep(const unsigned int time_ms)
{
    uint64_t curr_time;

    /* We cannot sleep in idle */
    if(active_thread == idle_thread)
    {
        KERNEL_ERROR("IDLE thread cannot sleep\n");
        return OS_ERR_UNAUTHORIZED_ACTION;
    }

    curr_time = time_get_current_uptime();
    active_thread->wakeup_time = curr_time + (uint64_t)time_ms * 1000000ULL;
    active_thread->state = THREAD_STATE_SLEEPING;

    KERNEL_DEBUG(SCHED_DEBUG_ENABLED,
                 "[SCHED] Thread %d asleep from %llu until %llu (%dms)",
                 active_thread->tid,
                 curr_time,
                 active_thread->wakeup_time,
                 time_ms);

    sched_schedule();

    return OS_NO_ERR;
}

uint32_t sched_get_thread_count(void)
{
    return thread_count;
}

uint64_t sched_get_schedule_count(void)
{
    return schedule_count;
}

/******************************************************************************
 * PROCESSES MANAGEMENT
 *****************************************************************************/

int32_t sched_get_ppid(void)
{
    if(active_process == NULL)
    {
        return -1;
    }
    return active_process->parent_process->pid;
}

int32_t sched_get_pid(void)
{
    if(active_process == NULL)
    {
        return -1;
    }
    return active_process->pid;
}

kernel_process_t* sched_get_current_process(void)
{
    return active_process;
}

static void sched_clean_process(kernel_process_t* process)
{
    kqueue_node_t*    thread_process;
    kernel_process_t* child_process;
    uint32_t          int_state;

    ENTER_CRITICAL(int_state);

    KERNEL_DEBUG(SCHED_DEBUG_ENABLED, "Cleaning process");

    /* Clean the process threads, note that the call to clean_thread will
     * remove the thread's node from the process' threads queue
     */
    thread_process = process->threads->head;
    while(thread_process != NULL)
    {
        sched_clean_thread(thread_process->data);

        thread_process = process->threads->head;
    }

    KERNEL_DEBUG(SCHED_DEBUG_ENABLED, "Cleaned process threads");

    /* Make all children process inherit INIT */
    thread_process = kqueue_pop(process->children);

    while(thread_process != NULL)
    {
        child_process = (kernel_process_t*)thread_process->data;
        child_process->parent_process = main_kprocess;
        kqueue_push(thread_process, main_kprocess->children);

        thread_process = kqueue_pop(process->children);
    }

    /* Update the INIT wait process queue if needed with current wait queue*/
    /* TODO */

    /* Clean page directory and frames */
    memory_delete_free_page_table(process->free_page_table);
    memory_clean_process_memory(process->page_dir);

    /* Clean process structures */
    kqueue_delete_queue(&process->threads);
    kqueue_delete_queue(&process->children);
    kfree(process);

    EXIT_CRITICAL(int_state);
}

/*****************************
 * PROCESS SYSCALLS
 ****************************/
void sched_fork_process(const SYSCALL_FUNCTION_E func, void* new_pid)
{

    kernel_process_t* new_proc;
    kernel_thread_t*  main_thread;
    kqueue_node_t*    main_thread_node;
    kqueue_node_t*    main_thread_node_th;
    kqueue_node_t*    new_proc_node;
    uint32_t          int_state;
    OS_RETURN_E       err;

    SCHED_ASSERT(new_pid != NULL,
                 "Internal error while forking process",
                 OS_ERR_NULL_POINTER);

    if(func != SYSCALL_FORK)
    {
        *(int32_t*)new_pid = -1;
        return;
    }

    if(process_count >= KERNEL_MAX_PROCESS)
    {
        *(int32_t*)new_pid = -1;
        return;
    }

    /* Allocate memory for the new process */
    new_proc = kmalloc(sizeof(kernel_process_t));
    if(new_proc == NULL)
    {
        *(int32_t*)new_pid = -1;
        return;
    }
    memset(new_proc, 0, sizeof(kernel_process_t));
    new_proc_node = kqueue_create_node(new_proc);

    ENTER_CRITICAL(int_state);

    /* Push the node in the current process children */
    kqueue_push(new_proc_node, active_process->children);

    /* Set the process control block */
    new_proc->children = kqueue_create_queue();
    new_proc->threads  = kqueue_create_queue();

    strncpy(new_proc->name,
            active_process->name,
            THREAD_NAME_MAX_LENGTH);

    /* Create the main process thread */
    main_thread = kmalloc(sizeof(kernel_thread_t));
    if(main_thread == NULL)
    {
        kqueue_remove(active_process->children, new_proc_node, TRUE);

        EXIT_CRITICAL(int_state);

        kqueue_delete_queue(&new_proc->children);
        kqueue_delete_queue(&new_proc->threads);
        kqueue_delete_node(&new_proc_node);
        kfree(new_proc);

        *(int32_t*)new_pid = -1;
        return;
    }

    main_thread->state = THREAD_STATE_COPYING;
    err = sched_copy_kernel_thread(main_thread);
    if(err != OS_NO_ERR)
    {
        kqueue_remove(active_process->children, new_proc_node, TRUE);

        EXIT_CRITICAL(int_state);

        kqueue_delete_queue(&new_proc->children);
        kqueue_delete_queue(&new_proc->threads);
        kqueue_delete_node(&new_proc_node);
        kfree(main_thread);
        kfree(new_proc);

        *(int32_t*)new_pid = -1;
        return;
    }

    main_thread->process = new_proc;

    /* Add the main process thread to the scheduler table and children table */
    main_thread_node = kqueue_create_node(main_thread);
    kqueue_push(main_thread_node, new_proc->threads);

    main_thread_node_th = kqueue_create_node(main_thread);
    main_thread->state = THREAD_STATE_READY;
    kqueue_push(main_thread_node_th,
                active_threads_table[main_thread->priority]);

    /* Update the main thread */
    new_proc->main_thread = main_thread_node_th;

    /* Create new free page table and page directory */
    err = memory_copy_self_mapping(new_proc,
                                   (void*)active_thread->kstack,
                                   active_thread->kstack_size);
    if(err != OS_NO_ERR)
    {
        kqueue_remove(new_proc->threads, main_thread_node, TRUE);
        kqueue_remove(active_threads_table[main_thread->priority],
                      main_thread_node_th, TRUE);

        sched_clean_thread_resources(main_thread);
        kqueue_remove(active_process->children, new_proc_node, TRUE);

        EXIT_CRITICAL(int_state);

        kqueue_delete_node(&main_thread_node);
        kqueue_delete_node(&main_thread_node_th);

        kqueue_delete_queue(&new_proc->children);
        kqueue_delete_queue(&new_proc->threads);
        kqueue_delete_node(&new_proc_node);
        kfree(main_thread);
        kfree(new_proc);

        *(int32_t*)new_pid = -1;
        return;
    }

    new_proc->pid            = last_given_pid++;
    new_proc->parent_process = active_process;

    ++process_count;

    KERNEL_DEBUG(SCHED_DEBUG_ENABLED,
                 "[SCHED] Forked current process %d to %d",
                 active_process->pid,
                 new_proc->pid);

    EXIT_CRITICAL(int_state);

    *(int32_t*)new_pid = new_proc->pid;
}

void sched_wait_process_pid(const SYSCALL_FUNCTION_E func, void* params)
{
    uint32_t                 int_state;
    kqueue_node_t*           child_node;
    kernel_process_t*        child;
    THREAD_TERMINATE_CAUSE_E term_cause;
    int32_t                  status;
    OS_RETURN_E              err;
    waitpid_params_t*        func_params;

    func_params = (waitpid_params_t*)params;

    if(func != SYSCALL_WAITPID)
    {
        if(func_params != NULL)
        {
            func_params->pid   = -1;
            func_params->error = OS_ERR_UNAUTHORIZED_ACTION;
        }
        return;
    }
    if(func_params == NULL)
    {
        return;
    }

    /* First we check if the PID is indeed one of the children of the current
     * process
     */
    ENTER_CRITICAL(int_state);

    KERNEL_DEBUG(SCHED_DEBUG_ENABLED,
                 "[SCHED] Process %d waiting for process %d",
                 active_process->pid, func_params->pid);

    child_node = active_process->children->head;
    child = NULL;
    while(child_node != NULL)
    {
        child = (kernel_process_t*)child_node->data;
        if(child->pid == func_params->pid)
        {
            break;
        }
        child_node = child_node->next;
    }

    EXIT_CRITICAL(int_state);

    if(child_node == NULL)
    {

        func_params->pid = -1;
        func_params->error = OS_ERR_NO_SUCH_ID;
        return;
    }
    term_cause = THREAD_TERMINATE_CORRECTLY;
    err = sched_join_thread(child->main_thread->data,
                            (void**)&status,
                            &term_cause);
    if(err != OS_NO_ERR)
    {
        func_params->pid = -1;
        func_params->status = err;
        func_params->error = err;
    }
    else
    {
        func_params->status = status;
        func_params->term_cause = (int32_t)term_cause;
        func_params->error = OS_NO_ERR;
    }

    sched_clean_process(child);

    kqueue_remove(active_process->children, child_node, TRUE);

    --process_count;

    EXIT_CRITICAL(int_state);

    kqueue_delete_node(&child_node);
}

/******************************************************************************
 * THREADS MANAGEMENT
 *****************************************************************************/

static void thread_exit(const THREAD_TERMINATE_CAUSE_E cause,
                        const THREAD_RETURN_STATE_E ret_state,
                        void* ret_val)
{
    kernel_thread_t* joining_thread;
    uint32_t         int_state;

    joining_thread = NULL;

    KERNEL_DEBUG(SCHED_DEBUG_ENABLED,
                 "[SCHED] Exit thread %d",
                 active_thread->tid);

    /* Cannot exit idle thread */
    SCHED_ASSERT(active_thread != idle_thread,
                 "Cannot exit IDLE thread",
                 OS_ERR_UNAUTHORIZED_ACTION);

    /* Set new thread state */
    active_thread->state = THREAD_STATE_ZOMBIE;

    ENTER_CRITICAL(int_state);

    /* Search for joining thread */
    if(active_thread->joining_thread != NULL)
    {
        joining_thread = (kernel_thread_t*)active_thread->joining_thread->data;

        if(joining_thread->state == THREAD_STATE_JOINING)
        {
            KERNEL_DEBUG(SCHED_DEBUG_ENABLED,
                         "Woke up joining thread %d",
                         joining_thread->tid);

            joining_thread->state = THREAD_STATE_READY;

            kqueue_push(active_thread->joining_thread,
                        active_threads_table[joining_thread->priority]);
        }
    }

    /* Clear the active thread node, it should not be in any queue at this point
     */
    kqueue_delete_node(&active_thread_node);

    /* Set the thread's stats and state */
    active_thread->end_time     = time_get_current_uptime();
    active_thread->ret_val      = ret_val;
    active_thread->return_cause = cause;
    active_thread->return_state = ret_state;

    EXIT_CRITICAL(int_state);

    /* Schedule thread */
    sched_schedule();

    /* We should never return */
    SCHED_ASSERT(FALSE,
                 "Thread retuned after exiting",
                 OS_ERR_UNAUTHORIZED_ACTION);
}

static void sched_clean_thread_resources(kernel_thread_t* thread)
{
    kqueue_node_t*    node;
    sched_resource_t* resource;

    while((node = kqueue_pop(thread->resources)) != NULL)
    {
        /* Call the resource's cleanup */
        resource = (sched_resource_t*)node->data;
        resource->cleanup(resource->data);

        /* Clean the node */
        kqueue_delete_node(&node);
    }

    /* Remove the resources queue */
    kqueue_delete_queue(&thread->resources);
}

static OS_RETURN_E sched_copy_kernel_thread(kernel_thread_t* dst_thread)
{
    uint32_t    int_state;
    SCHED_ASSERT(dst_thread != NULL,
                 "Tried to copy a NULL thread",
                 OS_ERR_NULL_POINTER);

    /* Copy metadata */
    memcpy(dst_thread, active_thread, sizeof(kernel_thread_t));

    /* Init new thread private data */
    dst_thread->state = THREAD_STATE_COPYING;
    dst_thread->joining_thread = NULL;

    /* Create a new resource queue */
    /* TODO: Maybe it will be usefull to change this and actually copy the
     * resources.
     */
    dst_thread->resources = kqueue_create_queue();

    ENTER_CRITICAL(int_state);

    /* Set new  TID */
    dst_thread->tid = last_given_tid++;

    KERNEL_DEBUG(SCHED_DEBUG_ENABLED,
                 "[SCHED] Copied thread %d to %d",
                 active_thread->tid,
                 dst_thread->tid);

    EXIT_CRITICAL(int_state);

    return OS_NO_ERR;
}

static void sched_clean_thread(kernel_thread_t* thread)
{
    kernel_process_t* process;
    kqueue_node_t*    thread_node;
    OS_RETURN_E       err;
    uint32_t          int_state;

    /* Remove thread from process table */
    process = thread->process;

    ENTER_CRITICAL(int_state);

    KERNEL_DEBUG(SCHED_DEBUG_ENABLED, "Cleaning thread");

    /* Clean thread's resources */
    sched_clean_thread_resources(thread);

    /* Clean the stacks */
    if(process != active_process)
    {
        memory_free_process_data((void*)thread->stack,
                                 thread->stack_size,
                                 process);
        memory_free_process_data((void*)thread->kstack,
                                 thread->kstack_size,
                                 process);
    }
    else
    {
        err = memory_free_stack((void*)thread->stack, thread->stack_size);
        SCHED_ASSERT(err == OS_NO_ERR,
                     "Could not clean thread's stacks",
                     err);
        err = memory_free_stack((void*)thread->kstack, thread->kstack_size);
        SCHED_ASSERT(err == OS_NO_ERR,
                     "Could not clean thread's stacks",
                     err);
    }

    KERNEL_DEBUG(SCHED_DEBUG_ENABLED, "Cleaned thread stacks");

    /* Remove from active thread table */
    thread_node = kqueue_find(active_threads_table[thread->priority], thread);
    if(thread_node != NULL)
    {
        kqueue_remove(active_threads_table[thread->priority],
                      thread_node,
                      TRUE);
        kqueue_delete_node(&thread_node);
    }

    /* Remove from sleeping table */
    thread_node = kqueue_find(sleeping_threads_table, thread);
    if(thread_node != NULL)
    {
        kqueue_remove(active_threads_table[thread->priority],
                      thread_node,
                      TRUE);
        kqueue_delete_node(&thread_node);
    }

    /* Remove from process table */
    thread_node = kqueue_find(process->threads, thread);
    SCHED_ASSERT(thread_node != NULL,
                 "Could not find thread in process threads table",
                 OS_ERR_NO_SUCH_ID);

    kqueue_remove(process->threads, thread_node, TRUE);
    kqueue_delete_node(&thread_node);

    /* Clean thread structure */
    kfree(thread);

    EXIT_CRITICAL(int_state);
}

kqueue_node_t* sched_lock_thread(const THREAD_WAIT_TYPE_E block_type)
{
    kqueue_node_t* current_thread_node;

    /* Cant lock kernel thread */
    if(active_thread == idle_thread)
    {
        return NULL;
    }

    current_thread_node = active_thread_node;

    /* Lock the thread */
    active_thread->state      = THREAD_STATE_WAITING;
    active_thread->block_type = block_type;

    KERNEL_DEBUG(SCHED_DEBUG_ENABLED, "Thread %d locked, reason: %d\n",
                 active_thread->tid, block_type);

    return current_thread_node;
}

OS_RETURN_E sched_unlock_thread(kqueue_node_t* node,
                                const THREAD_WAIT_TYPE_E block_type,
                                const bool_t do_schedule)
{
    uint32_t         int_state;
    kernel_thread_t* thread;

    thread = (kernel_thread_t*)node->data;

    /* Check thread value */
    if(thread == NULL || thread == idle_thread)
    {
        return OS_ERR_NO_SUCH_ID;
    }

    /* Check thread state */
    if(thread->state != THREAD_STATE_WAITING ||
       thread->block_type != block_type)
    {
        return OS_ERR_INCORRECT_VALUE;
    }

    ENTER_CRITICAL(int_state);

    /* Unlock thread state */
    thread->state = THREAD_STATE_READY;
    kqueue_push(node, active_threads_table[thread->priority]);

    KERNEL_DEBUG(SCHED_DEBUG_ENABLED, "Thread %d unlocked, reason: %d\n",
                 active_thread->tid, block_type);

    if(do_schedule == TRUE)
    {
        sched_schedule();
    }

    EXIT_CRITICAL(int_state);

    return OS_NO_ERR;
}

OS_RETURN_E sched_thread_add_resource(kernel_thread_t* thread,
                                      void* resource,
                                      void (*cleanup)(void* resource),
                                      kqueue_node_t** resource_node)
{
    sched_resource_t* res_node;

    if(thread == NULL || resource_node == NULL)
    {
        return OS_ERR_NULL_POINTER;
    }

    /* Create the node data */
    res_node = kmalloc(sizeof(sched_resource_t));
    if(res_node == NULL)
    {
        return OS_ERR_MALLOC;
    }
    res_node->cleanup = cleanup;
    res_node->data    = resource;

    /* Create the resource node */
    *resource_node = kqueue_create_node(res_node);

    /* Add the node to the queue */
    kqueue_push(*resource_node, thread->resources);

    return OS_NO_ERR;
}

OS_RETURN_E sched_thread_remove_resource(kernel_thread_t* thread,
                                         kqueue_node_t** resource_node)
{
    if(thread == NULL || resource_node == NULL)
    {
        return OS_ERR_NULL_POINTER;
    }

    /* Free the resource node data */
    kqueue_remove(thread->resources, *resource_node, TRUE);
    kfree((*resource_node)->data);
    kqueue_delete_node(resource_node);

    return OS_NO_ERR;
}

OS_RETURN_E sched_join_thread(kernel_thread_t* thread,
                              void** ret_val,
                              THREAD_TERMINATE_CAUSE_E* term_cause)
{
    if(thread == NULL)
    {
        return OS_ERR_NULL_POINTER;
    }

    KERNEL_DEBUG(SCHED_DEBUG_ENABLED,
                 "[SCHED] Thread %d waiting for thread %d",
                 active_thread->tid,
                 thread->tid);

    /* If there is already a joined thread, we cannot accept a new one. */
    if(thread->joining_thread != NULL)
    {
        return OS_ERR_UNAUTHORIZED_ACTION;
    }

    /* If thread already done then remove it from the thread table */
    if(thread->state == THREAD_STATE_ZOMBIE)
    {
        if(ret_val != NULL)
        {
            *ret_val = thread->ret_val;
        }
        sched_clean_thread(thread);
        return OS_NO_ERR;
    }

    /* Wait for the thread to finish */
    active_thread->state   = THREAD_STATE_JOINING;
    thread->joining_thread = active_thread_node;

    /* Schedule thread */
    sched_schedule();

    if(ret_val != NULL)
    {
        *ret_val = thread->ret_val;
    }
    if(term_cause != NULL)
    {
        *term_cause = thread->return_cause;
    }

    sched_clean_thread(thread);

    return OS_NO_ERR;
}

OS_RETURN_E sched_create_kernel_thread(kernel_thread_t** thread,
                                       const uint32_t priority,
                                       const char* name,
                                       const THREAD_TYPE_E type,
                                       const size_t stack_size,
                                       void* (*function)(void*),
                                       void* args)
{
    OS_RETURN_E      err;
    OS_RETURN_E      internal_err;
    kernel_thread_t* new_thread;
    kqueue_node_t*   new_thread_node;
    kqueue_node_t*   new_thread_node_table;
    uint32_t         int_state;

    /* Check if priority is valid */
    if(thread == NULL)
    {
        return OS_ERR_NULL_POINTER;
    }
    if(priority > KERNEL_LOWEST_PRIORITY)
    {
       return OS_ERR_FORBIDEN_PRIORITY;
    }

    if((stack_size & (KERNEL_PAGE_SIZE - 1)) != 0)
    {
        return OS_ERR_UNAUTHORIZED_ACTION;
    }

    new_thread = kmalloc(sizeof(kernel_thread_t));
    if(new_thread == NULL)
    {
        KERNEL_ERROR("Could not allocate thread structure\n");
        return OS_ERR_MALLOC;
    }
    new_thread_node       = kqueue_create_node(new_thread);
    new_thread_node_table = kqueue_create_node(new_thread);

    memset(new_thread, 0, sizeof(kernel_thread_t));

    /* Init thread settings */
    new_thread->process      = active_process;
    new_thread->type         = type;
    new_thread->priority     = priority;
    new_thread->state        = THREAD_STATE_READY;
    new_thread->args         = args;
    new_thread->function     = function;
    new_thread->kstack_size  = THREAD_KERNEL_STACK_SIZE;
    new_thread->stack_size   = stack_size;

    strncpy(new_thread->name, name, THREAD_NAME_MAX_LENGTH);

    /* Init thread stacks */
    new_thread->kstack = (uintptr_t)memory_alloc_stack(new_thread->kstack_size,
                                                       TRUE,
                                                       &err);
    if(new_thread->kstack == (uintptr_t)NULL || err != OS_NO_ERR)
    {
        KERNEL_ERROR("Could not allocate kernel stack structure\n");
        kqueue_delete_node(&new_thread_node);
        kqueue_delete_node(&new_thread_node_table);

        kfree(new_thread);
        return err;
    }

    new_thread->stack = (uintptr_t)memory_alloc_stack(new_thread->stack_size,
                                                      FALSE,
                                                      &err);
    if(new_thread->stack == (uintptr_t)NULL || err != OS_NO_ERR)
    {
        KERNEL_ERROR("Could not allocate stack structure\n");
        kqueue_delete_node(&new_thread_node);
        kqueue_delete_node(&new_thread_node_table);

        internal_err = memory_free_stack((void*)new_thread->kstack,
                                          new_thread->kstack_size);
        SCHED_ASSERT(internal_err == OS_NO_ERR,
                     "Internal error while creating thread",
                     internal_err);

        kfree(new_thread);
        return err;
    }

    /* Initialize thread's resource queue */
    new_thread->resources = kqueue_create_queue();

    cpu_init_thread_context(thread_wrapper, new_thread);

    ENTER_CRITICAL(int_state);

    /* Add the thread to the main kernel process. */
    kqueue_push(new_thread_node, active_process->threads);
    kqueue_push(new_thread_node_table,
                active_threads_table[new_thread->priority]);

    KERNEL_DEBUG(SCHED_DEBUG_ENABLED, "[SCHED] Kernel thread created");

    new_thread->tid = last_given_tid++;
    ++thread_count;

    EXIT_CRITICAL(int_state);

    *thread = new_thread;

    return OS_NO_ERR;
}

void sched_thread_terminate_self(void* ret_code)
{
    if(active_thread == NULL)
    {
        return;
    }

    /* Exit thread properly */
    thread_exit(THREAD_TERMINATE_CORRECTLY,
                THREAD_RETURN_STATE_KILLED,
                ret_code);
}

void sched_set_thread_termination_cause(const THREAD_TERMINATE_CAUSE_E cause)
{
    if(active_thread == NULL)
    {
        return;
    }

    active_thread->return_cause = cause;
}

int32_t sched_get_tid(void)
{
    if(active_thread == NULL)
    {
        return -1;
    }
    return active_thread->tid;
}

kernel_thread_t* sched_get_current_thread(void)
{
    if(active_thread == NULL)
    {
        return NULL;
    }
    return active_thread;
}

/*****************************
 * THREAD SYSCALLS
 ****************************/
void sched_get_thread_params(const SYSCALL_FUNCTION_E func, void* params)
{
    sched_param_t*           func_params;

    func_params = (sched_param_t*)params;

    if(func != SYSCALL_SCHED_GET_PARAMS)
    {
        if(func_params != NULL)
        {
            func_params->pid   = -1;
            func_params->tid   = -1;
            func_params->error = OS_ERR_UNAUTHORIZED_ACTION;
        }
        return;
    }
    if(func_params == NULL)
    {
        return;
    }

    /* Fills the structure. Here we will add new parameters when needed */
    func_params->pid = active_process->pid;
    func_params->tid = active_thread->tid;
    func_params->priority = active_thread->priority;

    func_params->error = OS_NO_ERR;
}

void sched_set_thread_params(const SYSCALL_FUNCTION_E func, void* params)
{
    sched_param_t*           func_params;

    func_params = (sched_param_t*)params;

    if(func != SYSCALL_SCHED_SET_PARAMS)
    {
        if(func_params != NULL)
        {
            func_params->pid   = -1;
            func_params->tid   = -1;
            func_params->error = OS_ERR_UNAUTHORIZED_ACTION;
        }
        return;
    }
    if(func_params == NULL)
    {
        return;
    }

    func_params->error = OS_NO_ERR;

    /* Here we will set new parameters when needed */
    if(func_params->priority <= KERNEL_LOWEST_PRIORITY)
    {
        active_thread->priority = func_params->priority;
    }
    else
    {
        func_params->error = OS_ERR_FORBIDEN_PRIORITY;
    }
}

/************************************ EOF *************************************/