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

/* UTK configuration file */
#include <config.h>

/* Test header */
#ifdef TEST_MODE_ENABLED
#include <test_bank.h>
#endif

/* Header file */
#include <scheduler.h>


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

/** @brief Count of the number of times the idle thread was scheduled. */
static volatile uint64_t idle_sched_count;

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
 * FUNCTIONS
 ******************************************************************************/

/**
 * @brief Thread's exit point.
 *
 * @details Exit point of a thread. The function will release the resources of
 * the thread and manage its children (INIT will inherit them). Put the thread
 * in a THREAD_STATE_ZOMBIE state. If an other thread is already joining the
 * active thread, then the joining thread will switch from blocked to ready
 * state.
 */
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
        KERNEL_ERROR("Cannot exit idle or init thread[%d]\n",
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
                EXIT_CRITICAL(int_state);
                KERNEL_ERROR("Could not enqueue joining thread[%d]\n", err);
                KERNEL_PANIC(err);
            }
        }
    }

    /* TODO: Clear process if there no no other thread that are not zombies */

    EXIT_CRITICAL(int_state);

    /* Schedule thread */
    sched_schedule();
}

/**
 * @brief Cleans a thread memory and resources.
 *
 * @details Cleans a thread memory and resources. The thread will be removed
 * from the memory. Before calling this function, the user must ensure the 
 * thread is not used in any place in the system.
 *
 * @param[in] thread The thread to clean.
 */
static void sched_clean_thread(kernel_thread_t* thread)
{
    /* TODO: Release thread,s stack from process if exists */

    kfree(thread);
}

/**
 * @brief Cleans a process memory and resources.
 *
 * @details Cleans a process memory and resources. The process will be removed
 * from the memory. Before calling this function, the user must ensure the 
 * thread is not used in any place in the system.
 *
 * @param[in] thread The thread to clean.
 */
static void sched_clean_process(kernel_process_t* process)
{
    /* TODO: Clean page directory and frames */
    kfree(process);
}

/**
 * @brief Copy the current thread to another kernel thread.
 * 
 * @details Copy the current thread to another kernel thread. The copied thread 
 * will be given a new tid, its state will be set to READY and the new thread 
 * will not inherit the joining thread of the source thread.
 * 
 * @param[out] dst_thread The thread that will receive the copy.
 * 
 * @return OS_NO_ERR is returned on success. Otherwise an error code is 
 * returned.
 */
static OS_RETURN_E sched_copy_kernel_thread(kernel_thread_t* dst_thread)
{
    uint32_t int_state;

    if(dst_thread == NULL)
    {
        return OS_ERR_NULL_POINTER;
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
    

    return OS_NO_ERR;
}                                        

/**
 * @brief Thread routine wrapper.
 *
 * @details Thread launch routine. Wrapper for the actual thread routine. The
 * wrapper will call the thread routine, pass its arguments and gather the
 * return value of the thread function to allow the joining thread to retreive
 * it. Some statistics about the thread might be added in this function.
 */
static void thread_wrapper(void)
{
    active_thread->start_time = time_get_current_uptime();

    if(active_thread->function == NULL)
    {
        KERNEL_ERROR("Thread routine cannot be NULL\n");
        KERNEL_PANIC(OS_ERR_UNAUTHORIZED_ACTION);
    }
    active_thread->ret_val = active_thread->function(active_thread->args);

    active_thread->return_state = THREAD_RETURN_STATE_RETURNED;

    active_thread->end_time = time_get_current_uptime();

    /* Exit thread properly */
    thread_exit();
}

/*******************************************************************************
 * System's specific functions.
 *
 * These are the thread routine used by the kernel to manage the system such as
 * the IDLE or INIT threads.
 ******************************************************************************/

/**
 * @brief IDLE thread routine.
 *
 * @details IDLE thread routine. This thread should always be ready, it is the
 * only thread running when no other trhread are ready. It allows better power
 * consumption management and CPU usage computation.
 *
 * @param[in] args The argument to send to the IDLE thread, usualy null.
 *
 * @warning The IDLE thread routine should never return.
 *
 * @return NULL always, should never return.
 */
static void* idle_sys(void* args)
{
    (void)args;

    KERNEL_INFO("IDLE Started | PID: %d | TID: %d\n", 
                sched_get_pid(), 
                sched_get_tid());

    while(1)
    {
        ++idle_sched_count;

        kernel_interrupt_restore(1);

        if(system_state == SYSTEM_STATE_HALTED)
        {
            KERNEL_INFO("\n -- System HALTED -- ");
            kernel_interrupt_disable();
        }
        cpu_hlt();
    }

    /* If we return better go away and cry in a corner */
    KERNEL_PANIC(OS_ERR_UNAUTHORIZED_ACTION);
    return NULL;
}

/*******************************************************************************
 * System's specific functions END.
 ******************************************************************************/
/**
 * @brief Creates the main kernel process.
 * 
 * @details Creates the main kernel process. 
 * 
 * @return On success OS_NO_ERR is returned. Otherwise an error code is given.
 */
static OS_RETURN_E create_main_kprocess(void)
{
    OS_RETURN_E err;

    main_kprocess = kmalloc(sizeof(kernel_process_t));
    if(main_kprocess == NULL)
    {
        KERNEL_ERROR("Could not allocated kernel main process\n");
        KERNEL_PANIC(OS_ERR_MALLOC);
    }

    main_kprocess->ppid = 0;
    main_kprocess->pid  = last_given_pid++;
    main_kprocess->children = queue_create_queue(
                                            QUEUE_ALLOCATOR(kmalloc, kfree), 
                                            &err);
    if(err != OS_NO_ERR)
    {
        return err;
    }
    main_kprocess->threads = queue_create_queue(QUEUE_ALLOCATOR(kmalloc, kfree), 
                                                &err);
    if(err != OS_NO_ERR)
    {
        queue_delete_queue(&main_kprocess->children);
        return err;
    }
    main_kprocess->free_page_table = memory_create_free_page_table(&err);
    if(err != OS_NO_ERR)
    {
        queue_delete_queue(&main_kprocess->children);
        queue_delete_queue(&main_kprocess->threads);
        return err;
    }
    main_kprocess->page_dir = cpu_get_current_pgdir();
    strncpy(main_kprocess->name, "UTK-Kernel\0", 11);

    active_process = main_kprocess;

    return err;
}

/**
 * @brief Creates the IDLE thread.
 * 
 * @details Creates the IDLE thread for the scheduler.
 * 
 * @return On success OS_NO_ERR is returned. Otherwise an error code is given.
 */
static void create_idle(void)
{
    OS_RETURN_E err;

    err = sched_create_kernel_thread(&idle_thread, 
                                     IDLE_THREAD_PRIORITY, 
                                     "IDLE", 
                                     THREAD_TYPE_KERNEL, 
                                     KERNEL_STACK_SIZE, 
                                     idle_sys, 
                                     NULL);
    if(err != OS_NO_ERR)
    {
        KERNEL_ERROR("Could not create IDLE thread[%d]\n", err);
        KERNEL_PANIC(err);
    }
    
    /* Initializes the scheduler active thread */
    idle_thread->state = SYSTEM_STATE_RUNNING;
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


/**
 * @brief Creates the INIT thread.
 * 
 * @details Creates the INIT thread for the scheduler.
 * 
 * @return On success OS_NO_ERR is returned. Otherwise an error code is given.
 */
static void create_init(void)
{
    OS_RETURN_E err;

    kernel_thread_t* init_thread;

    err = sched_create_kernel_thread(&init_thread, 
                                     KERNEL_HIGHEST_PRIORITY, 
                                     "INIT", 
                                     THREAD_TYPE_KERNEL, 
                                     KERNEL_STACK_SIZE, 
                                     init_sys, 
                                     NULL);

    if(err != OS_NO_ERR)
    {
        KERNEL_ERROR("Could not create INIT thread[%d]\n", err);
        KERNEL_PANIC(err);
    }
}
/**
 * @brief Selects the next thread to be scheduled.
 *
 * @details Selects the next thread to be scheduled. Sets the prev_thread and
 * active_thread pointers. The function will select the next most prioritary
 * thread to be executed. This function also wake up sleeping thread which
 * wake-up time has been reached
 */
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

        KERNEL_DEBUG(SCHED_DEBUG_ENABLED, "[SCHED] Checking threads to wakeup");

        /* If nothing to wakeup */
        if(sleeping_node == NULL)
        {
            break;
        }

        sleeping = (kernel_thread_t*)sleeping_node->data;

        /* If we should wakeup the thread */
        if(sleeping != NULL && sleeping->wakeup_time < current_time)
        {
            KERNEL_DEBUG(SCHED_DEBUG_ENABLED, 
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
            KERNEL_DEBUG(SCHED_DEBUG_ENABLED, 
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
    } while(sleeping_node != NULL);

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

    active_thread = (kernel_thread_t*)active_thread_node->data;
    active_process = active_thread->process;
    
    KERNEL_DEBUG(SCHED_DEBUG_ENABLED, 
                 "[SCHED] Elected new thread: %d", 
                 active_thread->tid);
                 
    active_thread->state = THREAD_STATE_RUNNING;
}

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
static void schedule_int(cpu_state_t* cpu_state, uintptr_t int_id,
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

SYSTEM_STATE_E get_system_state(void)
{
    return system_state;
}

OS_RETURN_E sched_init(void)
{
    OS_RETURN_E err;
    uint32_t    i;

    /* Init scheduler settings */
    last_given_tid = 0;
    thread_count   = 0;

    schedule_count   = 0;
    idle_sched_count = 0;

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
    err = create_main_kprocess();
    if(err != OS_NO_ERR)
    {
        KERNEL_ERROR("Could not create main kernel process[%d]\n", err);
        KERNEL_PANIC(err);
    }

    /* Create idle and init thread */
    create_idle();
    create_init();

    /* Register SW interrupt scheduling */
    err = kernel_interrupt_register_int_handler(SCHEDULER_SW_INT_LINE,
                                                schedule_int);
    if(err != OS_NO_ERR)
    {
        return err;
    }

    /* Register the scheduler on the main system timer. */
    err = time_register_scheduler(schedule_int);
    if(err != OS_NO_ERR)
    {
        return err;
    }

    KERNEL_DEBUG(SCHED_DEBUG_ENABLED, "[SCHED] Init scheduler");

    system_state = SYSTEM_STATE_RUNNING;

    cpu_restore_context(NULL, NULL, idle_thread);

    return OS_NO_ERR;
}

void sched_schedule(void)
{
    /* Raise scheduling interrupt */
    cpu_raise_interrupt(SCHEDULER_SW_INT_LINE);
}

OS_RETURN_E sched_sleep(const unsigned int time_ms)
{
    /* We cannot sleep in idle */
    if(active_thread == idle_thread)
    {
        return OS_ERR_UNAUTHORIZED_ACTION;
    }

    active_thread->wakeup_time = time_get_current_uptime() + time_ms - 1000 / 
                                 KERNEL_MAIN_TIMER_FREQ;
    active_thread->state = THREAD_STATE_SLEEPING;

    KERNEL_DEBUG(SCHED_DEBUG_ENABLED, 
                 "[SCHED] [%d] Thread %d asleep until %d (%dms)", 
                 (uint32_t)time_get_current_uptime(),
                 active_thread->tid,
                 (uint32_t)active_thread->wakeup_time,
                 time_ms);

    sched_schedule();

    return OS_NO_ERR;
}

uint32_t sched_get_thread_count(void)
{
    return thread_count;
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
    return active_process->ppid;
}

uint32_t sched_get_priority(void)
{
    return active_thread->priority;
}

OS_RETURN_E sched_set_priority(const uint32_t priority)
{
    /* Check if priority is free */
    if(priority > KERNEL_LOWEST_PRIORITY)
    {
        return OS_ERR_FORBIDEN_PRIORITY;
    }

    active_thread->priority = priority;

    return OS_NO_ERR;
}

void sched_set_thread_termination_cause(const THREAD_TERMINATE_CAUSE_E cause)
{
    active_thread->return_cause = cause;
}

void sched_terminate_thread(void)
{
    active_thread->return_state = THREAD_RETURN_STATE_KILLED;

    active_thread->end_time = time_get_current_uptime();

    /* Exit thread properly */
    thread_exit();
}

uint64_t sched_get_schedule_count(void)
{
    return schedule_count;
}

uint64_t sched_get_idle_schedule_count(void)
{
    return idle_sched_count;
}

uintptr_t sched_get_thread_phys_pgdir(void)
{
    return cpu_get_current_pgdir();
}


void sched_fork_process(const SYSCALL_FUNCTION_E func, void* new_pid)
{

    kernel_process_t* new_proc;
    kernel_thread_t*  main_thread;
    queue_node_t*     main_thread_node;
    queue_node_t*     main_thread_node_th;
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

    /* Set the process control block */
    new_proc->children = queue_create_queue(QUEUE_ALLOCATOR(kmalloc, kfree), 
                                            &err);
    if(err != OS_NO_ERR)
    {
        sched_clean_process(new_proc);
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
        queue_delete_queue(&new_proc->children);

        sched_clean_process(new_proc);
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
        queue_delete_queue(&new_proc->children);
        queue_delete_queue(&new_proc->threads);

        sched_clean_process(new_proc);
        if(new_pid != NULL)
        {
            *(int32_t*)new_pid = -1;
        }
        return;
    }
    main_thread->state = THREAD_STATE_COPYING;
    err = sched_copy_kernel_thread(main_thread);
    if(err != OS_NO_ERR)
    {
        queue_delete_queue(&new_proc->children);
        queue_delete_queue(&new_proc->threads);

        sched_clean_thread(main_thread);
        sched_clean_process(new_proc);
        if(new_pid != NULL)
        {
            *(int32_t*)new_pid = -1;
        }
        return;
    }  
    main_thread->process = new_proc;

    /* Add the main process thread to the scheduler table and children table */
    main_thread_node = queue_create_node(main_thread, 
                                         QUEUE_ALLOCATOR(kmalloc, kfree),
                                         &err);
    if(err != OS_NO_ERR)
    {
        queue_delete_queue(&new_proc->children);
        queue_delete_queue(&new_proc->threads);

        sched_clean_thread(main_thread);
        sched_clean_process(new_proc);
        if(new_pid != NULL)
        {
            *(int32_t*)new_pid = -1;
        }
        return;
    }  
    err = queue_push(main_thread_node, new_proc->threads);
    if(err != OS_NO_ERR)
    {
        queue_delete_queue(&new_proc->children);
        queue_delete_queue(&new_proc->threads);

        queue_delete_node(&main_thread_node);

        sched_clean_thread(main_thread);
        sched_clean_process(new_proc);
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
        queue_remove(new_proc->threads, main_thread_node);

        queue_delete_queue(&new_proc->children);
        queue_delete_queue(&new_proc->threads);

        queue_delete_node(&main_thread_node);

        sched_clean_thread(main_thread);
        sched_clean_process(new_proc);
        if(new_pid != NULL)
        {
            *(int32_t*)new_pid = -1;
        }
        return;
    }  

    ENTER_CRITICAL(int_state);
    main_thread->state = THREAD_STATE_READY;
    err = queue_push(main_thread_node, 
                     active_threads_table[main_thread->priority]);
    if(err != OS_NO_ERR)
    {
        queue_remove(new_proc->threads, main_thread_node);

        queue_delete_queue(&new_proc->children);
        queue_delete_queue(&new_proc->threads);

        queue_delete_node(&main_thread_node);
        queue_delete_node(&main_thread_node_th);

        sched_clean_thread(main_thread);
        sched_clean_process(new_proc);
        if(new_pid != NULL)
        {
            *(int32_t*)new_pid = -1;
        }
        return;
    }  

    /* Create new free page table and page directory */
    err = memory_copy_self_mapping(new_proc, 
                                   active_thread->kstack, 
                                   active_thread->kstack_size);
    if(err != OS_NO_ERR)
    {
        queue_remove(new_proc->threads, main_thread_node);
        queue_remove(active_threads_table[main_thread->priority], 
                     main_thread_node_th);

        queue_delete_queue(&new_proc->children);
        queue_delete_queue(&new_proc->threads);

        queue_delete_node(&main_thread_node);
        queue_delete_node(&main_thread_node_th);

        sched_clean_thread(main_thread);
        sched_clean_process(new_proc);
        if(new_pid != NULL)
        {
            *(int32_t*)new_pid = -1;
        }
        return;
    } 
    
    new_proc->pid  = last_given_pid++;
    new_proc->ppid = active_process->pid;

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
        queue_delete_node(&new_thread_node);
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
    new_thread->kstack_size  = KERNEL_STACK_SIZE;
    new_thread->stack_size   = stack_size;

    strncpy(new_thread->name, name, THREAD_NAME_MAX_LENGTH);

    /* Init thread stack */
    new_thread->kstack = (uintptr_t)memory_alloc_kstack(
                                                    new_thread->kstack_size);
    if(new_thread->kstack == (uintptr_t)NULL)
    {
        KERNEL_ERROR("Could not allocate kernel stack structure\n");
        queue_delete_node(&new_thread_node);
        queue_delete_node(&new_thread_node_table);
        kfree(new_thread);
        return OS_ERR_MALLOC;
    }
    /* Kernel thread don't use the kernel stack */
    if(type != THREAD_TYPE_KERNEL)
    {
        new_thread->stack = (uintptr_t)memory_alloc_stack(
                                                        new_thread->stack_size);
        if(new_thread->stack == (uintptr_t)NULL)
        {
            KERNEL_ERROR("Could not allocate stack structure\n");
            queue_delete_node(&new_thread_node);
            queue_delete_node(&new_thread_node_table);
            memory_free_kstack(new_thread->kstack, new_thread->kstack_size);
            kfree(new_thread);
            return OS_ERR_MALLOC;
        }
    }
    cpu_init_thread_context(thread_wrapper, new_thread);

    /* Add the thread to the main kernel process. */
    err = queue_push(new_thread_node, active_process->threads);
    if(err != OS_NO_ERR)
    {
        queue_delete_node(&new_thread_node);
        queue_delete_node(&new_thread_node_table);
        memory_free_kstack(new_thread->kstack, new_thread->kstack_size);
        memory_free_stack(new_thread->stack, new_thread->stack_size);
        kfree(new_thread);
        KERNEL_ERROR("Could not add thread to process\n");
        return err;
    }
    err = queue_push(new_thread_node_table, 
                     active_threads_table[new_thread->priority]);
    if(err != OS_NO_ERR)
    {
        queue_remove(active_process->threads, new_thread_node);
        queue_delete_node(&new_thread_node);
        queue_delete_node(&new_thread_node_table);
        memory_free_kstack(new_thread->kstack, new_thread->kstack_size);
        memory_free_stack(new_thread->stack, new_thread->stack_size);
        kfree(new_thread);
        KERNEL_ERROR("Could add thread to proess\n");
        return err;
    }

    KERNEL_DEBUG(SCHED_DEBUG_ENABLED, "[SCHED] Kernel thread created");

    ENTER_CRITICAL(int_state);
    new_thread->tid = last_given_tid++;
    ++thread_count;
    EXIT_CRITICAL(int_state);

    *thread = new_thread;

    return OS_NO_ERR;
}

kernel_process_t* sched_get_current_process(void)
{
    return active_process;
}