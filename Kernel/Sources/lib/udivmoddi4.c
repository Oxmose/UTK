/*******************************************************************************
 *
 * File: udivmoddi4.c
 *
 * Author: Alexy Torres Aurora Dugo
 *
 * Date: 08/01/2018
 *
 * Version: 1.0
 *
 * __udivmoddi4 function. To be used with stdlib.h header.
 *
 ******************************************************************************/

#include <lib/stdint.h> /* Generic int types */

uint64_t __udivmoddi4 (uint64_t num, uint64_t den, uint64_t* rem_p)
{
    uint64_t quot = 0;
    uint64_t qbit = 1;
 
    if(den == 0) 
    {
        return 1 / ((unsigned)den);
    }
 
    /* Left-justify denominator and count shift */
    while((int64_t)den >= 0) 
    {
        den <<= 1;
        qbit <<= 1;
    }
 
    while(qbit) 
    {
        if(den <= num) 
        {
            num -= den;
            quot += qbit;
        }
        den >>= 1;
        qbit >>= 1;
    }
 
    if(rem_p)
    {
        *rem_p = num;
    }
 
    return quot;
}