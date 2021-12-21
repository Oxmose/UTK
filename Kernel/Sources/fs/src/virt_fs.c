/*******************************************************************************
 * @file virt_fs.c
 *
 * @see virt_fs.h
 *
 * @author Alexy Torres Aurora Dugo
 *
 * @date 27/03/2021
 *
 * @version 1.0
 *
 * @brief Kernel's virtual filesystem driver.
 *
 * @details Kernel's virtual filesystem driver. Defines the functions and
 * structures used by the kernel to manage the virtual filesystem.
 *
 * @copyright Alexy Torres Aurora Dugo
 ******************************************************************************/

#include <kernel_error.h>  /* Kernel error codes */
#include <stdint.h>        /* Generic int types */
#include <stddef.h>        /* Standard definitions */
#include <string.h>        /* Memory manipulation */
#include <kernel_output.h> /* Kernel output methods */

/* UTK configuration file */
#include <config.h>

/* Tests header file */
#include <test_bank.h>

/* Header file */
#include <virt_fs.h>

/*******************************************************************************
 * CONSTANTS
 ******************************************************************************/

/* None */

/*******************************************************************************
 * STRUCTURES
 ******************************************************************************/

/* None */

/*******************************************************************************
 * GLOBAL VARIABLES
 ******************************************************************************/

/* None */

/*******************************************************************************
 * STATIC FUNCTIONS DECLARATION
 ******************************************************************************/

/* None */

/*******************************************************************************
 * FUNCTIONS
 ******************************************************************************/
