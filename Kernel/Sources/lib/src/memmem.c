/*******************************************************************************
 *
 * File: memmem.c
 *
 * Author: Alexy Torres Aurora Dugo
 *
 * Date: 03/10/2017
 *
 * Version: 1.0
 *
 * memmem function. To be used with string.h header.
 *
 ******************************************************************************/

#include <stddef.h> /* size_t */

/* Header file */
#include <string.h>

void *memmem(const void *haystack, size_t n, const void *needle, size_t m)
{
    const unsigned char *y = (const unsigned char *)haystack;
    const unsigned char *x = (const unsigned char *)needle;

    if (m > n || !m || !n)
        return NULL;

    if (1 != m) {
        size_t j, k, l;
        
        if (x[0] == x[1]) {
            k = 2;
            l = 1;
        } else {
            k = 1;
            l = 2;
        }

        j = 0;
        while (j <= n - m) {
            if (x[1] != y[j + 1]) {
                j += k;
            } else {
                if (!memcmp(x + 2, y + j + 2, m - 2)
                    && x[0] == y[j])
                    return (void *)&y[j];
                j += l;
            }
        }
    } else
        do {
            if (*y == *x)
                return (void *)y;
            y++;
        } while (--n);

    return NULL;
}
