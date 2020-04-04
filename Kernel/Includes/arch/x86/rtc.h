/*******************************************************************************
 * @file rtc.h
 * 
 * @see rtc.c
 *
 * @author Alexy Torres Aurora Dugo
 *
 * @date 04/10/2017
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

#ifndef __X86_RTC_H_
#define __X86_RTC_H_

#include <lib/stdint.h>           /* Generic int types */
#include <lib/stddef.h>           /* Standard definitions */
#include <interrupt/interrupts.h> /* Interrupts management */
#include <time/time_management.h> /* Timer factory */

/*******************************************************************************
 * CONSTANTS
 ******************************************************************************/

/* RTC settings */
/** @brief Initial RTC tick rate. */
#define RTC_INIT_RATE 10
/** @brief RTC minimal frequency. */
#define RTC_MIN_FREQ 2
/** @brief RTC maximal frequency. */
#define RTC_MAX_FREQ 8192

/** @brief RTC quartz frequency. */
#define RTC_QUARTZ_FREQ 32768

/* CMOS registers  */
/** @brief CMOS seconds register id. */
#define CMOS_SECONDS_REGISTER 0x00
/** @brief CMOS minutes register id. */
#define CMOS_MINUTES_REGISTER 0x02
/** @brief CMOS hours register id. */
#define CMOS_HOURS_REGISTER   0x04
/** @brief CMOS day of the week register id. */
#define CMOS_WEEKDAY_REGISTER 0x06
/** @brief CMOS day register id. */
#define CMOS_DAY_REGISTER     0x07
/** @brief CMOS month register id. */
#define CMOS_MONTH_REGISTER   0x08
/** @brief CMOS year register id. */
#define CMOS_YEAR_REGISTER    0x09
/** @brief CMOS century register id. */
#define CMOS_CENTURY_REGISTER 0x00

/* CMOS setings */
/** @brief CMOS NMI disabler bit. */
#define CMOS_NMI_DISABLE_BIT 0x01
/** @brief CMOS RTC enabler bit. */
#define CMOS_ENABLE_RTC      0x40
/** @brief CMOS A register id. */
#define CMOS_REG_A           0x0A
/** @brief CMOS B register id. */
#define CMOS_REG_B           0x0B
/** @brief CMOS C register id. */
#define CMOS_REG_C           0x0C

/** @brief CMOS CPU command port id. */
#define CMOS_COMM_PORT 0x70
/** @brief CMOS CPU data port id. */
#define CMOS_DATA_PORT 0x71

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
 *
 * @return The success state or the error code. 
 * - OS_NO_ERR is returned if no error is encountered. 
 * - OS_ERR_NO_SUCH_IRQ_LINE is returned if the IRQ number of the RTC is not 
 *   supported.
 */
OS_RETURN_E rtc_enable(void);

/**
 * @brief Disables RTC ticks.
 * 
 * @details Disables RTC ticks by setting the RTC's IRQ mask.
 *
 * @return The success state or the error code. 
 * - OS_NO_ERR is returned if no error is encountered. 
 * - OS_ERR_NO_SUCH_IRQ_LINE is returned if the IRQ number of the RTC is not 
 *   supported.
 */
OS_RETURN_E rtc_disable(void);

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
 * 
 * @return The success state or the error code. 
 * - OS_NO_ERR is returned if no error is detected. 
 * - OS_ERR_OUT_OF_BOUND is returned if the frequency is out of bounds.
 * - OS_ERR_NO_SUCH_IRQ_LINE is returned if the IRQ number of the RTC is not 
 *   supported.
 */
OS_RETURN_E rtc_set_frequency(const uint32_t frequency);

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

/**
 * @brief Returns the RTC IRQ number.
 * 
 * @details Returns the RTC IRQ number.
 * 
 * @return The RTC IRQ number.
 */
uint32_t rtc_get_irq(void);

#endif /* #ifndef __X86_RTC_H_ */