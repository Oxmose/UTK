/*******************************************************************************
 * @file strxspn.c
 *
 * @see string.h
 *
 * @author Alexy Torres Aurora Dugo
 *
 * @date 03/10/2017
 *
 * @version 1.0
 *
 * @brief __strxspn function. To be used with string.h header.
 *
 * @details __strxspn function. To be used with string.h header.
 *
 * @copyright Alexy Torres Aurora Dugo
 ******************************************************************************/

/*******************************************************************************
 * INCLUDES
 ******************************************************************************/

/* Included headers */
#include <stddef.h> /* Standard definitions */

/* Configuration files */
#include <config.h>
#include <test_bank.h>

/* Header file */
#include <string.h>

/*******************************************************************************
 * CONSTANTS
 ******************************************************************************/

#define UCHAR_MAX 255U

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

size_t __strxspn(const char *s, const char *map, int parity)
{
    char matchmap[UCHAR_MAX + 1];
    size_t n = 0;

    /* Create bitmap */
    memset(matchmap, 0, sizeof matchmap);
    while (*map)
        matchmap[(unsigned char)*map++] = 1;

    /* Make sure the null character never matches */
    matchmap[0] = parity;

    /* Calculate span length */
    while (matchmap[(unsigned char)*s++] ^ parity)
        n++;

    return n;
}

/************************************ EOF *************************************/
