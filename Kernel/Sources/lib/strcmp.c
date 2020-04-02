/*******************************************************************************
 *
 * File: strcmp.c
 *
 * Author: Alexy Torres Aurora Dugo
 *
 * Date: 03/10/2017
 *
 * Version: 1.0
 *
 * strcmp function. To be used with string.h header.
 *
 ******************************************************************************/

/* Header file */
#include <lib/string.h>

int strcmp(const char *s1, const char *s2)
{
    const unsigned char *c1 = (const unsigned char *)s1;
    const unsigned char *c2 = (const unsigned char *)s2;
    int d = 0;

    while (1) {
        unsigned char ch;
        d = (int)(ch = *c1++) - (int)*c2++;
        if (d || !ch)
            break;
    }

    return d;
}
