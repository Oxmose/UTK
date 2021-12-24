/*******************************************************************************
 * @file memcpy.c
 *
 * @see string.h
 *
 * @author Alexy Torres Aurora Dugo
 *
 * @date 03/10/2017
 *
 * @version 1.0
 *
 * @brief memcpy function. To be used with string.h header.
 *
 * @details memcpy function. To be used with string.h header.
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

void *memcpy(void *dst, const void *src, size_t n)
{
    const char *p = src;
    char *q = dst;
#if defined(__i386__)
    size_t nl = n >> 2;
    __asm__ __volatile__ ("cld ; rep ; movsl ; movl %3,%0 ; rep ; movsb":"+c"
              (nl),
              "+S"(p), "+D"(q)
              :"r"(n & 3));
#elif defined(__x86_64__)
    size_t nq = n >> 3;
    __asm__ __volatile__ ("cld ; rep ; movsq ; movl %3,%%ecx ; rep ; movsb":"+c"
              (nq), "+S"(p), "+D"(q)
              :"r"((uint32_t) (n & 7)));
#else
    while (n--) {
        *q++ = *p++;
    }
#endif

    return dst;
}

/************************************ EOF *************************************/
