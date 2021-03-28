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

#include <stdint.h> /* Generic int types */

/*******************************************************************************
 * CONSTANTS
 ******************************************************************************/

/* None */

/*******************************************************************************
 * STRUCTURES
 ******************************************************************************/

/* None */

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