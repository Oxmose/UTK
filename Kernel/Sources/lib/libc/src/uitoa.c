/*******************************************************************************
 * @file uiota.c
 *
 * @see stdlib.h
 *
 * @author Alexy Torres Aurora Dugo
 *
 * @date 08/01/2018
 *
 * @version 1.0
 *
 * @brief uitoa function. To be used with stdlib.h header.
 *
 * @details uitoa function. To be used with stdlib.h header.
 *
 * @copyright Alexy Torres Aurora Dugo
 ******************************************************************************/


/*******************************************************************************
 * INCLUDES
 ******************************************************************************/

/* Included headers */
#include <stdint.h> /* Generic integer types */

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

/**
 * @brief Hexadecimal characters table.
 */
static char hex_table[] =
     {'0','1','2','3','4','5','6','7','8','9','A','B','C','D','E','F'};

/*******************************************************************************
 * STATIC FUNCTIONS DECLARATIONS
 ******************************************************************************/

/* None */

/*******************************************************************************
 * FUNCTIONS
 ******************************************************************************/

void uitoa(uint64_t i, char* buf, uint32_t base)
{
    char tmp[128];

    uint32_t pos  = 0;
    uint32_t opos = 0;
    uint32_t top  = 0;

    if (i == 0 || base > 16)
    {
        *buf++ = '0';
        *buf = '\0';
        return;
    }

    /* Fill temp buffer */
    while (i != 0)
    {
        tmp[pos++] = hex_table[i % base];
        i /= base;
    }

    top = pos--;
    /* Fill buffer */
    for (opos = 0; opos < top; --pos, ++opos)
    {
        buf[opos] = tmp[pos];
    }

    /* Null termitate */
    buf[opos] = 0;
}

/************************************ EOF *************************************/