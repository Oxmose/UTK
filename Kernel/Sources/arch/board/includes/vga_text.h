/*******************************************************************************
 * @file vga_text.h
 *
 * @see vga_text.c
 *
 * @author Alexy Torres Aurora Dugo
 *
 * @date 28/02/2021
 *
 * @version 1.5
 *
 * @brief VGA text mode driver.
 *
 * @details Allows the kernel to display text and general ASCII characters to be
 * displayed on the screen. Includes cursor management, screen colors management
 * and other fancy screen driver things.
 *
 * @copyright Alexy Torres Aurora Dugo
 ******************************************************************************/

#ifndef __X86_VGA_TEXT_H_
#define __X86_VGA_TEXT_H_

/*******************************************************************************
 * INCLUDES
 ******************************************************************************/

#include <stdint.h>       /* Generic int types */
#include <kernel_error.h> /* Kernel error codes */
#include <graphic.h>      /* Graphic driver manager */

/*******************************************************************************
 * CONSTANTS
 ******************************************************************************/

/** @brief VGA background color definition: black. */
#define BG_BLACK            0x00
/** @brief VGA background color definition: blue. */
#define BG_BLUE             0x10
/** @brief VGA background color definition: green. */
#define BG_GREEN            0x20
/** @brief VGA background color definition: cyan. */
#define BG_CYAN             0x30
/** @brief VGA background color definition: red. */
#define BG_RED              0x40
/** @brief VGA background color definition: magenta. */
#define BG_MAGENTA          0x50
/** @brief VGA background color definition: brown. */
#define BG_BROWN            0x60
/** @brief VGA background color definition: grey. */
#define BG_GREY             0x70
/** @brief VGA background color definition: dark grey. */
#define BG_DARKGREY         0x80
/** @brief VGA background color definition: bright blue. */
#define BG_BRIGHTBLUE       0x90
/** @brief VGA background color definition: bright green. */
#define BG_BRIGHTGREEN      0xA0
/** @brief VGA background color definition: bright cyan. */
#define BG_BRIGHTCYAN       0xB0
/** @brief VGA background color definition: bright red. */
#define BG_BRIGHTRED        0xC0
/** @brief VGA background color definition: bright magenta. */
#define BG_BRIGHTMAGENTA    0xD0
/** @brief VGA background color definition: yellow. */
#define BG_YELLOW           0xE0
/** @brief VGA background color definition: white. */
#define BG_WHITE            0xF0

/** @brief VGA foreground color definition: black. */
#define FG_BLACK            0x00
/** @brief VGA foreground color definition: blue. */
#define FG_BLUE             0x01
/** @brief VGA foreground color definition: green. */
#define FG_GREEN            0x02
/** @brief VGA foreground color definition: cyan. */
#define FG_CYAN             0x03
/** @brief VGA foreground color definition: red. */
#define FG_RED              0x04
/** @brief VGA foreground color definition: magenta. */
#define FG_MAGENTA          0x05
/** @brief VGA foreground color definition: brown. */
#define FG_BROWN            0x06
/** @brief VGA foreground color definition: grey. */
#define FG_GREY             0x07
/** @brief VGA foreground color definition: dark grey. */
#define FG_DARKGREY         0x08
/** @brief VGA foreground color definition: bright blue. */
#define FG_BRIGHTBLUE       0x09
/** @brief VGA foreground color definition: bright green. */
#define FG_BRIGHTGREEN      0x0A
/** @brief VGA foreground color definition: bright cyan. */
#define FG_BRIGHTCYAN       0x0B
/** @brief VGA foreground color definition: bright red. */
#define FG_BRIGHTRED        0x0C
/** @brief VGA foreground color definition: bright magenta. */
#define FG_BRIGHTMAGENTA    0x0D
/** @brief VGA foreground color definition: yellow. */
#define FG_YELLOW           0x0E
/** @brief VGA foreground color definition: white. */
#define FG_WHITE            0x0F

/** @brief BIOS call interrupt id to set VGA mode. */
#define BIOS_INTERRUPT_VGA          0x10
/** @brief BIOS call id to set 80x25 vga text mode. */
#define BIOS_CALL_SET_VGA_TEXT_MODE 0x03

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

/* Imported global variables */
/* None */

/* Exported global variables */
/* None */

/* Static global variables */
/* None */

/*******************************************************************************
 * FUNCTIONS
 ******************************************************************************/

/**
 * @brief Initializes the VGA driver.
 *
 * @details Initializes the VGA driver by enabling VGA related exceptions
 * and memory management.
 */
void vga_init(void);

/**
 * @brief Clears the screen by printing space character on black background.
 */
void vga_clear_screen(void);

/**
 * @brief Places the cursor to the selected coordinates given as parameters.
 *
 * @details Places the screen cursor to the coordinated described with the
 * parameters. The function will check the boundaries or the position parameters
 * before setting the cursor position.
 *
 * @param[in] line The line index where to place the cursor.
 * @param[in] column The column index where to place the cursor.
 */
void vga_put_cursor_at(const uint32_t line, const uint32_t column);

/**
 * @brief Saves the cursor attributes in the buffer given as parameter.
 *
 * @details Fills the buffer given as parrameter with the current cursor
 * settings.
 *
 * @param[out] buffer The cursor buffer in which the current cursor possition is
 * going to be saved.
 */
void vga_save_cursor(cursor_t* buffer);

/**
 * @brief Restores the cursor attributes from the buffer given as parameter.
 *
 * @details The function will restores the cursor attributes from the buffer
 * given as parameter.
 *
 * @param[in] buffer The cursor buffer containing the new coordinates of the
 * cursor.
 */
void vga_restore_cursor(const cursor_t buffer);

/**
 * @brief Scrolls in the desired direction of lines_count lines.
 *
 * @details The function will scroll of lines_count line in the desired
 * direction.
 *
 * @param[in] direction The direction to whoch the console should be scrolled.
 * @param[in] lines_count The number of lines to scroll.
 */
void vga_scroll(const SCROLL_DIRECTION_E direction, const uint32_t lines_count);

/**
 * @brief Sets the color scheme of the screen.
 *
 * @details Replaces the curent color scheme used t output data with the new
 * one given as parameter.
 *
 * @param[in] color_scheme The new color scheme to apply to the screen console.
 */
void vga_set_color_scheme(const colorscheme_t color_scheme);

/**
 * @brief Saves the color scheme in the buffer given as parameter.
 *
 * @details Fills the buffer given as parameter with the current screen's
 * color scheme value.
 *
 * @param[out] buffer The buffer that will receive the current color scheme used
 * by the screen console.
 */
void vga_save_color_scheme(colorscheme_t* buffer);

/**
 * ­@brief Put a string to screen.
 *
 * @details The function will display the string given as parameter to the
 * screen.
 *
 * @param[in] string The string to display on the screen.
 *
 * @warning string must be NULL terminated.
 */
void vga_put_string(const char* string);

/**
 * ­@brief Put a character to screen.
 *
 * @details The function will display the character given as parameter to the
 * screen.
 *
 * @param[in] character The char to display on the screen.
 */
void vga_put_char(const char character);

/**
 * @brief Used by the kernel to display strings on the screen from a keyboard
 * input.
 *
 * @details Display a character from the keyboard input. This allows
 * the kernel to know these character can be backspaced later.
 *
 * @param[in] str The string to display on the screen from a keybaord input.
 * @param[in] len The length of the string to display.
 */
void vga_console_write_keyboard(const char* str, const size_t len);

/**
 * @brief Returns the VGA graphic driver.
 *
 * @details Returns a constant handle to the VGA graphic driver.
 *
 * @return A constant handle to the VGA graphic driver is returned.
 */
const kernel_graphic_driver_t* vga_text_get_driver(void);


#endif /* #ifndef __X86_VGA_TEXT_H_ */

/* EOF */
