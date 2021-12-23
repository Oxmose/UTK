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

#ifndef __BOARD_INTERRUPT_SETTINGS_H_
#define __BOARD_INTERRUPT_SETTINGS_H_

/*******************************************************************************
 * INCLUDES
 ******************************************************************************/

#include <config.h> /* UTK configuration file */

#ifdef ARCH_I386
#include <../x86/includes/x86_interrupt_settings.h>
#else
#error Unknown CPU architecture
#endif

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

/* Imported global variables */
/* None */

/* Exported global variables */
/* None */

/* Static global variables */
/* None */

/*******************************************************************************
 * FUNCTIONS
 ******************************************************************************/

/* None */

#endif /* #ifndef __BOARD_INTERRUPT_SETTINGS_H_ */

/* EOF */