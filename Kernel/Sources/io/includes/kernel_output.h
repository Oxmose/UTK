/*******************************************************************************
 * @file kernel_output.h
 * 
 * @see kernel_output.c
 *
 * @author Alexy Torres Aurora Dugo
 *
 * @date 27/02/2021
 *
 * @version 2.0
 *
 * @brief Kernel's output methods.
 * 
 * @details Simple output functions to print messages to screen. These are 
 * really basic output too allow early kernel boot output and debug. These 
 * functions can be used in interrupts handlers since no lock is required to use
 * them. This also makes them non thread safe.
 * 
 * @copyright Alexy Torres Aurora Dugo
 ******************************************************************************/

#ifndef __IO_KERNEL_OUTPUT_H_
#define __IO_KERNEL_OUTPUT_H_

/*******************************************************************************
 * CONSTANTS
 ******************************************************************************/

/* None */

/*******************************************************************************
 * STRUCTURES
 ******************************************************************************/

/* None */

/*******************************************************************************
 * FUNCTIONS
 ******************************************************************************/

/* Defines the output macros that can be enabled or diabled at compile time */
#if KERNEL_LOG_LEVEL >= INFO_LOG_LEVEL
#define KERNEL_INFO(...)    kernel_info(__VA_ARGS__)
#define KERNEL_SUCCESS(...) kernel_success(__VA_ARGS__)
#else 
#define KERNEL_INFO(...) 
#define KERNEL_SUCCESS(...)
#endif

#if KERNEL_LOG_LEVEL >= ERROR_LOG_LEVEL
#define KERNEL_ERROR(...) kernel_error(__VA_ARGS__)
#else 
#define KERNEL_ERROR(...)
#endif

#if KERNEL_LOG_LEVEL >= DEBUG_LOG_LEVEL
#define KERNEL_DEBUG(MODULE, STR, ...) { 									   \
	if(MODULE) 																   \
	{																		   \
		kernel_uart_debug(STR " | " __FILE__ ":%d\n", ##__VA_ARGS__, __LINE__);\
	}																		   \
}
	
#else 
#define KERNEL_DEBUG(...)
#endif

/**
 * @brief Prints a formated string to the screen.
 * 
 * @details Prints the desired string to the screen. This uses the generic 
 * graphic driver to output data.
 *
 * @param[in] fmt The format string to output.
 * @param[in] ... format's parameters.
 */
void kernel_printf(const char *fmt, ...);

/**
 * @brief Prints the desired string to the screen. 
 * 
 * @details Prints the desired string to the screen. Adds a red [ERROR] tag at 
 * the beggining of the string before printing it. This uses the generic 
 * graphic driver to output data.
 *
 * @param fmt The format string to output.
 * @param[in] ... format's parameters.
 */
void kernel_error(const char *fmt, ...);

/**
 * @brief Prints the desired string to the screen. 
 * 
 * @details Prints the desired string to the screen. Adds a green [OK] tag at 
 * the beggining of the string before printing it. This uses the generic 
 * graphic driver to output data.
 *
 * @param fmt The format string to output.
 * @param[in] ... format's parameters.
 */
void kernel_success(const char *fmt, ...);

/**
 * @brief Prints the desired string to the screen. 
 * 
 * @details Prints the desired string to the screen. Adds a cyan [INFO] tag at 
 * the beggining of the string before printing it. This uses the generic 
 * graphic driver to output data.
 *
 * @param fmt The format string to output.
 * @param[in] ... format's parameters.
 */
void kernel_info(const char *fmt, ...);

/**
 * @brief Prints the desired string to the uart port. 
 * 
 * @details Prints the desired string to the uart port. Adds a [DEBUG] tag at 
 * the beggining of the string before outputing it.
 * 
 * @param fmt The format string to output.
 * @param[in] ... format's parameters.
 */
void kernel_uart_debug(const char *fmt, ...);

/**
 * @brief Prints a string to the screen attached to the arguments list.
 * 
 * @details Prints the desired string to the screen with the argument list given 
 * as parameter, this is conveniently be used by printf.
 *
 * @warning This function should only be called when the kernel is fully
 * initialized.
 *
 * @param[in] str The format string to output.
 * @param[in] args The arguments list.
 */
void kernel_doprint(const char* str, __builtin_va_list args);

#endif /* #ifndef __IO_KERNEL_OUTPUT_H_ */
