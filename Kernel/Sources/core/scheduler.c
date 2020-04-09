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


#include <lib/stdint.h>           /* Generic int types */
#include <lib/stddef.h>           /* Standard definitions */
#include <lib/string.h>           /* String manipulation */
#include <lib/stdlib.h>           /* Standard library */
#include <memory/kheap.h>         /* Kernel heap */
#include <memory/memalloc.h>      /* Memory allocation*/
#include <memory/paging.h>        /* Memory management */
#include <cpu.h>                  /* CPU management */
#include <core/panic.h>           /* Kernel panic */
#include <interrupt/interrupts.h> /* Interrupt management */
#include <interrupt_settings.h>   /* Interrupt settings */
#include <io/kernel_output.h>     /* Kernel output methods */
#include <io/graphic.h>           /* Graphic API */
#include <core/kernel_queue.h>    /* Kernel queues */
#include <sync/critical.h>        /* Critical sections */
#include <time/time_management.h> /* Timers factory */

/* UTK configuration file */
#include <config.h>

/* Test header */
#if TEST_MODE_ENABLED == 1
#include <Tests/test_bank.h>
#endif

/* Header file */
#include <core/scheduler.h>

 /*******************************************************************************
 * GLOBAL VARIABLES
 ******************************************************************************/
/** @brief Main CPU ID. */
extern int32_t main_core_id;

/** @brief The last TID given by the kernel. */
static volatile uint32_t last_given_tid;
/** @brief The number of thread in the system (dead threads are not accounted).
 */
static volatile uint32_t thread_count;
/** @brief First scheduling flag, tells if it's the first time we schedule a
 * thread.
 */
static volatile uint32_t first_sched[MAX_CPU_COUNT];

/** @brief Idle thread handle. */
static kernel_thread_t*     idle_thread[MAX_CPU_COUNT];
/** @brief Idle thread queue node. */
static kernel_queue_node_t* idle_thread_node[MAX_CPU_COUNT];

/** @brief Init thread handle. */
static kernel_thread_t*     init_thread;
/** @brief Init thread queue node. */
static kernel_queue_node_t* init_thread_node;

/** @brief Current active thread handle. */
static kernel_thread_t*     active_thread[MAX_CPU_COUNT];
/** @brief Current active thread queue node. */
static kernel_queue_node_t* active_thread_node[MAX_CPU_COUNT];
/** @brief Previously active thread handle. */
static kernel_thread_t*     prev_thread[MAX_CPU_COUNT];
/** @brief Previously active thread queue node. */
static kernel_queue_node_t* prev_thread_node[MAX_CPU_COUNT];

#if MAX_CPU_COUNT > 1
/** @brief Per CPU lock */
static spinlock_t cpu_locks[MAX_CPU_COUNT];

/** @brief Global scheduler lock. */
static spinlock_t sched_lock;
#endif

/** @brief Current system state. */
static volatile SYSTEM_STATE_E system_state;

/** @brief Count of the number of times the scheduler was called. */
static volatile uint64_t schedule_count[MAX_CPU_COUNT];

/** @brien Count of the number of times the idle thread was scheduled. */
static volatile uint64_t idle_sched_count[MAX_CPU_COUNT];

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
static kernel_queue_t* active_threads_table[MAX_CPU_COUNT]
                                           [KERNEL_LOWEST_PRIORITY + 1];

/** @brief Sleeping threads table. The threads are sorted by their wakeup time
 * value.
 */
static kernel_queue_t* sleeping_threads_table[MAX_CPU_COUNT];

/** @brief Zombie threads table. */
static kernel_queue_t* zombie_threads_table;

/** @brief Global thread table. */
static kernel_queue_t* global_threads_table;

/** @brief Extern user program entry point. */
extern int main(int, char**);

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
    OS_RETURN_E          err;
    kernel_thread_t*     joining_thread = NULL;
    kernel_queue_node_t* node;
    uint32_t             int_state;
    int32_t              cpu_id;

#if MAX_CPU_COUNT > 1
    uint32_t lock;
#endif

    cpu_id = cpu_get_id();
    if(cpu_id == -1)
    {
        cpu_id = 0;
    }

#if SCHED_KERNEL_DEBUG == 1
    kernel_serial_debug("Exit thread %d\n", active_thread[cpu_id]->tid);
#endif

    /* Cannot exit idle thread */
    if(active_thread[cpu_id] == idle_thread[cpu_id])
    {
        kernel_error("Cannot exit idle thread[%d]\n",
                     OS_ERR_UNAUTHORIZED_ACTION);
        kernel_panic(OS_ERR_UNAUTHORIZED_ACTION);
    }

    /* Set new thread state */
    active_thread[cpu_id]->state = THREAD_STATE_ZOMBIE;

    if(active_thread[cpu_id] == init_thread)
    {
        /* Schedule thread, should never return since the state is zombie */
        sched_schedule();

        return;
    }

#if MAX_CPU_COUNT > 1
    ENTER_CRITICAL(int_state, &cpu_locks[cpu_id]);
#else
    ENTER_CRITICAL(int_state);
#endif

    /* Enqueue thread in zombie list. */
    err = kernel_queue_push(active_thread_node[cpu_id], zombie_threads_table);
    if(err != OS_NO_ERR)
    {
#if MAX_CPU_COUNT > 1
        EXIT_CRITICAL(int_state, &cpu_locks[cpu_id]);
#else
        EXIT_CRITICAL(int_state);
#endif

        kernel_error("Could not enqueue zombie thread[%d]\n", err);
        kernel_panic(err);
    }

    /* All the children of the thread are inherited by init */
    node = kernel_queue_pop(active_thread[cpu_id]->children, &err);
    while(node != NULL && err == OS_NO_ERR)
    {
        kernel_thread_t* thread = (kernel_thread_t*)node->data;

        thread->ptid = init_thread->tid;

        if(thread->joining_thread->data == active_thread[cpu_id])
        {
            thread->joining_thread->data = NULL;
        }

        err = kernel_queue_push(node, init_thread->children);
        if(err != OS_NO_ERR)
        {
#if MAX_CPU_COUNT > 1
            EXIT_CRITICAL(int_state, &cpu_locks[cpu_id]);
#else
            EXIT_CRITICAL(int_state);
#endif

            kernel_error("Could not enqueue thread to init[%d]\n", err);
            kernel_panic(err);
        }

        node = kernel_queue_pop(active_thread[cpu_id]->children, &err);
    }
    if(err != OS_NO_ERR)
    {
#if MAX_CPU_COUNT > 1
        EXIT_CRITICAL(int_state, &cpu_locks[cpu_id]);
#else
        EXIT_CRITICAL(int_state);
#endif
        kernel_error("Could not dequeue thread from children[%d]\n", err);
        kernel_panic(err);
    }

    /* Delete list */
    err = kernel_queue_delete_queue(&active_thread[cpu_id]->children);
    if(err != OS_NO_ERR)
    {
#if MAX_CPU_COUNT > 1
        EXIT_CRITICAL(int_state, &cpu_locks[cpu_id]);
#else
        EXIT_CRITICAL(int_state);
#endif
        kernel_error("Could not delete list of children[%d]\n", err);
        kernel_panic(err);
    }

    /* Search for joining thread */
    if(active_thread[cpu_id]->joining_thread != NULL)
    {
        joining_thread =
            (kernel_thread_t*)active_thread[cpu_id]->joining_thread->data;
    }

    /* Wake up joining thread */
    if(joining_thread != NULL)
    {
#if MAX_CPU_COUNT > 1
        ENTER_CRITICAL(lock, &joining_thread->lock);
#endif

        if(joining_thread->state == THREAD_STATE_JOINING)
        {
#if SCHED_KERNEL_DEBUG == 1
            kernel_serial_debug("Woke up joining thread %d\n",
                joining_thread->tid);
#endif

            joining_thread->state = THREAD_STATE_READY;

            err = kernel_queue_push(active_thread[cpu_id]->joining_thread,
                        active_threads_table[joining_thread->cpu_affinity]
                                            [joining_thread->priority]);
            if(err != OS_NO_ERR)
            {
#if MAX_CPU_COUNT > 1
                EXIT_CRITICAL(lock, &joining_thread->lock);
#endif

#if MAX_CPU_COUNT > 1
                EXIT_CRITICAL(int_state, &cpu_locks[cpu_id]);
#else
                EXIT_CRITICAL(int_state);
#endif
                kernel_error("Could not enqueue joining thread[%d]\n", err);
                kernel_panic(err);
            }
        }

#if MAX_CPU_COUNT > 1
        EXIT_CRITICAL(lock, &joining_thread->lock);
#endif
    }

#if MAX_CPU_COUNT > 1
    EXIT_CRITICAL(int_state, &cpu_locks[cpu_id]);
#else
    EXIT_CRITICAL(int_state);
#endif

    /* Schedule thread */
    sched_schedule();
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
    kernel_queue_node_t* node;
    OS_RETURN_E          err;
    uint32_t             int_state;    
    int32_t              cpu_id;

#if MAX_CPU_COUNT > 1
    uint32_t secint_state;
#endif

    cpu_id = cpu_get_id();
    if(cpu_id == -1)
    {
        cpu_id = 0;
    }

#if MAX_CPU_COUNT > 1
    ENTER_CRITICAL(int_state, &cpu_locks[cpu_id]);
#else
    ENTER_CRITICAL(int_state);
#endif

    /* Remove node from children table */
    node = kernel_queue_find(active_thread[cpu_id]->children, thread, &err);
    if(err != OS_NO_ERR && err != OS_ERR_NO_SUCH_ID)
    {
#if MAX_CPU_COUNT > 1
        EXIT_CRITICAL(int_state, &cpu_locks[cpu_id]);
#else
        EXIT_CRITICAL(int_state);
#endif

        kernel_error("Could not find joined thread in children table[%d]\n",
                     err);
        kernel_panic(err);
    }

    if(node != NULL && err == OS_NO_ERR)
    {
        err = kernel_queue_remove(active_thread[cpu_id]->children, node);
        if(err != OS_NO_ERR)
        {
#if MAX_CPU_COUNT > 1
            EXIT_CRITICAL(int_state, &cpu_locks[cpu_id]);
#else
            EXIT_CRITICAL(int_state);
#endif

            kernel_error("Could delete thread node in children table[%d]\n",
                         err);
            kernel_panic(err);
        }

        err = kernel_queue_delete_node(&node);
        if(err != OS_NO_ERR)
        {
#if MAX_CPU_COUNT > 1
            EXIT_CRITICAL(int_state, &cpu_locks[cpu_id]);
#else
            EXIT_CRITICAL(int_state);
#endif

            kernel_error("Could delete thread node[%d]\n", err);
            kernel_panic(err);
        }
    }

    /* Remove node from zombie table */

    node = kernel_queue_find(zombie_threads_table, thread, &err);
    if(err != OS_NO_ERR && err != OS_ERR_NO_SUCH_ID)
    {
#if MAX_CPU_COUNT > 1
        EXIT_CRITICAL(int_state, &cpu_locks[cpu_id]);
#else
        EXIT_CRITICAL(int_state);
#endif

        kernel_error("Could not find joined thread in zombie table[%d]\n",
                     err);
        kernel_panic(err);
    }

    if(node != NULL && err == OS_NO_ERR)
    {
        err = kernel_queue_remove(zombie_threads_table, node);
        if(err != OS_NO_ERR)
        {
#if MAX_CPU_COUNT > 1
            EXIT_CRITICAL(int_state, &cpu_locks[cpu_id]);
#else
            EXIT_CRITICAL(int_state);
#endif

            kernel_error("Could delete thread node in zombie table[%d]\n",
                         err);
            kernel_panic(err);
        }

        err = kernel_queue_delete_node(&node);
        if(err != OS_NO_ERR)
        {
#if MAX_CPU_COUNT > 1
            EXIT_CRITICAL(int_state, &cpu_locks[cpu_id]);
#else
            EXIT_CRITICAL(int_state);
#endif

            kernel_error("Could delete thread node[%d]\n", err);
            kernel_panic(err);
        }
    }

    /* Remove node from general table */
    node = kernel_queue_find(global_threads_table, thread, &err);
    if(err != OS_NO_ERR && err != OS_ERR_NO_SUCH_ID)
    {
#if MAX_CPU_COUNT > 1
        EXIT_CRITICAL(int_state, &cpu_locks[cpu_id]);
#else
        EXIT_CRITICAL(int_state);
#endif

        kernel_error("Could not find joined thread in general table[%d]\n",
                     err);
        kernel_panic(err);
    }
    if(node != NULL && err == OS_NO_ERR)
    {
        err = kernel_queue_remove(global_threads_table, node);
        if(err != OS_NO_ERR)
        {
#if MAX_CPU_COUNT > 1
            EXIT_CRITICAL(int_state, &cpu_locks[cpu_id]);
#else
            EXIT_CRITICAL(int_state);
#endif

            kernel_error("Could delete thread node in general table[%d]\n",
                         err);
            kernel_panic(err);
        }

        err = kernel_queue_delete_node(&node);
        if(err != OS_NO_ERR)
        {
#if MAX_CPU_COUNT > 1
            EXIT_CRITICAL(int_state, &cpu_locks[cpu_id]);
#else
            EXIT_CRITICAL(int_state);
#endif

            kernel_error("Could delete thread node[%d]\n", err);
            kernel_panic(err);
        }
    }

#if SCHED_KERNEL_DEBUG == 1
    kernel_serial_debug("Thread %d joined thread %d\n",
                         active_thread[cpu_id]->tid,
                         thread->tid);
#endif

    /* Free the thread stack */
    kfree(thread->stack);
    kfree(thread);

#if MAX_CPU_COUNT > 1
    ENTER_CRITICAL(secint_state, &sched_lock);
    --thread_count;
    EXIT_CRITICAL(secint_state, &sched_lock);
#else 
    --thread_count;
#endif

#if MAX_CPU_COUNT > 1
    EXIT_CRITICAL(int_state, &cpu_locks[cpu_id]);
#else
    EXIT_CRITICAL(int_state);
#endif
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
    int32_t cpu_id;

    cpu_id = cpu_get_id();
    if(cpu_id == -1)
    {
        cpu_id = 0;
    }

    active_thread[cpu_id]->start_time = time_get_current_uptime();

    if(active_thread[cpu_id]->function == NULL)
    {
        kernel_error("Thread routine cannot be NULL\n");
        kernel_panic(OS_ERR_UNAUTHORIZED_ACTION);
    }
    active_thread[cpu_id]->ret_val =
        active_thread[cpu_id]->function(active_thread[cpu_id]->args);

    active_thread[cpu_id]->return_state = THREAD_RETURN_STATE_RETURNED;

    active_thread[cpu_id]->end_time = time_get_current_uptime();

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
 * @brief Scheduler's main function kickstarter.
 *
 * @details Main kickstarter, just call the main. This is used to start main in
 * its own thread.
 *
 * @param[in] args The arguments for the main thread. Unsued at the moment.
 *
 * @return The main return state.
 */
static void* main_kickstart(void* args)
{
    (void)args;
    char* argv[2] = {"main", NULL};

    /* Call main */
    int32_t ret = main(1, argv);

    return (void*)(uintptr_t)ret;
}

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
    int32_t  cpu_id;

    cpu_id = cpu_get_id();
    if(cpu_id == -1)
    {
        cpu_id = 0;
    }

#if SCHED_KERNEL_DEBUG == 1
    kernel_serial_debug("IDLE Started\n");
#endif

    (void)args;

    /* Halt forever, cpu_hlt for energy consumption */
    while(1)
    {
        ++idle_sched_count[cpu_id];

        kernel_interrupt_restore(1);

        if(cpu_id == main_core_id && system_state == SYSTEM_STATE_HALTED)
        {
            if(cpu_id == MAX_CPU_COUNT - 1)
            {
                kernel_printf("\n");
                kernel_info(" -- System HALTED -- ");
            }
            kernel_interrupt_disable();
            ++main_core_id;
        }
        cpu_hlt();
    }

    /* If we return better go away and cry in a corner */
    return NULL;
}

/** 
 * @details INIT thread routine. In addition to the IDLE thread, the INIT thread is the
 * last thread to run. The thread will gather all orphan thread and wait for
 * their death before halting the system. The INIT thread routine is also
 * responsible for calling the main function.
 *
 * @param args The argument to send to the INIT thread, usualy null.
 * @return NULL always
 */
static void* init_func(void* args)
{
    kernel_thread_t*     thread;
    kernel_thread_t*     main_thread;
    kernel_queue_node_t* thread_node;
    uint32_t             sys_thread;
    OS_RETURN_E          err;
    uint32_t             int_state;
    int32_t              cpu_id;

    cpu_id = cpu_get_id();
    if(cpu_id == -1)
    {
        cpu_id = 0;
    }

    (void)args;

#if SCHED_KERNEL_DEBUG == 1
    kernel_serial_debug("INIT Started\n");
#endif


#if TEST_MODE_ENABLED == 1
    scheduler_load_test();
    scheduler_load_mc_test();
    scheduler_preemt_test();
    scheduler_sleep_test();
    scheduler_sleep_mc_test();
    critical_test();
    div_by_zero_test();
    mutex_test();
    mutex_mc_test();
    semaphore_test();
    semaphore_mc_test();
    mailbox_test();
    userqueue_test();
    spinlock_test();
    sse_test();
    while(1)
    {
        sched_sleep(10000000);
    }
#endif

    err = sched_create_kernel_thread(&main_thread, KERNEL_HIGHEST_PRIORITY,
                                     "main", SCHEDULER_MAIN_STACK_SIZE,
                                     0, main_kickstart,
                                     (void*)1);
    if(err != OS_NO_ERR)
    {
        kernel_error("Cannot kickstart main, aborting [%d]\n", err);
        kernel_panic(err);
    }

    err = sched_wait_thread(main_thread, NULL, NULL);
    if(err != OS_NO_ERR)
    {
        kernel_error("Cannot wait main, aborting [%d]\n", err);
        kernel_panic(err);
    }

#if SCHED_KERNEL_DEBUG == 1
    kernel_serial_debug("Main returned, INIT waiting for children\n");
#endif

    /* System thread are idle threads and INIT */
    sys_thread = 1 + MAX_CPU_COUNT;

#if MAX_CPU_COUNT > 1
    ENTER_CRITICAL(int_state, &cpu_locks[cpu_id]);
#else
    ENTER_CRITICAL(int_state);
#endif

    /* Wait all children */
    while(thread_count > sys_thread)
    {

        thread_node = kernel_queue_pop(active_thread[cpu_id]->children, &err);

        while(thread_node != NULL && err == OS_NO_ERR)
        {

            thread = (kernel_thread_t*)thread_node->data;

#if MAX_CPU_COUNT > 1
            EXIT_CRITICAL(int_state, &cpu_locks[cpu_id]);
#else
            EXIT_CRITICAL(int_state);
#endif

            err = sched_wait_thread(thread, NULL, NULL);
            if(err != OS_NO_ERR)
            {
#if MAX_CPU_COUNT > 1
                EXIT_CRITICAL(int_state, &cpu_locks[cpu_id]);
#else
                EXIT_CRITICAL(int_state);
#endif

                kernel_error("Error while waiting thread in INIT [%d]\n", err);
                kernel_panic(err);
            }

#if MAX_CPU_COUNT > 1
            ENTER_CRITICAL(int_state, &cpu_locks[cpu_id]);
#else
            ENTER_CRITICAL(int_state);
#endif

            err = kernel_queue_delete_node(&thread_node);
            if(err != OS_NO_ERR)
            {
#if MAX_CPU_COUNT > 1
            EXIT_CRITICAL(int_state, &cpu_locks[cpu_id]);
#else
            EXIT_CRITICAL(int_state);
#endif

                kernel_error("Error while deleting thread node in INIT [%d]\n",
                             err);
                kernel_panic(err);
            }
            thread_node = kernel_queue_pop(active_thread[cpu_id]->children, &err);
        }
    }

#if MAX_CPU_COUNT > 1
    EXIT_CRITICAL(int_state, &cpu_locks[cpu_id]);
#else
    EXIT_CRITICAL(int_state);
#endif

#if SCHED_KERNEL_DEBUG == 1
    kernel_serial_debug("INIT Ended\n");
#endif

    /* If here, the system is halted */
    system_state = SYSTEM_STATE_HALTED;

    return NULL;
}

/*******************************************************************************
 * System's specific functions END.
 ******************************************************************************/

static OS_RETURN_E create_idle(const uint32_t idle_stack_size)
{
    OS_RETURN_E          err;
    kernel_queue_node_t* second_idle_thread_node;
    char                 idle_name[5] = "Idle\0";
    uint32_t             stack_index;
    int32_t              cpu_id;

    /* Get CPU ID */
    cpu_id = cpu_get_id();
    if(cpu_id == -1)
    {
        cpu_id = 0;
    }

    idle_thread[cpu_id] = kmalloc(sizeof(kernel_thread_t));
    idle_thread_node[cpu_id] =
        kernel_queue_create_node(idle_thread[cpu_id], &err);

    if(err != OS_NO_ERR ||
       idle_thread[cpu_id] == NULL ||
       idle_thread_node[cpu_id] == NULL)
    {
        if(idle_thread[cpu_id] != NULL)
        {
            kfree(idle_thread[cpu_id]);
        }
        else
        {
            err = OS_ERR_MALLOC;
        }
        return err;
    }

    memset(idle_thread[cpu_id], 0, sizeof(kernel_thread_t));

    /* Init thread settings */
    idle_thread[cpu_id]->tid            = last_given_tid;
    idle_thread[cpu_id]->ptid           = last_given_tid;
    idle_thread[cpu_id]->priority       = IDLE_THREAD_PRIORITY;
    idle_thread[cpu_id]->init_prio      = IDLE_THREAD_PRIORITY;
    idle_thread[cpu_id]->args           = 0;
    idle_thread[cpu_id]->function       = idle_sys;
    idle_thread[cpu_id]->joining_thread = NULL;
    idle_thread[cpu_id]->state          = THREAD_STATE_RUNNING;
    idle_thread[cpu_id]->stack_size     = idle_stack_size;
    idle_thread[cpu_id]->cpu_affinity   = cpu_id;

#if MAX_CPU_COUNT > 1
    INIT_SPINLOCK(&idle_thread[cpu_id]->lock);
#endif

    idle_thread[cpu_id]->children = kernel_queue_create_queue(&err);
    if(err != OS_NO_ERR)
    {
        kfree(idle_thread[cpu_id]);
        return err;
    }

    /* Init thread stack */
    stack_index = (idle_stack_size + ALIGN - 1) & (~(ALIGN - 1));
    stack_index /= sizeof(uintptr_t);
    idle_thread[cpu_id]->stack = kmalloc(stack_index * sizeof(uintptr_t));
    if(idle_thread[cpu_id]->stack == NULL)
    {
        kfree(idle_thread[cpu_id]);
        return OS_ERR_MALLOC;
    }

    cpu_init_thread_context(thread_wrapper, stack_index, 
                            (uintptr_t)kernel_free_pages, 
                            cpu_get_current_pgdir(), idle_thread[cpu_id]);

    strncpy(idle_thread[cpu_id]->name, idle_name, 5);

#if SCHED_KERNEL_DEBUG == 1
    kernel_serial_debug("IDLE thread created\n");
#endif

    second_idle_thread_node =
        kernel_queue_create_node(idle_thread[cpu_id], &err);
    if(err != OS_NO_ERR || second_idle_thread_node == NULL)
    {
        kfree(idle_thread[cpu_id]->stack);
        kfree(idle_thread[cpu_id]);
        return err;
    }

    err = kernel_queue_push(second_idle_thread_node, global_threads_table);
    if(err != OS_NO_ERR)
    {
        kfree(idle_thread[cpu_id]->stack);
        kfree(idle_thread[cpu_id]);
        return err;
    }

    ++thread_count;
    ++last_given_tid;

    /* Initializes the scheduler active thread */
    active_thread[cpu_id] = idle_thread[cpu_id];
    active_thread_node[cpu_id] = idle_thread_node[cpu_id];
    prev_thread[cpu_id] = active_thread[cpu_id];
    prev_thread_node[cpu_id] = active_thread_node[cpu_id];

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
    OS_RETURN_E          err;
    kernel_queue_node_t* sleeping_node;
    uint32_t             i;
    uint64_t             current_time = time_get_current_uptime();
    int32_t              cpu_id;

    cpu_id = cpu_get_id();
    if(cpu_id == -1)
    {
        cpu_id = 0;
    }
    /* Switch running thread */
    prev_thread[cpu_id] = active_thread[cpu_id];
    prev_thread_node[cpu_id] = active_thread_node[cpu_id];

    /* If the thread was not locked */
    if(prev_thread[cpu_id]->state == THREAD_STATE_RUNNING)
    {
        prev_thread[cpu_id]->state = THREAD_STATE_READY;
        err = kernel_queue_push(prev_thread_node[cpu_id],
                active_threads_table[cpu_id][prev_thread[cpu_id]->priority]);
        if(err != OS_NO_ERR)
        {
            kernel_error("Could not enqueue old thread[%d]\n", err);
            kernel_panic(err);
        }
    }
    else if(prev_thread[cpu_id]->state == THREAD_STATE_SLEEPING)
    {
        err = kernel_queue_push_prio(prev_thread_node[cpu_id],
                                     sleeping_threads_table[cpu_id],
                                     prev_thread[cpu_id]->wakeup_time);
        if(err != OS_NO_ERR)
        {
            kernel_error("Could not enqueue old thread[%d]\n", err);
            kernel_panic(err);
        }
    }

    /* Wake up the sleeping threads */
    do
    {
        kernel_thread_t* sleeping;

        sleeping_node = kernel_queue_pop(sleeping_threads_table[cpu_id], &err);
        if(err != OS_NO_ERR)
        {
            kernel_error("Could not dequeue sleeping thread[%d]\n", err);
            kernel_panic(err);
        }
#if SCHED_KERNEL_DEBUG == 1
    kernel_serial_debug("Checking threads to wakeup\n");
#endif

        /* If nothing to wakeup */
        if(sleeping_node == NULL)
        {
            break;
        }

        sleeping = (kernel_thread_t*)sleeping_node->data;

        /* If we should wakeup the thread */
        if(sleeping != NULL && sleeping->wakeup_time < current_time)
        {
#if SCHED_KERNEL_DEBUG == 1
    kernel_serial_debug("Waking up %d\n", sleeping->tid);
#endif

            sleeping->state = THREAD_STATE_READY;

            err = kernel_queue_push(sleeping_node,
                            active_threads_table[cpu_id][sleeping->priority]);
            if(err != OS_NO_ERR)
            {
                kernel_error("Could not enqueue sleeping thread[%d]\n", err);
                kernel_panic(err);
            }
        }
        else if(sleeping != NULL)
        {
#if SCHED_KERNEL_DEBUG == 1
    kernel_serial_debug("Keeping sleep %d\n", sleeping->tid);
#endif
            err = kernel_queue_push_prio(sleeping_node,
                                         sleeping_threads_table[cpu_id],
                                         sleeping->wakeup_time);
            if(err != OS_NO_ERR)
            {
                kernel_error("Could not enqueue sleeping thread[%d]\n", err);
                kernel_panic(err);
            }
            break;
        }
    } while(sleeping_node != NULL);

    /* Get the new thread */
    for(i = 0; i < KERNEL_LOWEST_PRIORITY + 1; ++i)
    {
        active_thread_node[cpu_id] =
            kernel_queue_pop(active_threads_table[cpu_id][i], &err);
        if(err != OS_NO_ERR)
        {
            kernel_error("Could not dequeue next thread[%d]\n", err);
            kernel_panic(err);
        }
#if SCHED_KERNEL_DEBUG == 1
    kernel_serial_debug("Poped One");
    kernel_serial_debug(" 0x%p\n", active_thread_node[cpu_id]);
#endif
        if(active_thread_node[cpu_id] != NULL)
        {
            break;
        }


    }
    if(active_thread_node[cpu_id] == NULL || err != OS_NO_ERR)
    {
        kernel_error("Could not dequeue next thread[%d]\n", err);
        kernel_panic(err);
    }

    active_thread[cpu_id] = (kernel_thread_t*)active_thread_node[cpu_id]->data;

#if SCHED_KERNEL_DEBUG == 1
    kernel_serial_debug("Elected thread %d\n", active_thread[cpu_id]->tid);
#endif


    if(active_thread[cpu_id] == NULL)
    {
        kernel_error("Next thread to schedule should not be NULL\n");
        kernel_panic(err);
    }
    active_thread[cpu_id]->state = THREAD_STATE_RUNNING;
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
    int32_t cpu_id;

    cpu_id = cpu_get_id();
    if(cpu_id == -1)
    {
        cpu_id = 0;
    }

    (void) int_id;

    cpu_save_context(first_sched[cpu_id], cpu_state, stack_state, active_thread[cpu_id]);

    /* Search for next thread */
    select_thread();

    ++schedule_count[cpu_id];

#if SCHED_KERNEL_DEBUG == 1
    kernel_serial_debug("CPU Sched %d -> %d\n",
                        prev_thread[cpu_id]->tid, active_thread[cpu_id]->tid);
#endif

     /* Restore thread context */
    cpu_update_pgdir(active_thread[cpu_id]->cpu_context.cr3);
    cpu_restore_context(cpu_state, stack_state, active_thread[cpu_id]);

    first_sched[cpu_id] = 1;
}

SYSTEM_STATE_E get_system_state(void)
{
    return system_state;
}

OS_RETURN_E sched_init(void)
{
    OS_RETURN_E err;
    uint32_t    i;
    uint32_t    j;
    int32_t     cpu_id;

    cpu_id = cpu_get_id();
    if(cpu_id == -1)
    {
        cpu_id = 0;
    }

    /* Init scheduler settings */
    last_given_tid   = 0;
    thread_count     = 0;

    memset((void*)schedule_count, 0, sizeof(uint64_t));
    memset((void*)idle_sched_count, 0, sizeof(uint64_t));

    init_thread      = NULL;
    init_thread_node = NULL;

    memset((void*)first_sched, 0, sizeof(uintptr_t) * MAX_CPU_COUNT);

#if MAX_CPU_COUNT > 1
    INIT_SPINLOCK(&sched_lock);
#endif

    /* Init thread tables */
    global_threads_table = kernel_queue_create_queue(&err);
    if(err != OS_NO_ERR)
    {
        kernel_error("Could not create global_threads_table[%d]\n", err);
        kernel_panic(err);
    }

    for(i = 0; i < MAX_CPU_COUNT; ++i)
    {
#if MAX_CPU_COUNT > 1
        INIT_SPINLOCK(&cpu_locks[i]);
#endif

        for(j = 0; j < KERNEL_LOWEST_PRIORITY + 1; ++j)
        {
            active_threads_table[i][j] = kernel_queue_create_queue(&err);
            if(err != OS_NO_ERR)
            {
                kernel_error("Could not create active_threads_table %d [%d]\n",
                            j, err);
                kernel_panic(err);
            }
        }
    }

    zombie_threads_table = kernel_queue_create_queue(&err);
    if(err != OS_NO_ERR)
    {
        kernel_error("Could not create zombie_threads_table[%d]\n", err);
        kernel_panic(err);
    }

    for(i = 0; i < MAX_CPU_COUNT; ++i)
    {
        sleeping_threads_table[i] = kernel_queue_create_queue(&err);
        if(err != OS_NO_ERR)
        {
            kernel_error("Could not create sleeping_threads_table %d [%d]\n",
            i, err);
            kernel_panic(err);
        }
    }

    /* Create idle thread */
    err = create_idle(SCHEDULER_IDLE_STACK_SIZE);
    if(err != OS_NO_ERR)
    {
        kernel_error("Could not create IDLE thread[%d]\n", err);
        kernel_panic(err);
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

    /* Create INIT thread */
    err = sched_create_kernel_thread(&init_thread, KERNEL_HIGHEST_PRIORITY,
                                     "init", SCHEDULER_INIT_STACK_SIZE,
                                     0, init_func,
                                     (void*)0);
    if(err != OS_NO_ERR)
    {
        return err;
    }

    system_state = SYSTEM_STATE_RUNNING;

    return OS_NO_ERR;
}

OS_RETURN_E sched_init_ap(void)
{
    OS_RETURN_E err;

    /* Wait for the main CPU to schedule */
    while(first_sched[0] == 0);

    /* Create idle thread */
    err = create_idle(SCHEDULER_IDLE_STACK_SIZE);
    if(err != OS_NO_ERR)
    {
        kernel_error("Could not create IDLE thread[%d]\n", err);
        kernel_panic(err);
    }

    /* Restore interrupt and schedule */
    kernel_interrupt_restore(1);
    sched_schedule();

    return OS_ERR_UNAUTHORIZED_ACTION;
}

void sched_schedule(void)
{
    /* Raise scheduling interrupt */
    cpu_raise_interrupt(SCHEDULER_SW_INT_LINE);
}

OS_RETURN_E sched_sleep(const unsigned int time_ms)
{
    int32_t cpu_id;

    cpu_id = cpu_get_id();
    if(cpu_id == -1)
    {
        cpu_id = 0;
    }
    /* We cannot sleep in idle */
    if(active_thread[cpu_id] == idle_thread[cpu_id])
    {
        return OS_ERR_UNAUTHORIZED_ACTION;
    }

    active_thread[cpu_id]->wakeup_time =
        time_get_current_uptime() + time_ms - 1000 / KERNEL_MAIN_TIMER_FREQ;
    active_thread[cpu_id]->state       = THREAD_STATE_SLEEPING;

#if SCHED_KERNEL_DEBUG == 1
    kernel_serial_debug("[%d] Thread %d asleep until %d (%dms)\n",
                        (uint32_t)time_get_current_uptime(),
                        active_thread[cpu_id]->tid,
                        (uint32_t)active_thread[cpu_id]->wakeup_time,
                        time_ms);
#endif

    sched_schedule();

    return OS_NO_ERR;
}

uint32_t sched_get_thread_count(void)
{
    return thread_count;
}

int32_t sched_get_tid(void)
{
    int32_t cpu_id;

    cpu_id = cpu_get_id();
    if(cpu_id == -1)
    {
        cpu_id = 0;
    }

    if(first_sched[cpu_id] == 0)
    {
        return 0;
    }
    return active_thread[cpu_id]->tid;
}

kernel_thread_t* sched_get_self(void)
{
    int32_t cpu_id;

    cpu_id = cpu_get_id();
    if(cpu_id == -1)
    {
        cpu_id = 0;
    }

    if(first_sched[cpu_id] == 0)
    {
        return NULL;
    }
    return active_thread[cpu_id];
}

int32_t sched_get_ptid(void)
{
    int32_t cpu_id;

    cpu_id = cpu_get_id();
    if(cpu_id == -1)
    {
        cpu_id = 0;
    }

    return active_thread[cpu_id]->ptid;
}

uint32_t sched_get_priority(void)
{
    int32_t cpu_id;

    cpu_id = cpu_get_id();
    if(cpu_id == -1)
    {
        cpu_id = 0;
    }

    return active_thread[cpu_id]->priority;
}

OS_RETURN_E sched_set_priority(const uint32_t priority)
{
    int32_t cpu_id;

    cpu_id = cpu_get_id();
    if(cpu_id == -1)
    {
        cpu_id = 0;
    }

    /* Check if priority is free */
    if(priority > KERNEL_LOWEST_PRIORITY)
    {
        return OS_ERR_FORBIDEN_PRIORITY;
    }

    active_thread[cpu_id]->priority = priority;

    return OS_NO_ERR;
}

OS_RETURN_E get_threads_info(thread_info_t* threads, int32_t* size)
{
    int32_t               i;
    kernel_queue_node_t*  cursor;
    kernel_thread_t*      cursor_thread;

    if(threads == NULL)
    {
        return OS_ERR_NULL_POINTER;
    }
    if(size == NULL)
    {
        return OS_ERR_NULL_POINTER;
    }

    if(*size > (int)thread_count)
    {
        *size = thread_count;
    }

    /* Walk the thread list and fill the structures */
    cursor = global_threads_table->head;
    cursor_thread = (kernel_thread_t*)cursor->data;
    for(i = 0; cursor != NULL && i < *size; ++i)
    {
        thread_info_t *current = &threads[i];
        current->tid = cursor_thread->tid;
        current->ptid = cursor_thread->ptid;
        strncpy(current->name, cursor_thread->name, THREAD_MAX_NAME_LENGTH);
        current->priority = cursor_thread->priority;
        current->state = cursor_thread->state;
        current->start_time = cursor_thread->start_time;
        if(current->state != THREAD_STATE_ZOMBIE)
        {
            current->end_time = time_get_current_uptime();
        }
        else
        {
            current->end_time = cursor_thread->end_time;
        }

        cursor = cursor->next;
        cursor_thread = (kernel_thread_t*)cursor->data;
    }

    return OS_NO_ERR;
}

void sched_set_thread_termination_cause(const THREAD_TERMINATE_CAUSE_E cause)
{
    int32_t cpu_id;

    cpu_id = cpu_get_id();
    if(cpu_id == -1)
    {
        cpu_id = 0;
    }

    active_thread[cpu_id]->return_cause = cause;
}

void sched_terminate_thread(void)
{
    int32_t cpu_id;

    cpu_id = cpu_get_id();
    if(cpu_id == -1)
    {
        cpu_id = 0;
    }

    active_thread[cpu_id]->return_state = THREAD_RETURN_STATE_KILLED;

    active_thread[cpu_id]->end_time = time_get_current_uptime();

    /* Exit thread properly */
    thread_exit();
}

OS_RETURN_E sched_create_kernel_thread(thread_t* thread,
                                       const uint32_t priority,
                                       const char* name,
                                       const size_t stack_size,
                                       const uint32_t cpu_affinity,
                                       void* (*function)(void*),
                                       void* args)
{
    OS_RETURN_E          err;
    kernel_thread_t*     new_thread;
    kernel_queue_node_t* new_thread_node;
    kernel_queue_node_t* seconde_new_thread_node;
    kernel_queue_node_t* children_new_thread_node;
    uint32_t             stack_index;
    uint32_t             int_state;
    int32_t              cpu_id;

#if MAX_CPU_COUNT > 1
    uint32_t             secint_state;
#endif

    if(cpu_affinity > MAX_CPU_COUNT - 1)
    {
        return OS_ERR_UNAUTHORIZED_ACTION;
    }

    cpu_id = cpu_get_id();
    if(cpu_id == -1)
    {
        cpu_id = 0;
    }

    if(thread != NULL)
    {
        *thread = NULL;
    }

    /* Check if priority is valid */
    if(priority > KERNEL_LOWEST_PRIORITY)
    {
        return OS_ERR_FORBIDEN_PRIORITY;
    }

#if MAX_CPU_COUNT > 1
    ENTER_CRITICAL(int_state, &cpu_locks[cpu_id]);
#else
    ENTER_CRITICAL(int_state);
#endif

    new_thread = kmalloc(sizeof(kernel_thread_t));
    new_thread_node = kernel_queue_create_node(new_thread, &err);
    if(err != OS_NO_ERR ||
       new_thread == NULL ||
       new_thread_node == NULL)
    {
        if(new_thread != NULL)
        {
            kfree(new_thread);
        }
        else
        {
            err = OS_ERR_MALLOC;
        }

#if MAX_CPU_COUNT > 1
        EXIT_CRITICAL(int_state, &cpu_locks[cpu_id]);
#else
        EXIT_CRITICAL(int_state);
#endif

        return err;
    }
    memset(new_thread, 0, sizeof(kernel_thread_t));

    /* Init thread settings */
    new_thread->tid            = ++last_given_tid;
    new_thread->ptid           = active_thread[cpu_id]->tid;
    new_thread->priority       = priority;
    new_thread->init_prio      = priority;
    new_thread->args           = args;
    new_thread->function       = function;
    new_thread->joining_thread = NULL;
    new_thread->state          = THREAD_STATE_READY;
    new_thread->stack_size     = stack_size;
    new_thread->cpu_affinity   = cpu_affinity;

#if MAX_CPU_COUNT > 1
    INIT_SPINLOCK(&new_thread->lock);
#endif

    new_thread->children = kernel_queue_create_queue(&err);
    if(err != OS_NO_ERR)
    {
        kernel_queue_delete_node(&new_thread_node);
        kfree(new_thread);

#if MAX_CPU_COUNT > 1
        EXIT_CRITICAL(int_state, &cpu_locks[cpu_id]);
#else
        EXIT_CRITICAL(int_state);
#endif

        return err;
    }

    /* Init thread stack and align stack size */
    stack_index = (stack_size + ALIGN - 1) & (~(ALIGN - 1));
    stack_index /= sizeof(uintptr_t);
    new_thread->stack = kmalloc(stack_index * sizeof(uintptr_t));
    if(new_thread->stack == NULL)
    {
        kernel_queue_delete_node(&new_thread_node);
        kernel_queue_delete_queue(&new_thread->children);
        kfree(new_thread);

#if MAX_CPU_COUNT > 1
        EXIT_CRITICAL(int_state, &cpu_locks[cpu_id]);
#else
        EXIT_CRITICAL(int_state);
#endif

        return OS_ERR_MALLOC;
    }

    /* Init thread's context */
    cpu_init_thread_context(thread_wrapper, stack_index, 
                            active_thread[cpu_id]->free_page_table,
                            active_thread[cpu_id]->cpu_context.cr3,
                            new_thread);

    strncpy(new_thread->name, name, THREAD_MAX_NAME_LENGTH);

    /* Add thread to the system's queues. */
    seconde_new_thread_node = kernel_queue_create_node(new_thread, &err);
    if(err != OS_NO_ERR)
    {
        kernel_queue_delete_queue(&new_thread->children);
        kernel_queue_delete_node(&new_thread_node);
        kfree(new_thread->stack);
        kfree(new_thread);

#if MAX_CPU_COUNT > 1
        EXIT_CRITICAL(int_state, &cpu_locks[cpu_id]);
#else
        EXIT_CRITICAL(int_state);
#endif

        return err;
    }

    children_new_thread_node = kernel_queue_create_node(new_thread, &err);
    if(err != OS_NO_ERR)
    {
        kernel_queue_delete_queue(&new_thread->children);
        kernel_queue_delete_node(&new_thread_node);
        kernel_queue_delete_node(&seconde_new_thread_node);
        kfree(new_thread->stack);
        kfree(new_thread);

#if MAX_CPU_COUNT > 1
        EXIT_CRITICAL(int_state, &cpu_locks[cpu_id]);
#else
        EXIT_CRITICAL(int_state);
#endif

        return err;
    }

    err = kernel_queue_push(seconde_new_thread_node, global_threads_table);
    if(err != OS_NO_ERR)
    {
        kernel_queue_delete_queue(&new_thread->children);
        kernel_queue_delete_node(&children_new_thread_node);
        kernel_queue_delete_node(&new_thread_node);
        kernel_queue_delete_node(&seconde_new_thread_node);
        kfree(new_thread->stack);
        kfree(new_thread);

#if MAX_CPU_COUNT > 1
        EXIT_CRITICAL(int_state, &cpu_locks[cpu_id]);
#else
        EXIT_CRITICAL(int_state);
#endif

        return err;
    }


    err = kernel_queue_push(children_new_thread_node,
                            active_thread[cpu_id]->children);
    if(err != OS_NO_ERR)
    {
        kernel_queue_delete_queue(&new_thread->children);
        kernel_queue_delete_node(&children_new_thread_node);
        kernel_queue_delete_node(&new_thread_node);
        kernel_queue_delete_node(&seconde_new_thread_node);
        kernel_queue_remove(global_threads_table, seconde_new_thread_node);
        kfree(new_thread->stack);
        kfree(new_thread);

#if MAX_CPU_COUNT > 1
        EXIT_CRITICAL(int_state, &cpu_locks[cpu_id]);
#else
        EXIT_CRITICAL(int_state);
#endif

        return err;
    }

    err = kernel_queue_push(new_thread_node,
                            active_threads_table[cpu_affinity][priority]);
    if(err != OS_NO_ERR)
    {
        kernel_queue_delete_queue(&new_thread->children);
        kernel_queue_delete_node(&children_new_thread_node);
        kernel_queue_delete_node(&new_thread_node);
        kernel_queue_delete_node(&seconde_new_thread_node);
        kernel_queue_remove(global_threads_table, seconde_new_thread_node);
        kernel_queue_remove(active_thread[cpu_id]->children,
                            children_new_thread_node);
        kfree(new_thread->stack);
        kfree(new_thread);

#if MAX_CPU_COUNT > 1
        EXIT_CRITICAL(int_state, &cpu_locks[cpu_id]);
#else
        EXIT_CRITICAL(int_state);
#endif

        return err;
    }

#if MAX_CPU_COUNT > 1
    ENTER_CRITICAL(secint_state, &sched_lock);
#endif
    ++thread_count;
#if MAX_CPU_COUNT > 1
    EXIT_CRITICAL(secint_state, &sched_lock);
#endif

#if MAX_CPU_COUNT > 1
    EXIT_CRITICAL(int_state, &cpu_locks[cpu_id]);
#else
    EXIT_CRITICAL(int_state);
#endif

#if SCHED_KERNEL_DEBUG == 1
    kernel_serial_debug("Created thread %d\n", new_thread->tid);
#endif

    if(thread != NULL)
    {
        *thread = new_thread;
    }

    return OS_NO_ERR;
}

OS_RETURN_E sched_wait_thread(thread_t thread, void** ret_val,
                              THREAD_TERMINATE_CAUSE_E* term_cause)
{
    int32_t cpu_id;

    cpu_id = cpu_get_id();
    if(cpu_id == -1)
    {
        cpu_id = 0;
    }

    if(thread == NULL)
    {
        return OS_ERR_NULL_POINTER;
    }

#if SCHED_KERNEL_DEBUG == 1
    kernel_serial_debug("Thread %d waiting for thread %d\n",
                         active_thread[cpu_id]->tid,
                         thread->tid);
#endif

    if(thread->state == THREAD_STATE_DEAD)
    {
        return OS_ERR_NO_SUCH_ID;
    }

    /* If thread already done then remove it from the thread table */
    if(thread->state == THREAD_STATE_ZOMBIE)
    {
        thread->state = THREAD_STATE_DEAD;

        if(ret_val != NULL)
        {
            *ret_val = thread->ret_val;
        }
        sched_clean_joined_thread(thread);
        return OS_NO_ERR;
    }

    /* Wait for the thread to finish */
    active_thread[cpu_id]->state   = THREAD_STATE_JOINING;
    thread->joining_thread = active_thread_node[cpu_id];

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
    sched_clean_joined_thread(thread);

    return OS_NO_ERR;
}

kernel_queue_node_t* sched_lock_thread(const THREAD_WAIT_TYPE_E block_type)
{
    kernel_queue_node_t* current_thread_node;
    int32_t              cpu_id;

    cpu_id = cpu_get_id();
    if(cpu_id == -1)
    {
        cpu_id = 0;
    }

    /* Cant lock kernel thread */
    if(active_thread[cpu_id] == idle_thread[cpu_id])
    {
        return NULL;
    }

    current_thread_node = active_thread_node[cpu_id];

    /* Lock the thread */
    active_thread[cpu_id]->state      = THREAD_STATE_WAITING;
    active_thread[cpu_id]->block_type = block_type;

#if SCHED_KERNEL_DEBUG == 1
    kernel_serial_debug("Thread %d locked, reason: %d\n",
                        active_thread[cpu_id]->tid,
                        block_type);
#endif

    return current_thread_node;
}

OS_RETURN_E sched_unlock_thread(kernel_queue_node_t* node,
                                const THREAD_WAIT_TYPE_E block_type,
                                const uint32_t do_schedule)
{
    OS_RETURN_E      err;
    uint32_t         int_state;
    int32_t          cpu_id;
    kernel_thread_t* thread;

    thread = (kernel_thread_t*)node->data;

    cpu_id = cpu_get_id();
    if(cpu_id == -1)
    {
        cpu_id = 0;
    }

    /* Check thread value */
    if(thread == NULL || thread == idle_thread[cpu_id])
    {
        return OS_ERR_NO_SUCH_ID;
    }

    /* Check thread state */
    if(thread->state != THREAD_STATE_WAITING ||
       thread->block_type != block_type)
    {
        switch(block_type)
        {
            case THREAD_WAIT_TYPE_SEM:
                return OS_ERR_NO_SEM_BLOCKED;
            case THREAD_WAIT_TYPE_MUTEX:
                return OS_ERR_NO_MUTEX_BLOCKED;
            default:
                return OS_ERR_NULL_POINTER;
        }
    }

#if MAX_CPU_COUNT > 1
    ENTER_CRITICAL(int_state, &cpu_locks[cpu_id]);
#else
    ENTER_CRITICAL(int_state);
#endif

    /* Unlock thread state */
    thread->state = THREAD_STATE_READY;
    err = kernel_queue_push(node,
                            active_threads_table[thread->cpu_affinity]
                                                [thread->priority]);
    if(err != OS_NO_ERR)
    {
#if MAX_CPU_COUNT > 1
        EXIT_CRITICAL(int_state, &cpu_locks[cpu_id]);
#else
        EXIT_CRITICAL(int_state);
#endif

        kernel_error("Could not enqueue thread in active table[%d]\n", err);
        kernel_panic(err);
    }

#if SCHED_KERNEL_DEBUG == 1
    kernel_serial_debug("Thread %d unlocked, reason: %d\n",
                         thread->tid,
                         block_type);
#endif

#if MAX_CPU_COUNT > 1
    EXIT_CRITICAL(int_state, &cpu_locks[cpu_id]);
#else
    EXIT_CRITICAL(int_state);
#endif

    if(do_schedule)
    {
        sched_schedule();
    }

    return OS_NO_ERR;
}

uint64_t sched_get_schedule_count(void)
{
    int32_t cpu_id;

    cpu_id = cpu_get_id();
    if(cpu_id == -1)
    {
        cpu_id = 0;
    }
    return schedule_count[cpu_id];
}

uint64_t sched_get_idle_schedule_count(void)
{
    int32_t cpu_id;

    cpu_id = cpu_get_id();
    if(cpu_id == -1)
    {
        cpu_id = 0;
    }
    return idle_sched_count[cpu_id];
}

uintptr_t sched_get_thread_free_page_table(void)
{
    int32_t cpu_id;

    cpu_id = cpu_get_id();
    if(cpu_id == -1)
    {
        cpu_id = 0;
    }

    if(first_sched[cpu_id] == 1)
    {
        return (uintptr_t)kernel_free_pages;
    }
    return active_thread[cpu_id]->free_page_table;
}

uintptr_t sched_get_thread_phys_pgdir(void)
{
    int32_t cpu_id;

    cpu_id = cpu_get_id();
    if(cpu_id == -1)
    {
        cpu_id = 0;
    }

    return cpu_get_current_pgdir();
}