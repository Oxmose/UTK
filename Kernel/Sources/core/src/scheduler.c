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
#include <paging.h>             /* Memory management */
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

/** @brief First scheduling flag, tells if it's the first time we schedule a
 * thread.
 */
static volatile uint32_t first_sched;

/** @brief Idle thread handle. */
static kernel_thread_t* idle_thread;
/** @brief Idle thread queue node. */
static queue_node_t*    idle_thread_node;

/** @brief Kernel main process. */
static kernel_process_t main_kprocess;

/** @brief Current active thread handle. */
static kernel_thread_t* active_thread;
/** @brief Current active thread queue node. */
static queue_node_t*    active_thread_node;

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

/** @brief Zombie threads table. */
static queue_t* zombie_threads_table;

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

    /* Enqueue thread in zombie list. */
    err = queue_push(active_thread_node, zombie_threads_table);
    if(err != OS_NO_ERR)
    {
        EXIT_CRITICAL(int_state);

        KERNEL_ERROR("Could not enqueue zombie thread[%d]\n", err);
        KERNEL_PANIC(err);
    }

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

    EXIT_CRITICAL(int_state);

    /* Schedule thread */
    sched_schedule();
}
#if 0
/**
 * @brief Cleans a thread memory and resources.
 *
 * @details Cleans a thread memory and resources. The thread will be removed
 * from the memory. Before calling this function, the user must ensure the 
 * thread is not used in any place in the system.
 *
 * @param[in] thread The thread to clean.
 */
void sched_clean_thread(kernel_thread_t* thread)
{
    /* Release memory */
    kfree((void*)thread->stack);
    kfree((void*)thread->kstack);
    kfree(thread);
}

/**
 * @brief Cleans a joined thread footprint in the system.
 *
 * @details Cleans a thread that is currently being joined by the curent active
 * thread. Removes the thread from all lists and cleans the lists nodes.
 *
 * @param[in] thread The thread to clean.
 */
static void sched_clean_joined_thread(kernel_thread_t* thread)
{
    queue_node_t* node;
    OS_RETURN_E   err;
    uint32_t      int_state;    
    queue_t*      thread_list;

    ENTER_CRITICAL(int_state);

    thread_list = active_thread->process->children;
    /* Remove node from children table */
    node = queue_find(thread_list, thread, &err);
    if(err != OS_NO_ERR)
    {
        KERNEL_ERROR("Could not find joined thread in children table[%d]\n",
                     err);
        KERNEL_PANIC(err);
    }
    err = queue_remove(thread_list, node);
    if(err != OS_NO_ERR)
    {
        EXIT_CRITICAL(int_state);
        KERNEL_ERROR("Could delete thread node in children table[%d]\n",
                     err);
        KERNEL_PANIC(err);
    }
    err = queue_delete_node(&node);
    if(err != OS_NO_ERR)
    {
        EXIT_CRITICAL(int_state);
        KERNEL_ERROR("Could delete thread node[%d]\n", err);
        KERNEL_PANIC(err);
    }

    /* Remove node from zombie table */
    node = queue_find(zombie_threads_table, thread, &err);
    if(err != OS_NO_ERR)
    {
        EXIT_CRITICAL(int_state);
        KERNEL_ERROR("Could not find joined thread in zombie table[%d]\n",
                     err);
        KERNEL_PANIC(err);
    }
    err = queue_remove(zombie_threads_table, node);
    if(err != OS_NO_ERR)
    {
        EXIT_CRITICAL(int_state);
        KERNEL_ERROR("Could delete thread node in zombie table[%d]\n",
                     err);
        KERNEL_PANIC(err);
    }
    err = queue_delete_node(&node);
    if(err != OS_NO_ERR)
    {
        EXIT_CRITICAL(int_state);
        KERNEL_ERROR("Could delete thread node[%d]\n", err);
        KERNEL_PANIC(err);
    }

    KERNEL_DEBUG(SCHED_DEBUG_ENABLED, 
                 "Thread %d joined thread %d",
                 active_thread->tid,
                 thread->tid);

    sched_clean_thread(thread);    

    --thread_count;

    EXIT_CRITICAL(int_state);
}
#endif
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
static OS_RETURN_E create_main_kprocess()
{
    OS_RETURN_E err;

    main_kprocess.ppid = 0;
    main_kprocess.pid  = last_given_pid++;
    main_kprocess.children = queue_create_queue(QUEUE_ALLOCATOR(kmalloc, kfree), 
                                                &err);
    if(err != OS_NO_ERR)
    {
        return err;
    }
    main_kprocess.threads = queue_create_queue(QUEUE_ALLOCATOR(kmalloc, kfree), 
                                                &err);
    if(err != OS_NO_ERR)
    {
        queue_delete_queue(&main_kprocess.children);
        return err;
    }
    main_kprocess.free_page_table = memory_get_kernel_free_pages();
    main_kprocess.page_dir = cpu_get_current_pgdir();
    strncpy(main_kprocess.name, "UTK-Kernel\0", 11);

    return err;
}

/**
 * @brief Creates the IDLE thread.
 * 
 * @details Creates the IDLE thread for the scheduler.
 * 
 * @return On success OS_NO_ERR is returned. Otherwise an error code is given.
 */
static OS_RETURN_E create_idle(void)
{
    OS_RETURN_E   err;
    uint32_t      stack_index;
    queue_node_t* idle_thread_node_list;

    idle_thread = kmalloc(sizeof(kernel_thread_t));
    if(idle_thread == NULL)
    {
        KERNEL_ERROR("Could not allocate IDLE thread structure\n");
        KERNEL_PANIC(OS_ERR_MALLOC);
    }
    idle_thread_node = queue_create_node(idle_thread, 
                                         QUEUE_ALLOCATOR(kmalloc, kfree), 
                                         &err);
    if(err != OS_NO_ERR)
    {
        KERNEL_ERROR("Could not create IDLE thread node\n");
        KERNEL_PANIC(err);
    }
    idle_thread_node_list = queue_create_node(idle_thread, 
                                         QUEUE_ALLOCATOR(kmalloc, kfree), 
                                         &err);
    if(err != OS_NO_ERR )
    {
        KERNEL_ERROR("Could not create IDLE thread node list\n");
        KERNEL_PANIC(err);
    }

    memset(idle_thread, 0, sizeof(kernel_thread_t));

    /* Init thread settings */
    idle_thread->tid         = last_given_tid;
    idle_thread->type        = THREAD_TYPE_KERNEL;    
    idle_thread->priority    = IDLE_THREAD_PRIORITY;
    idle_thread->state       = THREAD_STATE_READY;
    idle_thread->function    = idle_sys;
    idle_thread->kstack_size = SCHEDULER_IDLE_STACK_SIZE;

    strncpy(idle_thread->name, "IDLE\0", 5);

    /* Init thread stack */
    stack_index = (idle_thread->kstack_size + STACK_ALIGN - 1) & 
                  (~(STACK_ALIGN - 1));
    idle_thread->kstack = (uintptr_t)kmalloc(stack_index * sizeof(uint8_t));
    if(idle_thread->kstack == (uintptr_t)NULL)
    {
        KERNEL_ERROR("Could not create IDLE thread stack\n");
        KERNEL_PANIC(OS_ERR_MALLOC);
    }

    cpu_init_thread_context(thread_wrapper, stack_index, idle_thread);

    /* Add the thread to the main kernel process. */
    err = queue_push(idle_thread_node, main_kprocess.threads);
    if(err != OS_NO_ERR)
    {
        KERNEL_ERROR("Could add IDLE thread to kernel main process\n");
        KERNEL_PANIC(err);
    }

    /* Add idle to the list of read ythreads */
    err = queue_push(idle_thread_node_list, 
                     active_threads_table[idle_thread->priority]);
    if(err != OS_NO_ERR)
    {
        KERNEL_ERROR("Could add IDLE thread to kernel main process\n");
        KERNEL_PANIC(err);
    }

    idle_thread->process = &main_kprocess;

    KERNEL_DEBUG(SCHED_DEBUG_ENABLED, "[SCHED] IDLE created");

    ++thread_count;
    ++last_given_tid;

    return OS_NO_ERR;
}


/**
 * @brief Creates the INIT thread.
 * 
 * @details Creates the INIT thread for the scheduler.
 * 
 * @return On success OS_NO_ERR is returned. Otherwise an error code is given.
 */
static OS_RETURN_E create_init(void)
{
    OS_RETURN_E      err;
    uint32_t         stack_index;
    kernel_thread_t* init_thread;
    queue_node_t*    init_thread_node;

    init_thread = kmalloc(sizeof(kernel_thread_t));
    if(init_thread == NULL)
    {
        KERNEL_ERROR("Could not allocate INIT thread structure\n");
        return OS_ERR_MALLOC;
    }
    init_thread_node = queue_create_node(init_thread, 
                                         QUEUE_ALLOCATOR(kmalloc, kfree), 
                                         &err);
    if(err != OS_NO_ERR )
    {
        kfree(init_thread);
        return err;
    }

    memset(init_thread, 0, sizeof(kernel_thread_t));

    /* Init thread settings */
    init_thread->tid         = last_given_tid;
    init_thread->type        = THREAD_TYPE_KERNEL;    
    init_thread->priority    = KERNEL_INIT_PRIORITY;
    init_thread->state       = THREAD_STATE_RUNNING;
    init_thread->function    = init_sys;
    init_thread->kstack_size = KERNEL_STACK_SIZE;

    strncpy(init_thread->name, "INIT\0", 5);

    /* Init thread stack */
    stack_index = (init_thread->kstack_size + STACK_ALIGN - 1) & 
                  (~(STACK_ALIGN - 1));
    init_thread->kstack = (uintptr_t)kmalloc(stack_index * sizeof(uint8_t));
    if(init_thread->kstack == (uintptr_t)NULL)
    {
        KERNEL_ERROR("Could not allocate INIT stack structure\n");
        kfree(init_thread);
        return OS_ERR_MALLOC;
    }

    cpu_init_thread_context(thread_wrapper, stack_index, init_thread);

    /* Add the thread to the main kernel process. */
    err = queue_push(init_thread_node, main_kprocess.threads);
    if(err != OS_NO_ERR)
    {
        KERNEL_ERROR("Could add INIT thread to kernel main process\n");
        return err;
    }
    init_thread->process = &main_kprocess;

    KERNEL_DEBUG(SCHED_DEBUG_ENABLED, "[SCHED] INIT created");

    ++thread_count;
    ++last_given_tid;

    /* Initializes the scheduler active thread */
    active_thread = init_thread;
    active_thread_node = init_thread_node;

    return OS_NO_ERR;
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

    active_thread = (kernel_thread_t*)active_thread_node->data;

    KERNEL_DEBUG(SCHED_DEBUG_ENABLED, 
                 "[SCHED] Elected new thread: %d", 
                 active_thread->tid);

    if(active_thread == NULL)
    {
        KERNEL_ERROR("Next thread to schedule should not be NULL\n");
        KERNEL_PANIC(err);
    }
    active_thread->state = THREAD_STATE_RUNNING;
}

/**
 * @brief Scheduler interrupt handler, executes the conetxt switch.
 *
 * @details Scheduling function, set a new ESP to the pre interrupt cpu context
 * and save the old ESP to the current thread stack. The function will call the
 * select_thread function and then set the CPU registers with the values on the
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

    cpu_save_context(first_sched, cpu_state, stack_state, active_thread);

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

     /* Restore thread context */
    cpu_update_pgdir(active_thread->process->page_dir);
    cpu_restore_context(cpu_state, stack_state, active_thread);

    first_sched = 1;
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

    first_sched = 0;

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

    zombie_threads_table = queue_create_queue(QUEUE_ALLOCATOR(kmalloc, kfree), 
                                              &err);
    if(err != OS_NO_ERR)
    {
        KERNEL_ERROR("Could not create zombie_threads_table[%d]\n", err);
        KERNEL_PANIC(err);
    }

    sleeping_threads_table = queue_create_queue(QUEUE_ALLOCATOR(kmalloc, kfree),
                                                &err);
    if(err != OS_NO_ERR)
    {
        kernel_error("Could not create sleeping_threads_table [%d]\n",
                     err);
        KERNEL_PANIC(err);
    }

    /* Create main kernel process */
    err = create_main_kprocess();
    if(err != OS_NO_ERR)
    {
        kernel_error("Could not create main kernel process[%d]\n", err);
        KERNEL_PANIC(err);
    }

    /* Create idle and init thread */
    err = create_idle();
    if(err != OS_NO_ERR)
    {
        kernel_error("Could not create IDLE thread[%d]\n", err);
        KERNEL_PANIC(err);
    }
    err = create_init();
    if(err != OS_NO_ERR)
    {
        kernel_error("Could not create INIT thread[%d]\n", err);
        KERNEL_PANIC(err);
    }

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
    return active_thread->tid;
}

kernel_thread_t* sched_get_self(void)
{
    if(first_sched == 0)
    {
        return NULL;
    }
    return active_thread;
}

int32_t sched_get_pid(void)
{
    return active_thread->process->pid;
}

int32_t sched_get_ppid(void)
{
    return active_thread->process->ppid;
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

#if 0
OS_RETURN_E sched_fork_process(void)
{

    kernel_process_t* new_proc;
    kernel_thread_t*  main_thread;
    queue_node_t*     main_thread_node;
    queue_node_t*     main_thread_node_th;
    uint32_t          int_state;
    OS_RETURN_E       err;

    /* Allocate memory for the new process */
    new_proc = kmalloc(sizeof(kernel_process_t));
    if(new_proc == NULL)
    {
        return OS_ERR_MALLOC;
    }
    memset(new_proc, 0, sizeof(kernel_process_t));

    /* Create new free page table and page directory */
    err = memory_copy_process_image(&new_proc, 
                                    &active_thread->process);
    if(err != OS_NO_ERR)
    {
        kfree(new_proc);
        return err;
    }

    /* Set the process control block */
    new_proc->children = queue_create_queue(QUEUE_ALLOCATOR(kmalloc, kfree), 
                                            &err);
    if(err != OS_NO_ERR)
    {
        memory_delete_proc_pgdir(new_proc->page_dir);
        memory_delete_proc_free_pgtable(new_proc->free_page_table);
        kfree(new_proc);
        return err;
    }
    new_proc->threads = queue_create_queue(QUEUE_ALLOCATOR(kmalloc, kfree), 
                                            &err);
    if(err != OS_NO_ERR)
    {
        queue_delete_queue(&new_proc->children);

        memory_delete_proc_pgdir(new_proc->page_dir);
        memory_delete_proc_free_pgtable(new_proc->free_page_table);

        kfree(new_proc);
        return err;
    }    
    strncpy(new_proc->name, 
            active_thread->process->name, 
            THREAD_NAME_MAX_LENGTH);

    /* Create the main process thread */
    err = sched_copy_kernel_thread(&main_thread, active_thread);
    if(err != OS_NO_ERR)
    {
        queue_delete_queue(&new_proc->children);
        queue_delete_queue(&new_proc->threads);

        memory_delete_proc_pgdir(new_proc->page_dir);
        memory_delete_proc_free_pgtable(new_proc->free_page_table);

        kfree(new_proc);
        return err;
    }  

    /* Add the main process thread to the scheduler table and children table */
    main_thread_node = queue_create_node(&main_thread, 
                                          QUEUE_ALLOCATOR(kmalloc, kfree),
                                          &err);
    if(err != OS_NO_ERR)
    {
        queue_delete_queue(&new_proc->children);
        queue_delete_queue(&new_proc->threads);

        memory_delete_proc_pgdir(new_proc->page_dir);
        memory_delete_proc_free_pgtable(new_proc->free_page_table);

        sched_delete_thread(main_thread);
        kfree(new_proc);
        return err;
    }  
    err = queue_push(main_thread_node, new_proc->threads);
    if(err != OS_NO_ERR)
    {
        queue_delete_queue(&new_proc->children);
        queue_delete_queue(&new_proc->threads);

        queue_delete_node(&main_thread_node);

        memory_delete_proc_pgdir(new_proc->page_dir);
        memory_delete_proc_free_pgtable(new_proc->free_page_table);

        sched_delete_thread(main_thread);
        kfree(new_proc);
        return err;
    }  

    main_thread_node_th = queue_create_node(&main_thread, 
                                            QUEUE_ALLOCATOR(kmalloc, kfree),
                                            &err);
    if(err != OS_NO_ERR)
    {        
        queue_remove(new_proc->threads, main_thread_node);

        queue_delete_queue(&new_proc->children);
        queue_delete_queue(&new_proc->threads);

        queue_delete_node(&main_thread_node);

        memory_delete_proc_pgdir(new_proc->page_dir);
        memory_delete_proc_free_pgtable(new_proc->free_page_table);

        sched_delete_thread(main_thread);
        kfree(new_proc);
        return err;
    }  
    err = queue_push(main_thread_node, 
                     active_threads_table[main_thread->priority]);
    if(err != OS_NO_ERR)
    {
        queue_remove(new_proc->threads, main_thread_node);

        queue_delete_queue(&new_proc->children);
        queue_delete_queue(&new_proc->threads);

        queue_delete_node(&main_thread_node);
        queue_delete_node(&main_thread_node_th);

        memory_delete_proc_pgdir(new_proc->page_dir);
        memory_delete_proc_free_pgtable(new_proc->free_page_table);

        sched_delete_thread(main_thread);
        kfree(new_proc);
        return err;
    }  

    if(err != OS_NO_ERR)
    {
        queue_remove(new_proc->threads, main_thread_node);
        queue_remove(active_threads_table[main_thread->priority], 
                     main_thread_node_th);

        queue_delete_queue(&new_proc->children);
        queue_delete_queue(&new_proc->threads);

        queue_delete_node(&main_thread_node);
        queue_delete_node(&main_thread_node_th);

        memory_delete_proc_pgdir(new_proc->page_dir);
        memory_delete_proc_free_pgtable(new_proc->free_page_table);

        sched_delete_thread(main_thread);
        kfree(new_proc);
        return err;
    } 

    ENTER_CRITICAL(int_state);
    new_proc->pid = last_given_pid++;
    EXIT_CRITICAL(int_state);

    return OS_NO_ERR:
}
#endif