/*******************************************************************************
 * @file uart.h
 *
 * @author Alexy Torres Aurora Dugo
 *
 * @date 28/02/2021
 *
 * @version 1.0
 *
 * @brief UART communication driver.
 *
 * @details UART communication driver. Initializes the uart ports as in and
 * output. The uart can be used to output data or communicate with other
 * prepherals that support this communication method
 *
 * @copyright Alexy Torres Aurora Dugo
 ******************************************************************************/

#ifndef __BOARD_UART_H_
#define __BOARD_UART_H_

/*******************************************************************************
 * INCLUDES
 ******************************************************************************/

#include <stdint.h>       /* Generic int types */
#include <kernel_error.h> /* Kernel error codes */
#include <graphic.h>      /* Graphic driver manager */

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
 * FUNCTIONS
 ******************************************************************************/

/**
 * @brief Initializes the uart driver structures and hardware.
 *
 * @details Initializes all the uart communication ports supported by the
 * driver and ennables the interrupt related to the uart hardware.
 *
 */
void uart_init(void);

/**
 * @brief Write the string given as patameter on the debug port.
 *
 * @details The function will output the data given as parameter on the debug
 * port. This call is blocking until the data has been sent to the uart port
 * controler.
 *
 * @param[in] string The string to write to the uart port.
 *
 * @warning string must be NULL terminated.
 */
void uart_put_string(const char* string);

/**
 * @brief Write the character given as patameter on the debug port.
 *
 * @details The function will output the character given as parameter on the
 * debug port. This call is blocking until the data has been sent to the uart
 * port controler.
 *
 * @param[in] character The character to write to the uart port.
 */
void uart_put_char(const char character);

/**
 * @brief Tells if the data on the uart port are ready to be read.
 *
 * @details The function will returns 1 if a data was received by the uart
 * port referenced by the port given as parameter.
 *
 * @param[in] port The uart port on which the test should be executed.
 *
 * @return TRUE is returned if data can be read from the port. FALSE is returned
 * otherwise.
 */
bool_t uart_received(const uint32_t port);

/**
 * @brief Reads a byte from the uart port given as parameter.
 *
 * @details The function will read the input data on the selected port. This
 * call is blocking until the data has been received by the uart port
 * controler.
 *
 * @param[in] port The port on which the data should be read.
 *
 * @return The byte that has been read on the uart port.
 */
uint8_t uart_read(const uint32_t port);

/**
 * @brief Clears the screen.
 *
 * @details On 80x25 uart screen, this function will print 80 line feeds
 * and thus, clear the screen.
 */
void uart_clear_screen(void);

/**
 * @brief Scrolls the screen downn.
 *
 * @details Scrolls the screen by printing lines feed to the uart.
 * This function can only be called with parameter direction to
 * SCROLL_DOWN. Otherwise, this function has no effect.
 *
 * @param[in] direction Should always be SCROLL_DOWN.
 *
 * @param[in] lines_count The amount of lines to scroll down.
 */
void uart_scroll(const SCROLL_DIRECTION_E direction,
                 const uint32_t lines_count);

/**
 * @brief Write the string given as patameter on the debug port.
 *
 * @details The function will output the data given as parameter on the debug
 * port. This call is blocking until the data has been sent to the uart port
 * controler.
 *
 * @param[in] string The string to write to the uart port.
 * @param[in] len The length of the string to print.
 */
void uart_console_write_keyboard(const char* string, const size_t len);

/**
 * @brief Returns the UART graphic driver.
 *
 * @details Returns a constant handle to the UART graphic driver.
 *
 * @return A constant handle to the UART graphic driver is returned.
 */
const kernel_graphic_driver_t* uart_get_driver(void);

#endif /* #ifndef __BOARD_UART_H_ */

/************************************ EOF *************************************/