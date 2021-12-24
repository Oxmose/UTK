/*******************************************************************************
 * @file printf.c
 *
 * @see string.h
 *
 * @author Alexy Torres Aurora Dugo
 *
 * @date 03/10/2017
 *
 * @version 1.0
 *
 * @brief printf function and derivatives . To be used with stdio.h header.
 *
 * @details printf function and derivatives . To be used with stdio.h header.
 *
 * @copyright Alexy Torres Aurora Dugo
 ******************************************************************************/

/*******************************************************************************
 * INCLUDES
 ******************************************************************************/

/* Included headers */
#include <stddef.h>        /* Standard definitions */
#include <kernel_output.h> /* Kernel output API */
#include <graphic.h>       /* Kernel graphic API */

/* Configuration files */
#include <config.h>
#include <test_bank.h>

/* Header file */
#include <stdio.h>

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
/* None */

/************************* Exported global variables **************************/
/* None */

/************************** Static global variables ***************************/
/* None */

/*******************************************************************************
 * STATIC FUNCTIONS DECLARATIONS
 ******************************************************************************/

/* None */

/*******************************************************************************
 * FUNCTIONS
 ******************************************************************************/

int vprintf(const char *fmt, __builtin_va_list args)
{
    kernel_doprint(fmt, args);

    return 0;
}

int printf(const char *fmt, ...)
{
    __builtin_va_list    args;
    int                  err;

    __builtin_va_start(args, fmt);

    err = vprintf(fmt, args);

    __builtin_va_end(args);

    return err;
}

/************************************ EOF *************************************/