/*******************************************************************************
 * @file iota.c
 *
 * @see stdlib.h
 *
 * @author Alexy Torres Aurora Dugo
 *
 * @date 08/01/2018
 *
 * @version 1.0
 *
 * @brief itoa function. To be used with stdlib.h header.
 *
 * @details itoa function. To be used with stdlib.h header.
 *
 * @copyright Alexy Torres Aurora Dugo
 ******************************************************************************/

/*******************************************************************************
 * INCLUDES
 ******************************************************************************/

/* Included headers */
#include <stdint.h> /* Generic integer definitions */

/* Configuration files */
#include <config.h>
#include <test_bank.h>

/* Header file */
#include <stdlib.h>

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

void itoa(int64_t i, char* buf, uint32_t base)
{
    /* If base is unknown just return */
    if (base > 16)
    {
        return;
    }

    /* Check sign */
    if (base == 10 && i < 0)
    {
        *buf++ = '-';
        i *= -1;
    }

    /* To the job */
    uitoa(i, buf, base);
}

/************************************ EOF *************************************/