/*******************************************************************************
 * @file strlcat.c
 *
 * @see string.h
 *
 * @author Alexy Torres Aurora Dugo
 *
 * @date 03/10/2017
 *
 * @version 1.0
 *
 * @brief strlcat function. To be used with string.h header.
 *
 * @details strlcat function. To be used with string.h header.
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

size_t strlcat(char *dst, const char *src, size_t size)
{
    size_t bytes = 0;
    char *q = dst;
    const char *p = src;
    char ch;

    while (bytes < size && *q) {
        q++;
        bytes++;
    }
    if (bytes == size)
        return (bytes + strlen(src));

    while ((ch = *p++)) {
        if (bytes + 1 < size)
            *q++ = ch;

        bytes++;
    }

    *q = '\0';
    return bytes;
}

/************************************ EOF *************************************/
