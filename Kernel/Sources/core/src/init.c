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
 * GLOBAL VARIABLES
 ******************************************************************************/

void* init_sys(void* args)
{
    int32_t pid;
    uint32_t time;

    (void)args;

    KERNEL_INFO("INIT Started | PID: %d | TID: %d\n", 
                sched_get_pid(), 
                sched_get_tid());

    pid = fork();
    if(pid)
    {
        fork();
    }

    kernel_printf("Hello from Init %d\n", sched_get_pid());

    
    while(1)
    {
        if(sched_get_pid() == 0)
        {
            time = rtc_get_current_daytime();
            kernel_printf("\r%02d:%02d:%02d", time / 60 / 60, (time / 60) % 60, time % 60);
        }
        sched_sleep(1000);
    }

    /* If we return better go away and cry in a corner */
    KERNEL_PANIC(OS_ERR_UNAUTHORIZED_ACTION);
    return NULL;
}