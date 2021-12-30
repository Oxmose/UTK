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

/*******************************************************************************
 * INCLUDES
 ******************************************************************************/

/* Included headers */
#include <stdint.h>        /* Generic int types */
#include <stddef.h>        /* Standard definitions */
#include <kernel_output.h> /* Kernel output methods */
#include <panic.h>         /* Kernel panic */
#include <sys/process.h>   /* Fork API */
#include <rt_clock.h>      /* RTC API */
#include <vga_text.h>      /* VGA colors */
#include <scheduler.h>     /* Scheduler API */
#include <cpu_api.h>       /* CPU API */

/* Configuration files */
#include <config.h>
#include <test_bank.h>

/* Header file */
#include <init.h>

/*******************************************************************************
 * CONSTANTS
 ******************************************************************************/

/* None */

/*******************************************************************************
 * STRUCTURES AND TYPES
 ******************************************************************************/

/* None */

/*******************************************************************************
 * MACROS
 ******************************************************************************/

/**
 * @brief Assert macro used by the init module to ensure correctness of
 * execution.
 *
 * @details Assert macro used by the init module to ensure correctness of
 * execution. Due to the critical nature of the init module, any error
 * generates a kernel panic.
 *
 * @param[in] COND The condition that should be true.
 * @param[in] MSG The message to display in case of kernel panic.
 * @param[in] ERROR The error code to use in case of kernel panic.
 */
#define INIT_ASSERT(COND, MSG, ERROR) {                     \
    if((COND) == FALSE)                                     \
    {                                                       \
        PANIC(ERROR, "INIT", MSG, TRUE);                    \
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
/** @brief Count of the number of times the idle thread was scheduled. */
static volatile uint64_t idle_sched_count = 0;

/*******************************************************************************
 * STATIC FUNCTIONS DECLARATIONS
 ******************************************************************************/

/* None */

/*******************************************************************************
 * FUNCTIONS
 ******************************************************************************/

#if 0

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
    scheme.background = BG_CYAN;
    scheme.foreground = FG_BLACK;
    scheme.vga_color  = FALSE;
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

/* Just a schell for fun */
void init_shell(void)
{
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

    KERNEL_TEST_POINT(ustar_test);
    KERNEL_TEST_POINT(fork_test);
    KERNEL_TEST_POINT(user_heap_test);
    KERNEL_TEST_POINT(memory_usage_test);
    KERNEL_TEST_POINT(critical_test);
    KERNEL_TEST_POINT(scheduler_load_test);
    KERNEL_TEST_POINT(scheduler_preempt_test);
    KERNEL_TEST_POINT(scheduler_sleep_test);
    KERNEL_TEST_POINT(futex_test);
    KERNEL_TEST_POINT(spinlock_test);
    KERNEL_TEST_POINT(mutex_test);
    KERNEL_TEST_POINT(semaphore_test);

    pid = fork();

    if(pid != 0)
    {
        pid = waitpid(pid, &status, &term_cause, &err);
        kernel_printf("Process %d returned %d, %d\n", pid, status, err);
    }
    else
    {
        /* Here we should load an elf and start another program */
        kernel_printf("\n");
        kernel_printf("\rCannot find any process panic in 3");
        sched_sleep(1000);
        kernel_printf("\rCannot find any process panic in 2");
        sched_sleep(1000);
        kernel_printf("\rCannot find any process panic in 1");
        sched_sleep(1000);
        kernel_printf("\n");
        INIT_ASSERT(FALSE,
                    "No process to launch",
                    OS_ERR_UNAUTHORIZED_ACTION);
        return (void*)OS_ERR_UNAUTHORIZED_ACTION;
    }

    /* If we return better go away and cry in a corner */
    INIT_ASSERT(FALSE, "INIT returned", OS_ERR_UNAUTHORIZED_ACTION);
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
    INIT_ASSERT(FALSE, "IDLE returned", OS_ERR_UNAUTHORIZED_ACTION);
    return NULL;
}

uint64_t sched_get_idle_schedule_count(void)
{
    return idle_sched_count;
}

/************************************ EOF *************************************/