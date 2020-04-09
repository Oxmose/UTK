/*******************************************************************************
 * @file stdio.h
 *
 * @author Alexy Torres Aurora Dugo
 *
 * @date 04/10/2017
 *
 * @version 1.0
 *
 * @brief Kernel's intput output definitions.
 * 
 * @details Standard input output file header
 * 
 * @copyright Alexy Torres Aurora Dugo
 ******************************************************************************/

#ifndef __STDIO_H_
#define __STDIO_H_

/**
 * @brief Prints a string corresponding to the error given as parameter.
 * 
 * @details Prints a string corresponding to the error given as parameter.
 * 
 * @param[in] error The error code to print the description of.
 * 
 * @return 0 in all cases.
 */
int perror(const int error);

/**
 * @brief Prints a formated string.
 * 
 * @details Prints a formated string with the attached parameters to be included
 * in the string.
 * 
 * @param[in] __format The format string to print.
 * @param[in] ... The arguments attached to the format string.
 * 
 * @return 0 in all cases.
 */
int printf(const char *__format, ...) __attribute__((format (printf, 1, 2)));

/**
 * @brief Prints a formated string.
 * 
 * @details Prints a formated string with the attached parameters to be included
 * in the string.
 * 
 * @param[in] __format The format string to print.
 * @param[in] __vl The arguments attached to the format string.
 * 
 * @return 0 in all cases.
 */
int vprintf(const char *__format, __builtin_va_list __vl) 
            __attribute__((format (printf, 1, 0)));

/**
 * @brief Prints a formated string into a buffer.
 * 
 * @details Prints a formated string into a buffer with the attached parameters 
 * to be included in the string.
 * 
 * @param[out] __dest The buffer to print the string into.
 * @param[in] __format The format string to print.
 * @param[in] __vl The arguments attached to the format string.
 * 
 * @return 0 in all cases.
 */
int vsprintf(char *__dest, const char *__format, __builtin_va_list __vl) 
             __attribute__((format (printf, 2, 0)));

/**
 * @brief Prints a formated string into a buffer.
 * 
 * @details Prints a formated string into a buffer with the attached parameters 
 * to be included in the string.
 * 
 * @param[out] __dest The buffer to print the string into.
 * @param[in] __size The maximal size of the buffer.
 * @param[in] __format The format string to print.
 * @param[in] __vl The arguments attached to the format string.
 * 
 * @return 0 in all cases.
 */
int vsnprintf(char *__dest, unsigned int __size, const char *__format, 
              __builtin_va_list __vl) 
              __attribute__((format (printf, 3, 0)));

#endif /* __STDIO_H_ */