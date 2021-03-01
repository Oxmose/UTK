/*******************************************************************************
 * @file interrupt_settings.h
 *
 * @see interrupt_settings.c
 * 
 * @author Alexy Torres Aurora Dugo
 *
 * @date 27/02/2021
 *
 * @version 1.0
 *
 * @brief Interrupt settings definitions.
 *
 * @details Interrupt settings definitions.
 *
 * @copyright Alexy Torres Aurora Dugo
 ******************************************************************************/

#ifndef __INTERRUPT_SETTINGS_
#define __INTERRUPT_SETTINGS_

#include <config.h> /* Configuration file */

#ifdef ARCH_I386
#include <../x86/includes/interrupt_settings.h>
#else 
#error Unknown CPU architecture
#endif

#endif /* #ifndef __INTERRUPT_SETTINGS_ */