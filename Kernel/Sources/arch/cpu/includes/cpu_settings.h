/*******************************************************************************
 * @file cpu_settings.h
 *
 * @see cpu_settings.c
 *
 * @author Alexy Torres Aurora Dugo
 *
 * @date 27/02/2021
 *
 * @version 1.0
 *
 * @brief CPU settings routines.
 *
 * @details CPU settings routines.
 *
 * @copyright Alexy Torres Aurora Dugo
 ******************************************************************************/

#ifndef __CPU_SETTINGS_H_
#define __CPU_SETTINGS_H_

/*******************************************************************************
 * INCLUDES
 ******************************************************************************/

#include <config.h> /* Configuration file */

#ifdef ARCH_I386
#include <../i386/includes/cpu_settings.h>
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

#endif /* #ifndef __CPU_SETTINGS_H_ */

/* EOF */