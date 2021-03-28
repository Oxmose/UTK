/*******************************************************************************
 * @file init.c
 *
 * @see init.h
 *
 * @author Alexy Torres Aurora Dugo
 *
 * @date 19/03/2021
 *
 * @version 1.0
 *
 * @brief Kernel's init thread
 *
 * @details Kernel's init thread. Starts the first processes.
 *
 * @copyright Alexy Torres Aurora Dugo
 ******************************************************************************/

#include <stdint.h>        /* Generic int types */
#include <stddef.h>        /* Standard definitions */
#include <kernel_output.h> /* Kernel output methods */
#include <scheduler.h>     /* Kernel scheduler */
#include <panic.h>         /* Kernel panic */
#include <cpu_api.h>       /* CPU API */

#include <sys/process.h> 
#include <rt_clock.h>

/* UTK configuration file */
#include <config.h>

#ifdef TEST_MODE_ENABLED
#include <test_bank.h>
#endif

/* Header file */
#include <init.h>


/*******************************************************************************
 * CONSTANTS
 ******************************************************************************/

/* None */

/*******************************************************************************
 * STRUCTURES
 ******************************************************************************/

/* None */

/*******************************************************************************
 * GLOBAL VARIABLES
 ******************************************************************************/

/** @brief Count of the number of times the idle thread was scheduled. */
static volatile uint64_t idle_sched_count = 0;

/*******************************************************************************
 * STATIC FUNCTIONS DECLARATION
 ******************************************************************************/

/* None */

/*******************************************************************************
 * FUNCTIONS
 ******************************************************************************/
/*
static __attribute__((optimize("O0"))) void* thread_func_test(void* args) 
{
    (void)args;
    for(volatile unsigned int i = 0; i < 400000000; ++i);
    return NULL;
}
*/
void* init_sys(void* args)
{
    uint32_t time;
    int32_t  pid;
    int32_t  status;
    //kernel_thread_t* new_thread;
    int32_t  term_cause;
    OS_RETURN_E err;

    (void)args;

    KERNEL_INFO("INIT Started | PID: %d | TID: %d\n", 
                sched_get_pid(), 
                sched_get_tid());

    pid = fork();

    kernel_printf("Hello from Init %d\n", sched_get_pid());

    if(pid != 0)
    {
        sched_sleep(2000);
        //sched_create_kernel_thread(&new_thread, 32, "test", THREAD_TYPE_KERNEL, 0x1000, thread_func_test, NULL);
        //sched_join_thread(new_thread, NULL, NULL);
        time = rtc_get_current_daytime();
        //kernel_printf("\r%02d:%02d:%02d", time / 60 / 60, (time / 60) % 60, time % 60);

        pid = waitpid(pid, &status, &term_cause, &err);
        kernel_printf("Process %d returned %d, %d\n", pid, status, err);
    }
    
    while(1)
    {
        if(pid != 0)
        {
            time = rtc_get_current_daytime();
            (void)time;
            //kernel_printf("\r%02d:%02d:%02d", time / 60 / 60, (time / 60) % 60, time % 60);
        }
        else 
        {
            kernel_printf("Forked returns\n");
            return (void*)42;
        }
        sched_sleep(500);
    }

    /* If we return better go away and cry in a corner */
    KERNEL_PANIC(OS_ERR_UNAUTHORIZED_ACTION);
    return NULL;
}

void* idle_sys(void* args)
{
    (void)args;

    KERNEL_INFO("IDLE Started | PID: %d | TID: %d\n", 
                sched_get_pid(), 
                sched_get_tid());

    while(1)
    {
        ++idle_sched_count;

        kernel_interrupt_restore(1);
        cpu_hlt();
    }

    /* If we return better go away and cry in a corner */
    KERNEL_PANIC(OS_ERR_UNAUTHORIZED_ACTION);

    return NULL;
}

uint64_t sched_get_idle_schedule_count(void)
{
    return idle_sched_count;
}