/*******************************************************************************
 * @file rtc.h
 * 
 * @see rtc.c
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

#ifndef __X86_RTC_H_
#define __X86_RTC_H_

#include <stdint.h>          /* Generic int types */
#include <stddef.h>          /* Standard definitions */
#include <interrupts.h>      /* Interrupts management */
#include <time_management.h> /* Timer factory */
#include <rt_clock.h>        /* Generic RTC driver */

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

/*******************************************************************************
 * FUNCTIONS
 ******************************************************************************/

/**
 * @brief Returns the RTC IRQ number.
 * 
 * @details Returns the RTC IRQ number.
 * 
 * @return The RTC IRQ number.
 */
uint32_t rtc_get_irq(void);

#endif /* #ifndef __X86_RTC_H_ */