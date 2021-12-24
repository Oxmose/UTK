/*******************************************************************************
 * @file time_management.c
 *
 * @see time_management.h
 *
 * @author Alexy Torres Aurora Dugo
 *
 * @date 09/03/2021
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

/*******************************************************************************
 * INCLUDES
 ******************************************************************************/

/* Included headers */
#include <stdint.h>        /* Generic int types */
#include <stddef.h>        /* Standard definitions */
#include <string.h>        /* String manipulation */
#include <cpu_settings.h>  /* CPU structures */
#include <cpu_api.h>       /* CPU management */
#include <kheap.h>         /* Kernel heap */
#include <bsp_api.h>       /* BSP API */
#include <kernel_output.h> /* Kernel output manager */
#include <interrupts.h>    /* Interrupt manager */

/* Configuration files */
#include <config.h>
#include <test_bank.h>

/* Header file */
#include <time_management.h>

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

/* None */

/*******************************************************************************
 * GLOBAL VARIABLES
 ******************************************************************************/

/************************* Imported global variables **************************/
/** @brief NULL timer driver. */
extern kernel_timer_t null_timer;

/************************* Exported global variables **************************/
/* None */

/************************** Static global variables ***************************/
/** @brief Stores the number of main kernel's timer tick since the
 * initialization of the time manager.
 */
static uint64_t* sys_tick_count;

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

/** @brief Active wait counter per CPU. */
static volatile int64_t* active_wait;

/** @brief Stores the routine to call the scheduler. */
void (*schedule_routine)(cpu_state_t*, uintptr_t, stack_state_t*) = NULL;

/** @brief RTC interrupt managet */
void (*rtc_int_manager)(void) = NULL;

/*******************************************************************************
 * STATIC FUNCTIONS DECLARATIONS
 ******************************************************************************/

/**
 * @brief The kernel's main timer interrupt handler.
 *
 * @details The kernel's main timer interrupt handler. This must be connected to
 * the main timer of the system.
 *
 * @param[in, out] cpu_state The cpu registers before the interrupt.
 * @param[in] int_id The interrupt line that called the handler.
 * @param[in, out] stack The stack state before the interrupt.
 */
static void time_main_timer_handler(cpu_state_t* cpu_state,
                                    uintptr_t int_id,
                                    stack_state_t* stack);

/**
 * @brief The kernel's RTC timer interrupt handler.
 *
 * @details The kernel's RTC timer interrupt handler. This must be connected to
 * the RTC timer of the system.
 *
 * @param[in, out] cpu_state The cpu registers before the interrupt.
 * @param[in] int_id The interrupt line that called the handler.
 * @param[in, out] stack The stack state before the interrupt.
 */
static void time_rtc_timer_handler(cpu_state_t* cpu_state,
                                   uintptr_t int_id,
                                   stack_state_t* stack);

/*******************************************************************************
 * FUNCTIONS
 ******************************************************************************/

static void time_main_timer_handler(cpu_state_t* cpu_state,
                                    uintptr_t int_id,
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

    /* EOI */
    kernel_interrupt_set_irq_eoi(sys_main_timer.get_irq());

    if(schedule_routine != NULL)
    {
        /* We might never come back from here */
        schedule_routine(cpu_state, int_id, stack);
    }
    else
    {
        if(active_wait[cpu_id] > 0)
        {
            uint32_t time_slice = 1000 / sys_main_timer.get_frequency();
            if(active_wait[cpu_id] >= time_slice)
            {
                active_wait[cpu_id] -= time_slice;
            }
            else
            {
                active_wait[cpu_id] = 0;
            }
        }
    }

    KERNEL_DEBUG(TIME_MGT_DEBUG_ENABLED, "[TIME] Time manager main handler");
}

static void time_rtc_timer_handler(cpu_state_t* cpu_state,
                                   uintptr_t int_id,
                                   stack_state_t* stack)
{
    (void)cpu_state;
    (void)int_id;
    (void)stack;

    if(rtc_int_manager != NULL)
    {
        rtc_int_manager();
    }

    KERNEL_DEBUG(TIME_MGT_DEBUG_ENABLED, "[TIME] Time manager RTC handler");

    /* EOI */
    kernel_interrupt_set_irq_eoi(sys_rtc_timer.get_irq());
}

OS_RETURN_E time_init(const kernel_timer_t* main_timer,
                      const kernel_timer_t* rtc_timer)
{
    OS_RETURN_E err;
    uint32_t    cpu_count;

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

    KERNEL_DEBUG(TIME_MGT_DEBUG_ENABLED, "[TIME] Time manager Initialization");

    /* Init the system's values */
    cpu_count = get_cpu_count();
    sys_tick_count = kmalloc(sizeof(uint64_t) * cpu_count);
    if(sys_tick_count == NULL)
    {
        return OS_ERR_MALLOC;
    }
    active_wait = kmalloc(sizeof(uint32_t) * cpu_count);
    if(active_wait == NULL)
    {
        kfree((void*)sys_tick_count);
        return OS_ERR_MALLOC;
    }
    memset(sys_tick_count, 0, sizeof(uint64_t) * cpu_count);

    /* Sets all the possible timer interrutps */
    sys_main_timer.set_frequency(KERNEL_MAIN_TIMER_FREQ);

    err = sys_main_timer.set_handler(time_main_timer_handler);
    if(err != OS_NO_ERR)
    {
        kfree((void*)sys_tick_count);
        kfree((void*)active_wait);
        return err;
    }

    KERNEL_DEBUG(TIME_MGT_DEBUG_ENABLED, "[TIME] Initialized main timer");

    if(sys_rtc_timer.set_frequency != NULL)
    {
        sys_rtc_timer.set_frequency(KERNEL_RTC_TIMER_FREQ);

        err = sys_rtc_timer.set_handler(time_rtc_timer_handler);
        if(err != OS_NO_ERR)
        {
            kfree((void*)sys_tick_count);
            kfree((void*)active_wait);
            return err;
        }
    }

    KERNEL_DEBUG(TIME_MGT_DEBUG_ENABLED, "[TIME] Initialized RTC timer");

    /* Enables all the possible timers */
    sys_main_timer.enable();
    if(sys_rtc_timer.set_frequency != NULL)
    {
        sys_rtc_timer.enable();
    }

    KERNEL_TEST_POINT(time_test);

    return OS_NO_ERR;
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

    time_slice = 1000000000ULL / (uint64_t)sys_main_timer.get_frequency();
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
    int32_t  cpu_id;

    cpu_id = cpu_get_id();

    if(schedule_routine != NULL)
    {
        return;
    }
    active_wait[cpu_id] = ms;
    while(active_wait[cpu_id] > 0){}
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


    KERNEL_DEBUG(TIME_MGT_DEBUG_ENABLED,
                 "[TIME] Registered scheduler routine at 0x%p",
                 scheduler_call);

    schedule_routine = scheduler_call;

    return OS_NO_ERR;
}

OS_RETURN_E time_register_rtc_manager(void (*rtc_manager)(void))
{
    if(rtc_manager == NULL)
    {
        return OS_ERR_NULL_POINTER;
    }

    KERNEL_DEBUG(TIME_MGT_DEBUG_ENABLED,
                 "[TIME] Registered RTC routine at 0x%p",
                 rtc_manager);

    rtc_int_manager = rtc_manager;

    return OS_NO_ERR;
}

/************************************ EOF *************************************/