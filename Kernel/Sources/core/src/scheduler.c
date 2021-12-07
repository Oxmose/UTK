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
#include <queue.h>              /* Kernel queues */
#include <critical.h>           /* Critical sections */
#include <time_management.h>    /* Timers factory */
#include <ctrl_block.h>         /* Threads and processes control block */
#include <init.h>               /* Init thread */
#include <syscall.h>            /* System call manager */
#include <kernel_error.h>       /* Kernel error codes */

/* UTK configuration file */
#include <config.h>

/* Test header */
#ifdef TEST_MODE_ENABLED
#include <test_bank.h>
#endif

/* Header file */
#include <scheduler.h>

/*******************************************************************************
 * CONSTANTS
 ******************************************************************************/

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

/* None */

/*******************************************************************************
 * GLOBAL VARIABLES
 ******************************************************************************/

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
static queue_node_t*    active_thread_node;

/** @brief Current active process handler. */
static kernel_process_t* active_process;

/** @brief Current system state. */
static volatile SYSTEM_STATE_E system_state;

/** @brief Count of the number of times the scheduler was called. */
static volatile uint64_t schedule_count;

/*******************************************************
 * THREAD TABLES
 * Sorted by priority:
 *     - sleeping_threads: thread wakeup time
 *
 * Global thread table used to browse the threads, even those
 * kept in a mutex / semaphore or other structure and that do
 * not appear in the three other tables.
 *
 *******************************************************/
/** @brief Active threads tables. The array is sorted by priority. */
static queue_t* active_threads_table[KERNEL_LOWEST_PRIORITY + 1];

/** @brief Sleeping threads table. The threads are sorted by their wakeup time
 * value.
 */
static queue_t* sleeping_threads_table;

/*******************************************************************************
 * STATIC FUNCTIONS DECLARATION
 ******************************************************************************/

static void sched_schedule(void);

/**
 * @brief Thread's exit point.
 *
 * @details Exit point of a thread. The function will release the resources of
 * the thread and manage its children (INIT will inherit them). Put the thread
 * in a THREAD_STATE_ZOMBIE state. If an other thread is already joining the
 * active thread, then the joining thread will switch from blocked to ready
 * state.
 */
static void thread_exit(void);

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
 * @brief Cleans a process memory and resources.
 *
 * @details Cleans a process memory and resources. The process will be removed
 * from the memory. Before calling this function, the user must ensure the 
 * thread is not used in any place in the system.
 *
 * @param[in] thread The thread to clean.
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
 */
static void sched_copy_kernel_thread(kernel_thread_t* dst_thread);

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

static void sched_schedule(void)
{
    OS_RETURN_E err;

    /* Raise scheduling interrupt */
    err = cpu_raise_interrupt(SCHEDULER_SW_INT_LINE);
    if(err != OS_NO_ERR)
    {
        KERNEL_ERROR("Could not raise schedule interrupt\n");
        KERNEL_PANIC(err);
    }
}

static void thread_exit(void)
{
    OS_RETURN_E      err;
    kernel_thread_t* joining_thread;
    uint32_t         int_state;

    joining_thread = NULL;

    KERNEL_DEBUG(SCHED_DEBUG_ENABLED, 
                 "[SCHED] Exit thread %d", 
                 active_thread->tid);

    /* Cannot exit idle thread */
    if(active_thread == idle_thread)
    {
        KERNEL_ERROR("Cannot exit idle thread[%d]\n",
                     OS_ERR_UNAUTHORIZED_ACTION);
        KERNEL_PANIC(OS_ERR_UNAUTHORIZED_ACTION);
    }

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

            err = queue_push(active_thread->joining_thread,
                             active_threads_table[joining_thread->priority]);
            if(err != OS_NO_ERR)
            {
                KERNEL_ERROR("Could not enqueue joining thread[%d]\n", err);
                KERNEL_PANIC(err);
            }
        }
    }

    /* Clear the active thread node, it should not be in any queue at this point
     */
    err = queue_delete_node(&active_thread_node);
    if(err != OS_NO_ERR)
    {
        KERNEL_ERROR("Could not remove thread node\n");
        KERNEL_PANIC(err);
    }

    EXIT_CRITICAL(int_state);

    /* Schedule thread */
    sched_schedule();
}

static void sched_clean_thread(kernel_thread_t* thread)
{
    kernel_process_t* process;
    queue_node_t*     thread_node;
    OS_RETURN_E       err;
    uint32_t          int_state;

    /* Remove thread from process table */
    process = thread->process;

    ENTER_CRITICAL(int_state);

    KERNEL_DEBUG(SCHED_DEBUG_ENABLED, "Cleaning thread");

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
        err |= memory_free_stack((void*)thread->kstack, thread->kstack_size);
        if(err != OS_NO_ERR)
        {
            KERNEL_ERROR("Could clean thread's stacks\n");
            KERNEL_PANIC(err);
        }
    }

    KERNEL_DEBUG(SCHED_DEBUG_ENABLED, "Cleaned thread stacks");

    /* Remove from active thread table */
    thread_node = queue_find(active_threads_table[thread->priority],
                             thread, 
                             &err);
    if(err == OS_NO_ERR)
    {
        err = queue_remove(active_threads_table[thread->priority], thread_node);
        if(err != OS_NO_ERR)
        {
            KERNEL_ERROR("Could not remove thread active threads table\n");
            KERNEL_PANIC(err);
        }
        err = queue_delete_node(&thread_node);
        if(err != OS_NO_ERR)
        {
            KERNEL_ERROR("Could not remove thread node\n");
            KERNEL_PANIC(err);
        }
    }
    else if(err != OS_ERR_NO_SUCH_ID)
    {
        KERNEL_ERROR("Could not find thread in active threads table\n");
        KERNEL_PANIC(err); 
    }

    /* Remove from sleeping table */
    thread_node = queue_find(sleeping_threads_table,
                             thread, 
                             &err);
    if(err == OS_NO_ERR)
    {
        err = queue_remove(active_threads_table[thread->priority], thread_node);
        if(err != OS_NO_ERR)
        {
            KERNEL_ERROR("Could not remove thread sleeping threads table\n");
            KERNEL_PANIC(err);
        }
        err = queue_delete_node(&thread_node);
        if(err != OS_NO_ERR)
        {
            KERNEL_ERROR("Could not remove thread node\n");
            KERNEL_PANIC(err);
        }
    }
    else if(err != OS_ERR_NO_SUCH_ID)
    {
        KERNEL_ERROR("Could not find thread in sleeping threads table\n");
        KERNEL_PANIC(err); 
    }

    /* Remove from process table */
    thread_node = queue_find(process->threads, thread, &err);
    if(err != OS_NO_ERR)
    {
        KERNEL_ERROR("Could not find thread in process table\n");
        KERNEL_PANIC(err);
    }
    
    err = queue_remove(process->threads, thread_node);
    if(err != OS_NO_ERR)
    {
        KERNEL_ERROR("Could not remove thread in process table\n");
        KERNEL_PANIC(err);
    }

    err = queue_delete_node(&thread_node);
    if(err != OS_NO_ERR)
    {
        KERNEL_ERROR("Could not remove thread node\n");
        KERNEL_PANIC(err);
    }

    /* Clean thread structure */
    kfree(thread);

    EXIT_CRITICAL(int_state);
}

static void sched_clean_process(kernel_process_t* process)
{
    queue_node_t*     thread_process;
    kernel_process_t* child_process;
    uint32_t          int_state;
    OS_RETURN_E       err;
    
    ENTER_CRITICAL(int_state);

    KERNEL_DEBUG(SCHED_DEBUG_ENABLED, "Cleaning process");

    /* Clean the process threads */
    thread_process = process->threads->head;
    while(thread_process != NULL)
    {
        sched_clean_thread(thread_process->data);

        thread_process = process->threads->head;
    }

    KERNEL_DEBUG(SCHED_DEBUG_ENABLED, "Cleaned process threads");

    /* Make all children process inherit the parent */
    thread_process = queue_pop(process->children, &err);
    if(err != OS_NO_ERR)
    {
        KERNEL_ERROR("Error while inheriting children\n");
        KERNEL_PANIC(err);
    }
    while(thread_process != NULL)
    {
        if(err != OS_NO_ERR)
        {
            KERNEL_ERROR("Error while inheriting children\n");
            KERNEL_PANIC(err);
        }

        child_process = (kernel_process_t*)thread_process->data;
        child_process->parent_process = process->parent_process;
        err = queue_push(thread_process, process->parent_process->children);
        if(err != OS_NO_ERR)
        {
            KERNEL_ERROR("Error while inheriting children\n");
            KERNEL_PANIC(err);
        }
        
        thread_process = queue_pop(process->children, &err);
        if(err != OS_NO_ERR)
        {
            KERNEL_ERROR("Error while inheriting children\n");
            KERNEL_PANIC(err);
        }
    }

    /* Clean page directory and frames */
    memory_delete_free_page_table(process->free_page_table);
    memory_clean_process_memory(process->page_dir);

    /* Clean process structures */
    err = queue_delete_queue(&process->threads);
    if(err != OS_NO_ERR)
    {
        KERNEL_ERROR("Error cleaning threads\n");
        KERNEL_PANIC(err);
    } 

    err = queue_delete_queue(&process->children);
    if(err != OS_NO_ERR)
    {
        KERNEL_ERROR("Error cleaning children\n");
        KERNEL_PANIC(err);
    } 
    kfree(process);

    EXIT_CRITICAL(int_state);
}

static void sched_copy_kernel_thread(kernel_thread_t* dst_thread)
{
    uint32_t int_state;

    if(dst_thread == NULL)
    {
        KERNEL_ERROR("Tried to copy a NULL thread\n");
        KERNEL_PANIC(OS_ERR_NULL_POINTER);
    }

    /* Copy metadata */
    memcpy(dst_thread, active_thread, sizeof(kernel_thread_t));

    /* Init new thread private data */
    dst_thread->state = THREAD_STATE_COPYING;
    dst_thread->joining_thread = NULL;

    ENTER_CRITICAL(int_state);

    /* Set new  TID */
    dst_thread->tid = last_given_tid++;  

    KERNEL_DEBUG(SCHED_DEBUG_ENABLED, 
                 "[SCHED] Copied thread %d to %d",
                 active_thread->tid, 
                 dst_thread->tid);

    EXIT_CRITICAL(int_state);
}                                        

static void thread_wrapper(void)
{
    /* TODO: At some point this function should be a CRT0 for threds in user
     * memory and not kernel memory
     */
    uint64_t start_time;
    uint64_t end_time;
    void*    ret_val;
    THREAD_RETURN_STATE_E return_state;

    start_time = time_get_current_uptime();

    if(active_thread->function == NULL)
    {
        KERNEL_ERROR("Thread routine cannot be NULL\n");
        KERNEL_PANIC(OS_ERR_UNAUTHORIZED_ACTION);
    }
    ret_val = active_thread->function(active_thread->args);

    /* After this function, all shared data are to be reloaded in case we 
     * forked */    
    return_state = THREAD_RETURN_STATE_RETURNED;

    end_time = time_get_current_uptime();

    sched_get_current_thread()->start_time   = start_time;
    sched_get_current_thread()->end_time     = end_time;
    sched_get_current_thread()->ret_val      = ret_val;
    sched_get_current_thread()->return_state = return_state;

    /* Exit thread properly */
    thread_exit();
}

static void create_main_kprocess(void)
{
    OS_RETURN_E err;

    main_kprocess = kmalloc(sizeof(kernel_process_t));
    if(main_kprocess == NULL)
    {
        KERNEL_ERROR("Could not allocated kernel main process\n");
        KERNEL_PANIC(OS_ERR_MALLOC);
    }

    main_kprocess->parent_process = NULL;
    main_kprocess->pid  = last_given_pid++;
    main_kprocess->children = queue_create_queue(
                                            QUEUE_ALLOCATOR(kmalloc, kfree), 
                                            &err);
    if(err != OS_NO_ERR)
    {
        KERNEL_ERROR("Could not create main kernel process\n");
        KERNEL_PANIC(err);
    }
    main_kprocess->threads = queue_create_queue(QUEUE_ALLOCATOR(kmalloc, kfree), 
                                                &err);
    if(err != OS_NO_ERR)
    {
        KERNEL_ERROR("Could not create main kernel process\n");
        KERNEL_PANIC(err);
    }
    main_kprocess->free_page_table = memory_create_free_page_table(&err);
    if(err != OS_NO_ERR)
    {
        KERNEL_ERROR("Could not create main kernel process\n");
        KERNEL_PANIC(err);
    }
    main_kprocess->page_dir = cpu_get_current_pgdir();
    strncpy(main_kprocess->name, "UTK-Kernel\0", 11);

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
    if(err != OS_NO_ERR)
    {
        KERNEL_ERROR("Could not create IDLE thread[%d]\n", err);
        KERNEL_PANIC(err);
    }
    
    /* Initializes the scheduler active thread */
    idle_thread->state = THREAD_STATE_READY;
    active_thread      = idle_thread;
    active_thread_node = queue_find(
        active_threads_table[idle_thread->priority], 
        idle_thread, 
        &err);

    if(err != OS_NO_ERR)
    {
        KERNEL_ERROR("Could not create IDLE thread[%d]\n", err);
        KERNEL_PANIC(err);
    }
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

    if(err != OS_NO_ERR)
    {
        KERNEL_ERROR("Could not create INIT thread[%d]\n", err);
        KERNEL_PANIC(err);
    }
}

static void select_thread(void)
{
    OS_RETURN_E      err;
    queue_node_t*    sleeping_node;
    uint32_t         i;
    uint64_t         current_time;
    kernel_thread_t* sleeping;
    
    current_time = time_get_current_uptime();

    /* If the thread was not locked */
    if(active_thread->state == THREAD_STATE_RUNNING)
    {
        active_thread->state = THREAD_STATE_READY;
        err = queue_push(active_thread_node, 
                         active_threads_table[active_thread->priority]);
        if(err != OS_NO_ERR)
        {
            KERNEL_ERROR("Could not enqueue old thread[%d]\n", err);
            KERNEL_PANIC(err);
        }
    }
    else if(active_thread->state == THREAD_STATE_SLEEPING)
    {
        err = queue_push_prio(active_thread_node,
                              sleeping_threads_table,
                              active_thread->wakeup_time);
        if(err != OS_NO_ERR)
        {
            KERNEL_ERROR("Could not enqueue old thread[%d]\n", err);
            KERNEL_PANIC(err);
        }
    }

    /* Wake up the sleeping threads */
    do
    {
        sleeping_node = queue_pop(sleeping_threads_table, &err);
        if(err != OS_NO_ERR)
        {
            KERNEL_ERROR("Could not dequeue sleeping thread[%d]\n", err);
            KERNEL_PANIC(err);
        }

        KERNEL_DEBUG(SCHED_ELECT_DEBUG_ENABLED, 
                     "[SCHED] Checking threads to wakeup");

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

            err = queue_push(sleeping_node,
                             active_threads_table[sleeping->priority]);
            if(err != OS_NO_ERR)
            {
                KERNEL_ERROR("Could not enqueue sleeping thread[%d]\n", err);
                KERNEL_PANIC(err);
            }
        }
        else if(sleeping != NULL)
        {
            KERNEL_DEBUG(SCHED_ELECT_DEBUG_ENABLED, 
                         "[SCHED] Sleep %d", 
                         sleeping->tid);
            err = queue_push_prio(sleeping_node,
                                  sleeping_threads_table,
                                  sleeping->wakeup_time);
            if(err != OS_NO_ERR)
            {
                KERNEL_ERROR("Could not enqueue sleeping thread[%d]\n", err);
                KERNEL_PANIC(err);
            }
            break;
        }
    }while(sleeping_node != NULL);

    /* Get the new thread */
    for(i = 0; i < KERNEL_LOWEST_PRIORITY + 1; ++i)
    {
        active_thread_node = queue_pop(active_threads_table[i], &err);
        if(err != OS_NO_ERR)
        {
            KERNEL_ERROR("Could not dequeue next thread[%d]\n", err);
            KERNEL_PANIC(err);
        }

        if(active_thread_node != NULL)
        {            
            break;
        }
    }
    if(active_thread_node == NULL)
    {
        KERNEL_ERROR("Could not dequeue next thread[%d]\n", err);
        KERNEL_PANIC(err);
    }

    if(active_thread == NULL)
    {
        KERNEL_ERROR("Next thread to schedule should not be NULL\n");
        KERNEL_PANIC(err);
    }

    active_thread        = (kernel_thread_t*)active_thread_node->data;
    active_thread->state = THREAD_STATE_RUNNING;
    active_process       = active_thread->process;
    
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

    KERNEL_ERROR("Returned from context restore\n");
    KERNEL_PANIC(OS_ERR_UNAUTHORIZED_ACTION);    
}

SYSTEM_STATE_E sched_get_system_state(void)
{
    return system_state;
}

void sched_init(void)
{
    OS_RETURN_E err;
    uint32_t    i;

    /* Init scheduler settings */
    last_given_tid = 0;
    thread_count   = 0;

    schedule_count   = 0;

    /* Init thread tables */
    for(i = 0; i < KERNEL_LOWEST_PRIORITY + 1; ++i)
    {
        active_threads_table[i] = queue_create_queue(
                                    QUEUE_ALLOCATOR(kmalloc, kfree),
                                    &err);
        if(err != OS_NO_ERR)
        {
            KERNEL_ERROR("Could not create active_threads_table %d [%d]\n",
                         i, 
                         err);
            KERNEL_PANIC(err);
        }
    }

    sleeping_threads_table = queue_create_queue(QUEUE_ALLOCATOR(kmalloc, kfree),
                                                &err);
    if(err != OS_NO_ERR)
    {
        KERNEL_ERROR("Could not create sleeping_threads_table [%d]\n",
                     err);
        KERNEL_PANIC(err);
    }

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
    if(err != OS_NO_ERR)
    {
        KERNEL_ERROR("Could not set scheduler interrupt[%d]\n", err);
        KERNEL_PANIC(err);
    }

    /* Register the scheduler on the main system timer. */
    err = time_register_scheduler(schedule_int);
    if(err != OS_NO_ERR)
    {
        KERNEL_ERROR("Could register scheduler interrupt[%d]\n", err);
        KERNEL_PANIC(err);
    }

    KERNEL_DEBUG(SCHED_DEBUG_ENABLED, "[SCHED] Init scheduler");

    system_state = SYSTEM_STATE_RUNNING;

    cpu_restore_context(NULL, NULL, idle_thread);
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

kernel_thread_t* sched_get_current_thread(void)
{
    if(active_thread == NULL)
    {
        return NULL;
    }
    return active_thread;
}

kernel_process_t* sched_get_current_process(void)
{
    return active_process;
}

int32_t sched_get_tid(void)
{
    if(active_thread == NULL)
    {
        return -1;
    }
    return active_thread->tid;
}

int32_t sched_get_pid(void)
{
    if(active_process == NULL)
    {
        return -1;
    }
    return active_process->pid;
}

int32_t sched_get_ppid(void)
{
    if(active_process == NULL)
    {
        return -1;
    }
    return active_process->parent_process->pid;
}

uint32_t sched_get_priority(void)
{
    if(active_thread == NULL)
    {
        return -1;
    }
    return active_thread->priority;
}

OS_RETURN_E sched_set_priority(const uint32_t priority)
{
    if(active_thread == NULL)
    {
        return OS_ERR_NULL_POINTER;
    }
    
    if(priority > KERNEL_LOWEST_PRIORITY)
    {
        return OS_ERR_FORBIDEN_PRIORITY;
    }

    active_thread->priority = priority;

    return OS_NO_ERR;
}

void sched_set_thread_termination_cause(const THREAD_TERMINATE_CAUSE_E cause)
{
    if(active_thread == NULL)
    {
        return;
    }

    active_thread->return_cause = cause;
}

void sched_terminate_self(void* ret_code)
{
    if(active_thread == NULL)
    {
        return;
    }

    active_thread->ret_val = ret_code;
    active_thread->return_state = THREAD_RETURN_STATE_KILLED;

    active_thread->end_time = time_get_current_uptime();

    /* Exit thread properly */
    thread_exit();
}

uint64_t sched_get_schedule_count(void)
{
    return schedule_count;
}

void sched_fork_process(const SYSCALL_FUNCTION_E func, void* new_pid)
{

    kernel_process_t* new_proc;
    kernel_thread_t*  main_thread;
    queue_node_t*     main_thread_node;
    queue_node_t*     main_thread_node_th;
    queue_node_t*     new_proc_node;
    uint32_t          int_state;
    OS_RETURN_E       err;

    if(func != SYSCALL_FORK)
    {
        if(new_pid != NULL)
        {
            *(int32_t*)new_pid = -1;
        }
    }

    /* Allocate memory for the new process */
    new_proc = kmalloc(sizeof(kernel_process_t));
    if(new_proc == NULL)
    {
        if(new_pid != NULL)
        {
            *(int32_t*)new_pid = -1;
        }
        return;
    }
    memset(new_proc, 0, sizeof(kernel_process_t));

    new_proc_node = queue_create_node(new_proc, 
                                      QUEUE_ALLOCATOR(kmalloc, kfree),
                                      &err);
    if(err != OS_NO_ERR)
    {
        kfree(new_proc);
        if(new_pid != NULL)
        {
            *(int32_t*)new_pid = -1;
        }
        return;
    }

    err = queue_push(new_proc_node, active_process->children);
    if(err != OS_NO_ERR)
    {
        err = queue_delete_node(&new_proc_node);
        if(err != OS_NO_ERR)
        {
            KERNEL_ERROR("Internal error while forking process\n");
            KERNEL_PANIC(err);
        }
        kfree(new_proc);
        if(new_pid != NULL)
        {
            *(int32_t*)new_pid = -1;
        }
        return;
    }

    /* Set the process control block */
    new_proc->children = queue_create_queue(QUEUE_ALLOCATOR(kmalloc, kfree), 
                                            &err);
    if(err != OS_NO_ERR)
    {
        err = queue_remove(active_process->children, new_proc_node);
        err |= queue_delete_node(&new_proc_node);
        if(err != OS_NO_ERR)
        {
            KERNEL_ERROR("Internal error while forking process\n");
            KERNEL_PANIC(err);
        }

        kfree(new_proc);
        if(new_pid != NULL)
        {
            *(int32_t*)new_pid = -1;
        }
        return;
    }
    new_proc->threads = queue_create_queue(QUEUE_ALLOCATOR(kmalloc, kfree), 
                                            &err);
    if(err != OS_NO_ERR)
    {
        err = queue_delete_queue(&new_proc->children);

        err |= queue_remove(active_process->children, new_proc_node);
        err |= queue_delete_node(&new_proc_node);
        if(err != OS_NO_ERR)
        {
            KERNEL_ERROR("Internal error while forking process\n");
            KERNEL_PANIC(err);
        }

        kfree(new_proc);
        if(new_pid != NULL)
        {
            *(int32_t*)new_pid = -1;
        }
        return;
    }    
    strncpy(new_proc->name, 
            active_process->name, 
            THREAD_NAME_MAX_LENGTH);

    /* Create the main process thread */
    main_thread = kmalloc(sizeof(kernel_thread_t));
    if(main_thread == NULL)
    {
        err = queue_delete_queue(&new_proc->children);
        err |= queue_delete_queue(&new_proc->threads);

        err |= queue_remove(active_process->children, new_proc_node);
        err |= queue_delete_node(&new_proc_node);

        if(err != OS_NO_ERR)
        {
            KERNEL_ERROR("Internal error while forking process\n");
            KERNEL_PANIC(err);
        }

        kfree(new_proc);
        if(new_pid != NULL)
        {
            *(int32_t*)new_pid = -1;
        }
        return;
    }
    main_thread->state = THREAD_STATE_COPYING;
    sched_copy_kernel_thread(main_thread);
    main_thread->process = new_proc;

    /* Add the main process thread to the scheduler table and children table */
    main_thread_node = queue_create_node(main_thread, 
                                         QUEUE_ALLOCATOR(kmalloc, kfree),
                                         &err);
    if(err != OS_NO_ERR)
    {
        err = queue_delete_queue(&new_proc->children);
        err |= queue_delete_queue(&new_proc->threads);

        err |= queue_remove(active_process->children, new_proc_node);
        err |= queue_delete_node(&new_proc_node);

        if(err != OS_NO_ERR)
        {
            KERNEL_ERROR("Internal error while forking process\n");
            KERNEL_PANIC(err);
        }

        kfree(main_thread);
        kfree(new_proc);
        if(new_pid != NULL)
        {
            *(int32_t*)new_pid = -1;
        }
        return;
    }  
    err = queue_push(main_thread_node, new_proc->threads);
    if(err != OS_NO_ERR)
    {
        err = queue_delete_queue(&new_proc->children);
        err |= queue_delete_queue(&new_proc->threads);

        err |= queue_delete_node(&main_thread_node);

        err |= queue_remove(active_process->children, new_proc_node);
        err |= queue_delete_node(&new_proc_node);

        if(err != OS_NO_ERR)
        {
            KERNEL_ERROR("Internal error while forking process\n");
            KERNEL_PANIC(err);
        }

        kfree(main_thread);
        kfree(new_proc);
        if(new_pid != NULL)
        {
            *(int32_t*)new_pid = -1;
        }
        return;
    }  

    main_thread_node_th = queue_create_node(main_thread, 
                                            QUEUE_ALLOCATOR(kmalloc, kfree),
                                            &err);
    if(err != OS_NO_ERR)
    {        
        err = queue_remove(new_proc->threads, main_thread_node);

        err |= queue_delete_queue(&new_proc->children);
        err |= queue_delete_queue(&new_proc->threads);

        err |= queue_delete_node(&main_thread_node);

        err |= queue_remove(active_process->children, new_proc_node);
        err |= queue_delete_node(&new_proc_node);

        if(err != OS_NO_ERR)
        {
            KERNEL_ERROR("Internal error while forking process\n");
            KERNEL_PANIC(err);
        }

        kfree(main_thread);
        kfree(new_proc);
        if(new_pid != NULL)
        {
            *(int32_t*)new_pid = -1;
        }
        return;
    }  

    ENTER_CRITICAL(int_state);
    main_thread->state = THREAD_STATE_READY;
    err = queue_push(main_thread_node_th, 
                     active_threads_table[main_thread->priority]);
    if(err != OS_NO_ERR)
    {
        err = queue_remove(new_proc->threads, main_thread_node);

        err |= queue_delete_queue(&new_proc->children);
        err |= queue_delete_queue(&new_proc->threads);

        err |= queue_delete_node(&main_thread_node);
        err |= queue_delete_node(&main_thread_node_th);

        err |= queue_remove(active_process->children, new_proc_node);
        err |= queue_delete_node(&new_proc_node);

        if(err != OS_NO_ERR)
        {
            KERNEL_ERROR("Internal error while forking process\n");
            KERNEL_PANIC(err);
        }

        kfree(main_thread);
        kfree(new_proc);
        if(new_pid != NULL)
        {
            *(int32_t*)new_pid = -1;
        }
        return;
    }  
    /* Update the main thread */
    new_proc->main_thread = main_thread_node_th;

    /* Create new free page table and page directory */
    err = memory_copy_self_mapping(new_proc, 
                                   (void*)active_thread->kstack, 
                                   active_thread->kstack_size);
    if(err != OS_NO_ERR)
    {
        err |= queue_remove(new_proc->threads, main_thread_node);
        err |= queue_remove(active_threads_table[main_thread->priority], 
                            main_thread_node_th);

        err |= queue_delete_queue(&new_proc->children);
        err |= queue_delete_queue(&new_proc->threads);

        err |= queue_delete_node(&main_thread_node);
        err |= queue_delete_node(&main_thread_node_th);

        err |= queue_remove(active_process->children, new_proc_node);
        err |= queue_delete_node(&new_proc_node);

        if(err != OS_NO_ERR)
        {
            KERNEL_ERROR("Internal error while forking process\n");
            KERNEL_PANIC(err);
        }

        kfree(main_thread);
        kfree(new_proc);
        if(new_pid != NULL)
        {
            *(int32_t*)new_pid = -1;
        }
        return;
    } 
    
    new_proc->pid  = last_given_pid++;
    new_proc->parent_process = active_process;

    KERNEL_DEBUG(SCHED_DEBUG_ENABLED, 
                 "[SCHED] Forked current process %d to %d",
                 active_process->pid, 
                 new_proc->pid);

    EXIT_CRITICAL(int_state);

    if(new_pid != NULL)
    {
        *(int32_t*)new_pid = new_proc->pid;
    }    
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
    queue_node_t*    new_thread_node;
    queue_node_t*    new_thread_node_table;
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
    new_thread_node = queue_create_node(new_thread, 
                                        QUEUE_ALLOCATOR(kmalloc, kfree), 
                                        &err);
    if(err != OS_NO_ERR)
    {
        kfree(new_thread);
        return err;
    }

    new_thread_node_table = queue_create_node(new_thread, 
                                              QUEUE_ALLOCATOR(kmalloc, kfree), 
                                              &err);
    if(err != OS_NO_ERR)
    {
        internal_err = queue_delete_node(&new_thread_node);
        if(internal_err != OS_NO_ERR)
        {
            KERNEL_ERROR("Internal error while creating thread\n");
            KERNEL_PANIC(err);
        }
        kfree(new_thread);
        return err;
    }

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

    /* Init thread stack */
    new_thread->kstack = (uintptr_t)memory_alloc_stack(new_thread->kstack_size,
                                                       TRUE,
                                                       &err);
    if(new_thread->kstack == (uintptr_t)NULL || err != OS_NO_ERR)
    {
        KERNEL_ERROR("Could not allocate kernel stack structure\n");
        internal_err = queue_delete_node(&new_thread_node);
        internal_err |= queue_delete_node(&new_thread_node_table);

        if(internal_err != OS_NO_ERR)
        {
            KERNEL_ERROR("Internal error while creating thread\n");
            KERNEL_PANIC(err);
        }

        kfree(new_thread);
        return OS_ERR_MALLOC;
    }

    new_thread->stack = (uintptr_t)memory_alloc_stack(new_thread->stack_size,
                                                      FALSE, 
                                                      &err);
    if(new_thread->stack == (uintptr_t)NULL || err != OS_NO_ERR)
    {
        KERNEL_ERROR("Could not allocate stack structure\n");
        internal_err = queue_delete_node(&new_thread_node);
        internal_err |= queue_delete_node(&new_thread_node_table);
        internal_err |= memory_free_stack((void*)new_thread->kstack, 
                                          new_thread->kstack_size);
        if(internal_err != OS_NO_ERR)
        {
            KERNEL_ERROR("Internal error while creating thread\n");
            KERNEL_PANIC(err);
        }
        kfree(new_thread);
        return OS_ERR_MALLOC;
    }

    cpu_init_thread_context(thread_wrapper, new_thread);

    ENTER_CRITICAL(int_state);

    /* Add the thread to the main kernel process. */
    err = queue_push(new_thread_node, active_process->threads);
    if(err != OS_NO_ERR)
    {
        internal_err = queue_delete_node(&new_thread_node);
        internal_err |= queue_delete_node(&new_thread_node_table);
        internal_err |= memory_free_stack((void*)new_thread->kstack, 
                                          new_thread->kstack_size);
        internal_err |= memory_free_stack((void*)new_thread->stack, 
                                          new_thread->stack_size);

        if(internal_err != OS_NO_ERR)
        {
            KERNEL_ERROR("Internal error while creating thread\n");
            KERNEL_PANIC(err);
        }

        kfree(new_thread);
        KERNEL_ERROR("Could not add thread to process\n");

        EXIT_CRITICAL(int_state);
        return err;
    }
    err = queue_push(new_thread_node_table, 
                     active_threads_table[new_thread->priority]);
    if(err != OS_NO_ERR)
    {
        internal_err = queue_remove(active_process->threads, new_thread_node);
        internal_err |= queue_delete_node(&new_thread_node);
        internal_err |= queue_delete_node(&new_thread_node_table);
        internal_err |= memory_free_stack((void*)new_thread->kstack, 
                                          new_thread->kstack_size);
        internal_err |= memory_free_stack((void*)new_thread->stack, 
                                          new_thread->stack_size);

        if(internal_err != OS_NO_ERR)
        {
            KERNEL_ERROR("Internal error while creating thread\n");
            KERNEL_PANIC(err);
        }

        kfree(new_thread);
        KERNEL_ERROR("Could add thread to proess\n");
        EXIT_CRITICAL(int_state);
        return err;
    }

    KERNEL_DEBUG(SCHED_DEBUG_ENABLED, "[SCHED] Kernel thread created");

    new_thread->tid = last_given_tid++;
    ++thread_count;

    EXIT_CRITICAL(int_state);

    *thread = new_thread;

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

void sched_wait_process_pid(const SYSCALL_FUNCTION_E func, void* params)
{
    uint32_t                 int_state;
    queue_node_t*            child_node;
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
    
    err = queue_remove(active_process->children, child_node);
    if(err != OS_NO_ERR)
    {
        KERNEL_ERROR("Could not unqueue child\n");
        KERNEL_PANIC(err);
    }

    err = queue_delete_node(&child_node);
    if(err != OS_NO_ERR)
    {
        KERNEL_ERROR("Could not clear node child\n");
        KERNEL_PANIC(err);
    }
}

void sched_get_process_params(const SYSCALL_FUNCTION_E func, void* params)
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