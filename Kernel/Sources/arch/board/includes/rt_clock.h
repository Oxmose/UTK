/*******************************************************************************
 * @file rt_clock.h
 * 
 * @see rt_clock.c
 *
 * @author Alexy Torres Aurora Dugo
 *
 * @date 09/03/2021
 *
 * @version 1.0
 *
 * @brief RTC (Real Time Clock) driver.
 * 
 * @details RTC (Real Time Clock) driver. Used as the kernel's time base. Timer
 * source in the kernel. This driver provides basic access to the RTC.
 * 
 * @copyright Alexy Torres Aurora Dugo
 ******************************************************************************/

#ifndef __BSP_RTC_H_
#define __BSP_RTC_H_

#include <stdint.h>          /* Generic int types */
#include <stddef.h>          /* Standard definitions */
#include <interrupts.h>      /* Interrupts management */
#include <time_management.h> /* Timer factory */

/*******************************************************************************
 * CONSTANTS
 ******************************************************************************/

/*******************************************************************************
 * STRUCTURES
 ******************************************************************************/

/** @brief RTC date structure. */
struct date
{
    /** @brief Day of the week. */
    uint16_t weekday;
    /** @brief Day of the month. */
    uint16_t day;
    /** @brief Month of the year. */
    uint16_t month;
    /** @brief Current year. */
    uint16_t year;
};

/** 
 * @brief Defines date_t type as a shorcut for struct date.
 */
typedef struct date date_t;

/** @brief RTC driver instance. */
extern kernel_timer_t rtc_driver;

/*******************************************************************************
 * FUNCTIONS
 ******************************************************************************/

/**
 * @brief Initializes the RTC settings and interrupt management.
 * 
 * @details Initializes RTC settings, sets the RTC interrupt manager and enables 
 * interrupts for the RTC.
 *
 * @return The success state or the error code. 
 * - OS_NO_ERR is returned if no error is encountered. 
 * - OS_ERR_NO_SUCH_IRQ_LINE is returned if the IRQ number of the RTC is not 
 *   supported.
 */
OS_RETURN_E rtc_init(void);

/**
 * @brief Enables RTC ticks.
 * 
 * @details Enables RTC ticks by clearing the RTC's IRQ mask.
 */
void rtc_enable(void);

/**
 * @brief Disables RTC ticks.
 * 
 * @details Disables RTC ticks by setting the RTC's IRQ mask.
 */
void rtc_disable(void);

/** 
 * @brief Sets the RTC's tick frequency.
 * 
 * @details Sets the RTC's tick frequency. The value must be between 2Hz and 
 * 8192Hz.
 * 
 * @warning The value must be between 2Hz and 8192Hz. The lower boundary RTC
 * frequency will be selected (refer to the code to understand the 14 available
 * frequencies).
 *
 * @param[in] frequency The new frequency to be set to the RTC.
 */
void rtc_set_frequency(const uint32_t frequency);

/**
 * @brief Returns the RTC tick frequency in Hz.
 * 
 * @details Returns the RTC tick frequency in Hz.
 * 
 * @return The RTC tick frequency in Hz.
 */
uint32_t rtc_get_frequency(void);

/**
 * @brief Sets the RTC tick handler.
 *
 * @details Sets the RTC tick handler. This function will be called at each RTC
 * tick received.
 * 
 * @param[in] handler The handler of the RTC interrupt.
 * 
 * @return The success state or the error code. 
 * - OS_NO_ERR is returned if no error is encountered. 
 * - OS_ERR_NULL_POINTER is returned if the handler is NULL.
 * - OR_ERR_UNAUTHORIZED_INTERRUPT_LINE is returned if the RTC interrupt line
 *   is not allowed. 
 * - OS_ERR_NULL_POINTER is returned if the pointer to the handler is NULL. 
 * - OS_ERR_INTERRUPT_ALREADY_REGISTERED is returned if a handler is already 
 *   registered for the RTC.
 */
OS_RETURN_E rtc_set_handler(void(*handler)(
                                 cpu_state_t*,
                                 uintptr_t,
                                 stack_state_t*
                                 ));

/**
 * @brief Removes the RTC tick handler.
 *
 * @details Removes the RTC tick handler. 
 * 
 * @return The success state or the error code. 
 * - OS_NO_ERR is returned if no error is encountered. 
 * - OR_ERR_UNAUTHORIZED_INTERRUPT_LINE is returned if the RTC interrupt line
 * is not allowed. 
 * - OS_ERR_INTERRUPT_NOT_REGISTERED is returned if the RTC line has no handler
 * attached.
 */
OS_RETURN_E rtc_remove_handler(void);

/**
 * @brief Returns the current date.
 * 
 * @details Returns the current date in RTC date format.
 *
 * @returns The current date in in RTC date format
 */
date_t rtc_get_current_date(void);

/**
 * @brief Returns the current daytime in seconds.
 * 
 * @details Returns the current daytime in seconds.
 *
 * @returns The current daytime in seconds.
 */
uint32_t rtc_get_current_daytime(void);

/**
 * @brief Updates the system's time and date.
 * 
 * @details Updates the system's time and date. This function also reads the 
 * CMOS registers. By doing that, the RTC registers are cleaned and the RTC able
 * to interrupt the CPU again.
 * 
 * @warning You MUST call that function in every RTC handler or the RTC will 
 * never raise interrupt again.
 */
void rtc_update_time(void);

#endif /* #ifndef __X86_RTC_H_ */