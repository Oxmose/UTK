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

#include <init_rd.h> 
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

#if 1

#include <graphic.h>
#include <string.h>

uint64_t last_idle_sched_count;

void update_shell(uint32_t freq)
{
    uint32_t time;
    date_t   date;
    uint8_t  cpu_usage = 50;
    colorscheme_t scheme;
    uint32_t idle_count;
/*
UTK - Shell v0.1            00:00:00 Wed 01/02/2022                   CPU: 100%
*/
    /* Draw the top line */
    scheme.background = BG_DARKGREY;
    scheme.foreground = FG_CYAN;
    scheme.vga_color  = 1;
    graphic_set_color_scheme(scheme);
    graphic_put_cursor_at(0, 0);
    kernel_printf(" UTK - Shell v0.1               ");
    
    /* Print date */
    time = rtc_get_current_daytime();
    date = rtc_get_current_date();

    kernel_printf("%02d:%02d:%02d %02d/%02d/%04d                     ", 
                  time / 60 / 60, (time / 60) % 60, time % 60,
                  date.day, date.month, date.year);

    /* Print CPU */
    idle_count = idle_sched_count - last_idle_sched_count;
    (void)freq;
    cpu_usage =  100 - idle_count * 100 / (KERNEL_MAIN_TIMER_FREQ / freq + 3);
    kernel_printf("CPU: %3d\%", cpu_usage);
    last_idle_sched_count = idle_sched_count;
}

void* thread_func_test(void* args)
{
    (void)args;
    while(1)
    {
        for(volatile int i = 0; i < 100000000; ++i)
        {

        }
        sched_sleep(500);
    }
}

/* Just a schell for fun */
void init_shell(void)
{
    kernel_thread_t* new_thread;
    sched_create_kernel_thread(&new_thread, 32, "test", THREAD_TYPE_KERNEL, 0x1000, thread_func_test, NULL);

    last_idle_sched_count = idle_sched_count;
    /* Clear screen */
    graphic_clear_screen();

    /* Wait in loop */
    while(1) 
    {
        update_shell(2);
        sched_sleep(500);
    }
}

#endif


void* init_sys(void* args)
{
    int32_t     pid;
    int32_t     status;
    int32_t     term_cause;
    OS_RETURN_E err;

    (void)args;

    KERNEL_INFO("INIT Started | PID: %d | TID: %d\n", 
                sched_get_pid(), 
                sched_get_tid());

    pid = fork();

    if(pid != 0)
    {
        pid = waitpid(pid, &status, &term_cause, &err);
        kernel_printf("Process %d returned %d, %d\n", pid, status, err);
    }
    else 
    {
        /* Here we should load an elf and start another program */
        init_shell();
        return 0;
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