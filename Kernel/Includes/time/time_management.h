/*******************************************************************************
 * @file time_management.h
 * 
 * @see time_management.c
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

#ifndef __TIME_TIME_MANAGEMENT_H_
#define __TIME_TIME_MANAGEMENT_H_

#include <lib/stdint.h>   /* Generic int types */
#include <lib/stddef.h>   /* Standard definitions */
#include <cpu_structs.h>  /* CPU structures */

/*******************************************************************************
 * CONSTANTS
 ******************************************************************************/

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
     * 
     * @return The success state or the error code. 
     * - OS_NO_ERR is returned if no error is rate. 
     * - OS_ERR_OUT_OF_BOUND is returned if the frequency is out of the bounds
     * defined by the timer source.
     * - OS_ERR_NO_SUCH_IRQ_LINE is returned if the IRQ number of the source is 
     * not supported.
     */
    OS_RETURN_E (*set_frequency)(const uint32_t frequency);

    /**
     * @brief The function should enable the timer's inetrrupt.
     * 
     * @details The function should enable the timer's inetrrupt.
     *
     * @return The success state or the error code. 
     * - OS_NO_ERR is returned if no error is encountered. 
     * - OS_ERR_NO_SUCH_IRQ_LINE is returned if the IRQ number of the timer is 
     * not supported.
     */
    OS_RETURN_E (*enable)(void);

    /**
     * @brief The function should disable the timer's inetrrupt.
     * 
     * @details The function should disable the timer's inetrrupt.
     *
     * @return The success state or the error code. 
     * - OS_NO_ERR is returned if no error is encountered. 
     * - OS_ERR_NO_SUCH_IRQ_LINE is returned if the IRQ number of the timer is 
     * not supported.
     */
    OS_RETURN_E (*disable)(void);

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

/** @brief NULL timer driver. */
extern kernel_timer_t null_timer;

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
                      const kernel_timer_t* rtc_timer,
                      const kernel_timer_t* aux_timer);

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
void time_main_timer_handler(cpu_state_t* cpu_state, uintptr_t int_id,
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
void time_rtc_timer_handler(cpu_state_t* cpu_state, uintptr_t int_id,
                            stack_state_t* stack);

/**
 * @brief The kernel's auxiliary timer interrupt handler.
 * 
 * @details The kernel's auxiliary timer interrupt handler. This must be 
 * connected to an auxiliary timer of the system.
 * 
 * @param[in, out] cpu_state The cpu registers before the interrupt.
 * @param[in] int_id The interrupt line that called the handler.
 * @param[in, out] stack The stack state before the interrupt.
 */
void time_aux_timer_handler(cpu_state_t* cpu_state, uintptr_t int_id,
                            stack_state_t* stack);

/** 
 * @brief Returns the current uptime.
 * 
 * @details Return the current uptime of the system in ms.
 *
 * @return The current uptime in ms.
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

#endif /* #ifndef __TIME_TIME_MANAGEMENT_H_ */