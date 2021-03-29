/*******************************************************************************
 *
 * File: uiota.c
 *
 * Author: Alexy Torres Aurora Dugo
 *
 * Date: 08/01/2018
 *
 * Version: 1.0
 *
 * uitoa function. To be used with stdlib.h header.
 *
 ******************************************************************************/

/* Header file */
#include <string.h>

/* Header include */
#include <stdlib.h>

/*******************************************************************************
 * GLOBAL VARIABLES
 ******************************************************************************/

static char hex_table[] =
     {'0','1','2','3','4','5','6','7','8','9','A','B','C','D','E','F'};

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
