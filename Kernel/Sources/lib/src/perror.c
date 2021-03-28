/*******************************************************************************
 *
 * File: perror.c
 *
 * Author: Alexy Torres Aurora Dugo
 *
 * Date: 03/10/2017
 *
 * Version: 1.0
 *
 * perror function. Used to print OS error codes in a string way using the error
 * code.
 *
 ******************************************************************************/

#include <stddef.h> /* size_t */

/* Header file */
#include <stdio.h>

int perror(const int error)
{
    switch(error)
    {
        default:
            printf("Unknown error");
    }

    return 0;
}
