/*******************************************************************************
 * @file time_management.c
 *
 * @see time_management.h
 *
 * @author Alexy Torres Aurora Dugo
 *
 * @date 29/09/2018
 *
 * @version 1.0
 *
 * @brief Kernel's time management methods.
 *
 * @details Kernel's time management method. Allow to define timers and keep
 * track on the system's time.
 *
 * @warning All the interrupt managers and timer sources drivers must be
 * initialized before using any of these functions.
 *
 * @copyright Alexy Torres Aurora Dugo
 ******************************************************************************/

#include <lib/stdint.h>   /* Generic int types */
#include <lib/stddef.h>   /* Standard definitions */
#include <lib/string.h>   /* String manipulation */
#include <cpu_structs.h>  /* CPU structures */
#include <cpu.h>          /* CPU management */
#include <rtc.h>          /* rtc_update_time */

/* UTK configuration file */
#include <config.h>

/* Tests header file */
#if TEST_MODE_ENABLED
#include <Tests/test_bank.h>
#endif

/* Header file */
#include <time/time_management.h>

/*******************************************************************************
 * GLOBAL VARIABLES
 ******************************************************************************/

/** @brief Stores the number of main kernel's timer tick since the
 * initialization of the time manager.
 */
static uint64_t sys_tick_count[MAX_CPU_COUNT];

/** @brief The kernel's main timer interrupt source.
 *
 *  @details The kernel's main timer interrupt source. If it's function pointers
 * are NULL, the driver is not initialized.
 */
static kernel_timer_t sys_main_timer = {NULL};

/** @brief The kernel's RTC timer interrupt source.
 *
 *  @details The kernel's RTC timer interrupt source. If it's function pointers
 * are NULL, the driver is not initialized.
 */
static kernel_timer_t sys_rtc_timer = {NULL};

/** @brief The kernel's auxiliary timer interrupt source.
 *
 *  @details The kernel's auxiliary timer interrupt source. If it's function
 * pointers are NULL, the driver is not initialized.
 */
static kernel_timer_t sys_aux_timer = {NULL};

/** @brief Active wait counter. */
static volatile uint32_t active_wait;

/** @brief Stores the routine to call the scheduler. */
void(*schedule_routine)(cpu_state_t*, uintptr_t, stack_state_t*) = NULL;

/*******************************************************************************
 * FUNCTIONS
 ******************************************************************************/

OS_RETURN_E time_init(const kernel_timer_t* main_timer,
                      const kernel_timer_t* rtc_timer,
                      const kernel_timer_t* aux_timer)
{
    OS_RETURN_E err;

#if TIME_KERNEL_DEBUG == 1
    kernel_serial_debug("Time manager Initialization\n");
#endif

    /* Check the main timer integrity */
    if(main_timer == NULL ||
       main_timer->get_frequency == NULL ||
       main_timer->set_frequency == NULL ||
       main_timer->enable == NULL ||
       main_timer->disable == NULL ||
       main_timer->set_handler == NULL ||
       main_timer->remove_handler == NULL ||
       main_timer->get_irq == NULL)

    {
        return OS_ERR_NULL_POINTER;
    }
    sys_main_timer = *main_timer;

    /* Check the rtc timer integrity */
    if(rtc_timer != NULL)
    {
        if(rtc_timer->get_frequency != NULL &&
           rtc_timer->set_frequency != NULL &&
           rtc_timer->enable != NULL &&
           rtc_timer->disable != NULL &&
           rtc_timer->set_handler != NULL &&
           rtc_timer->remove_handler != NULL &&
           rtc_timer->get_irq != NULL)
        {
            sys_rtc_timer = *rtc_timer;
        }
        else
        {
            return OS_ERR_NULL_POINTER;
        }
    }

    /* Check the aux timer integrity */
    if(aux_timer != NULL)
    {
        if(aux_timer->get_frequency != NULL ||
           aux_timer->set_frequency != NULL ||
           aux_timer->enable != NULL ||
           aux_timer->disable != NULL ||
           aux_timer->set_handler != NULL ||
           aux_timer->remove_handler != NULL ||
           aux_timer->get_irq != NULL)
        {

            sys_aux_timer = *aux_timer;
        }
        else
        {
            return OS_ERR_NULL_POINTER;
        }
    }

    /* Init he system's values */
    memset(sys_tick_count, 0, sizeof(uint64_t) * MAX_CPU_COUNT);

    /* Sets all the possible timer interrutps */
    err = sys_main_timer.set_frequency(KERNEL_MAIN_TIMER_FREQ);
    if(err != OS_NO_ERR)
    {
        return err;
    }
    err = sys_main_timer.set_handler(time_main_timer_handler);
    if(err != OS_NO_ERR)
    {
        return err;
    }

    if(sys_rtc_timer.set_frequency != NULL)
    {
        err = sys_rtc_timer.set_frequency(KERNEL_RTC_TIMER_FREQ);
        if(err != OS_NO_ERR)
        {
            return err;
        }
        err = sys_rtc_timer.set_handler(time_rtc_timer_handler);
        if(err != OS_NO_ERR)
        {
            return err;
        }
    }

    if(sys_aux_timer.set_frequency != NULL)
    {
        err = sys_aux_timer.set_frequency(KERNEL_AUX_TIMER_FREQ);
        if(err != OS_NO_ERR)
        {
            return err;
        }
        err = sys_aux_timer.set_handler(time_aux_timer_handler);
        if(err != OS_NO_ERR)
        {
            return err;
        }
    }

    /* Enables all the possible timers */
    err = sys_main_timer.enable();
    if(err != OS_NO_ERR)
    {
        return err;
    }
    if(sys_rtc_timer.set_frequency != NULL)
    {
        err = sys_rtc_timer.enable();
        if(err != OS_NO_ERR)
        {
            return err;
        }
    }
    if(sys_aux_timer.set_frequency != NULL)
    {
        err = sys_aux_timer.enable();
        if(err != OS_NO_ERR)
        {
            return err;
        }
    }

#if TEST_MODE_ENABLED
    time_test();
#endif


    return OS_NO_ERR;
}

void time_main_timer_handler(cpu_state_t* cpu_state, uintptr_t int_id,
                             stack_state_t* stack)
{
    int32_t cpu_id;

    (void)cpu_state;
    (void)int_id;
    (void)stack;

    cpu_id = cpu_get_id();
    if(cpu_id == -1)
    {
        cpu_id = 0;
    }

    /* Add a tick count */
    ++sys_tick_count[cpu_id];

    if(schedule_routine != NULL)
    {
        schedule_routine(cpu_state, int_id, stack);
    }
    else
    {
        if(active_wait > 0)
        {
            uint32_t time_slice = 1000 / sys_main_timer.get_frequency();
            if(active_wait >= time_slice)
            {
                active_wait -= time_slice;
            }
            else
            {
                active_wait = 0;
            }
        }
    }

#if TIME_KERNEL_DEBUG == 1
    kernel_serial_debug("Time manager main handler\n");
#endif

    /* EOI */
    kernel_interrupt_set_irq_eoi(sys_main_timer.get_irq());
}

void time_rtc_timer_handler(cpu_state_t* cpu_state, uintptr_t int_id,
                            stack_state_t* stack)
{
    (void)cpu_state;
    (void)int_id;
    (void)stack;

    rtc_update_time();

#if TIME_KERNEL_DEBUG == 1
    kernel_serial_debug("Time manager RTC handler\n");
#endif

    /* EOI */
    kernel_interrupt_set_irq_eoi(sys_rtc_timer.get_irq());
}

void time_aux_timer_handler(cpu_state_t* cpu_state, uintptr_t int_id,
                            stack_state_t* stack)
{
    (void)cpu_state;
    (void)int_id;
    (void)stack;

#if TIME_KERNEL_DEBUG == 1
    kernel_serial_debug("Time manager AUX handler\n");
#endif

    /* EOI */
    kernel_interrupt_set_irq_eoi(sys_aux_timer.get_irq());
}

uint64_t time_get_current_uptime(void)
{
    uint64_t time_slice;
    int32_t  cpu_id;

    cpu_id = cpu_get_id();
    if(cpu_id == -1)
    {
        cpu_id = 0;
    }

    if(sys_main_timer.get_frequency == NULL)
    {
        return 0;
    }

    time_slice = 1000 / sys_main_timer.get_frequency();
    return time_slice * sys_tick_count[cpu_id];
}

uint64_t time_get_tick_count(void)
{
    int32_t  cpu_id;

    cpu_id = cpu_get_id();
    if(cpu_id == -1)
    {
        cpu_id = 0;
    }

    return sys_tick_count[cpu_id];
}

void time_wait_no_sched(const uint32_t ms)
{
    if(schedule_routine != NULL)
    {
        return;
    }
    active_wait = ms;
    while(active_wait > 0);
}

OS_RETURN_E time_register_scheduler(void(*scheduler_call)(
                                             cpu_state_t*,
                                             uintptr_t,
                                             stack_state_t*
                                             )
                                       )
{
    if(scheduler_call == NULL)
    {
        return OS_ERR_NULL_POINTER;
    }

    schedule_routine = scheduler_call;

    return OS_NO_ERR;
}