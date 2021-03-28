/*******************************************************************************
 * @file time_management.h
 * 
 * @see time_management.c
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

#ifndef __TIME_TIME_MANAGEMENT_H_
#define __TIME_TIME_MANAGEMENT_H_

#include <stdint.h>       /* Generic int types */
#include <stddef.h>       /* Standard definitions */
#include <cpu_settings.h> /* CPU structures */

/*******************************************************************************
 * CONSTANTS
 ******************************************************************************/

/* None */

/*******************************************************************************
 * STRUCTURES
 ******************************************************************************/

/** 
 * @brief The kernel's timer driver abstraction.
 */
struct kernel_timer
{
    /** 
     * @brief The function should return the frequency of the timer source.
     * 
     * @details The function should return the frequency of the timer source.
     * 
     * @return The function should return the frequency of the timer source.
     */
    uint32_t (*get_frequency)(void);

    /** 
     * @brief The function should allow the kernel to set the frequency of a 
     * timer source.
     * 
     * @details The function should allow the kernel to set the frequency of a 
     * timer source. The frequency is defined in Hz.
     * 
     * @param[in] frequency The frequency to apply to the timer source.
     */
    void (*set_frequency)(const uint32_t frequency);

    /**
     * @brief The function should enable the timer's inetrrupt.
     * 
     * @details The function should enable the timer's inetrrupt.
     */
    void (*enable)(void);

    /**
     * @brief The function should disable the timer's inetrrupt.
     * 
     * @details The function should disable the timer's inetrrupt.
     */
    void (*disable)(void);

    /**
     * @brief The function should set the timer's tick handler.
     *
     * @details The function should set the timer's tick handler. The handler
     * will be called at each tick received.
     * 
     * @param[in] handler The handler of the timer's interrupt.
     * 
     * @return The success state or the error code. 
     * - OS_NO_ERR is returned if no error is encountered. 
     * - OS_ERR_NULL_POINTER is returned if the handler is NULL.
     * - OR_ERR_UNAUTHORIZED_INTERRUPT_LINE is returned if the timer interrupt 
     *   line is not allowed. 
     * - OS_ERR_NULL_POINTER is returned if the pointer to the handler is NULL. 
     * - OS_ERR_INTERRUPT_ALREADY_REGISTERED is returned if a handler is already 
     *   registered for the timer.
     */
    OS_RETURN_E (*set_handler)(void(*handler)(
                                 cpu_state_t*,
                                 uintptr_t,
                                 stack_state_t*
                                 ));

    /**
     * @brief The function should remove the timer tick handler.
     *
     * @details The function should remove the timer tick handler.
     * 
     * @return The success state or the error code. 
     * - OS_NO_ERR is returned if no error is encountered. 
     * - OR_ERR_UNAUTHORIZED_INTERRUPT_LINE is returned if the timer interrupt 
     *   line is not allowed. 
     * - OS_ERR_INTERRUPT_NOT_REGISTERED is returned if the timer line has no 
     *   handler attached.
     */
    OS_RETURN_E (*remove_handler)(void);

    /** 
     * @brief The function should return the IRQ line associated to the timer
     * source.
     * 
     * @details The function should return the IRQ line associated to the timer
     * source.
     * 
     * @return The function should return the IRQ line associated to the timer
     * source.
     */
    uint32_t (*get_irq)(void);
};

/** 
 * @brief Defines kernel_timer_t type as a shorcut for struct kernel_timer.
 */
typedef struct kernel_timer kernel_timer_t;

/*******************************************************************************
 * FUNCTIONS
 ******************************************************************************/

/** 
 * @brief Initializes the time manager.
 * 
 * @details Initializes the kernel's time manager. Set the basic time structures
 * and interrupts. The rtc and aux timer drivers can be set to NULL but the main
 * driver has to be fully set.
 * 
 * @param[in] main_timer The system's main timer driver.
 * @param[in] rtc_timer The RTC timer driver.
 * 
 * @warning All the interrupt managers and timer sources drivers must be 
 * initialized before using this function.
 * 
 * @return The success state or the error code. 
 * - OS_NO_ERR is returned if no error is encountered. 
 * - OS_ERR_NULL_POINTER if the main timer driver is NULL or has NULL function
 *   pointers. This value is also returned is the RTC or AUX driver is not NULL
 *   but contains NULL function pointers.
 * - OS_ERR_NO_SUCH_IRQ_LINE is returned if the IRQ number of the kernel timers
 *   sources are not supported.
 * - OR_ERR_UNAUTHORIZED_INTERRUPT_LINE is returned if the interrupt lines of 
 *   the kernel timers sources are not supported.
 */
OS_RETURN_E time_init(const kernel_timer_t* main_timer,
                      const kernel_timer_t* rtc_timer);

/** 
 * @brief Returns the current uptime.
 * 
 * @details Return the current uptime of the system in ns.
 *
 * @return The current uptime in ns.
 */
uint64_t time_get_current_uptime(void);

/**
 * @brief Returns the number of system's ticks since the system started.
 * 
 * @details Returns the number of system's ticks since the system started.
 *
 * @returns The number of system's ticks since the system started.
 */
uint64_t time_get_tick_count(void);

/**
 * @brief Performs a wait for ms milliseconds.
 * 
 * @details Performs a wait for ms milliseconds based on the kernel's main
 * timer.
 * 
 * @param[in] ms The time to wait in milliseconds.
 * 
 * @warning This function must only be called before the scheduler is 
 * initialized. Otherwise the function will immediatly return.
 */
void time_wait_no_sched(const uint32_t ms);

/** 
 * @brief Registers the function to call the system's scheduler.
 * 
 * @details Registers the function to call the system's scheduler. This function
 * will be called at each tick of the main timer.
 * 
 * @param[in] scheduler_call The scheduling routine to call every tick.
 * 
 * @return The success state or the error code. 
 * - OS_NO_ERR is returned if no error is encountered. 
 * - OS_ERR_NULL_POINTER if the scheduler routine pointer is NULL.
 */
OS_RETURN_E time_register_scheduler(void(*scheduler_call)(
                                             cpu_state_t*,
                                             uintptr_t,
                                             stack_state_t*
                                             )
                                       );

/** 
 * @brief Registers the function to call the RTC manager.
 * 
 * @details Registers the function to call the RTC manager. This function
 * will be called at each tick of the RTC timer.
 * 
 * @param[in] rtc_manager The rtc manager routine to call every RTC tick.
 * 
 * @return The success state or the error code. 
 * - OS_NO_ERR is returned if no error is encountered. 
 * - OS_ERR_NULL_POINTER if the scheduler routine pointer is NULL.
 */
OS_RETURN_E time_register_rtc_manager(void (*rtc_manager)(void));

#endif /* #ifndef __TIME_TIME_MANAGEMENT_H_ */