/*******************************************************************************
 * @file udivmoddi4.c
 *
 * @see stdlib.h
 *
 * @author Alexy Torres Aurora Dugo
 *
 * @date 03/10/2017
 *
 * @version 1.0
 *
 * @brief __udivmoddi4 function. To be used with stdlib.h header.
 *
 * @details __udivmoddi4 function. To be used with stdlib.h header.
 *
 * @copyright Alexy Torres Aurora Dugo
 ******************************************************************************/

/*******************************************************************************
 * INCLUDES
 ******************************************************************************/

/* Included headers */
#include <stdint.h> /* Generic int types */

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

/**
 * @brief This function returns the quotient of the unsigned division of num
 * and den.
 *
 * @param[in] num The numerator.
 * @param[in] den The denominator.
 * @param[out] rem_p The reminder buffer.
 *
 * @return The quotient of the unsigned division of num and den is returned.
 */
uint64_t __udivmoddi4(uint64_t num, uint64_t den, uint64_t* rem_p);

/*******************************************************************************
 * FUNCTIONS
 ******************************************************************************/

uint64_t __udivmoddi4(uint64_t num, uint64_t den, uint64_t* rem_p)
{
    uint64_t quot = 0;
    uint64_t qbit = 1;

    if(den == 0)
    {
        /* Force divide by 0 */
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

/************************************ EOF *************************************/