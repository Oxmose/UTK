/*******************************************************************************
 * @file stdlib.h
 *
 * @author Alexy Torres Aurora Dugo
 *
 * @date 04/10/2017
 *
 * @version 1.0
 *
 * @brief Kernel's standard lib functions.
 * 
 * @details Standard lib functions.
 * 
 * @copyright Alexy Torres Aurora Dugo
 ******************************************************************************/

#ifndef __STDLIB_H_
#define __STDLIB_H_

#include <lib/stddef.h> /* Generic int types */

/*******************************************************************************
 * CONSTANTS
 ******************************************************************************/
 
/*******************************************************************************
 * STRUCTURES
 ******************************************************************************/

/*******************************************************************************
 * FUNCTIONS
 ******************************************************************************/

/** 
 * @brief Convert a signed integer value to a string.
 * 
 * @details Convert a signed integer value to a string and inject the conversion 
 * result in the buffer given as parameter.
 * 
 * @param[in] value The value to convert.
 * @param[out] buf The buffer to receive the convertion's result.
 * @param[in] base The base of the signed integer to convert.
 */
void itoa(int64_t value, char* buf, uint32_t base);

/** 
 * @brief Convert a unsigned integer value to a string.
 * 
 * @details Convert a unsigned integer value to a string and inject the 
 * conversion result in the buffer given as parameter.
 * 
 * @param[in] value The value to convert.
 * @param[out] buf The buffer to receive the convertion's result.
 * @param[in] base The base of the unsigned integer to convert.
 */
void uitoa(uint64_t value, char* buf, uint32_t base);

#endif /* __STDLIB_H_ */
