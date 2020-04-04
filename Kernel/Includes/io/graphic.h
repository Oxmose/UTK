/*******************************************************************************
 * @file graphic.h
 * 
 * @see graphic.c
 *
 * @author Alexy Torres Aurora Dugo
 *
 * @date 04/01/2018
 *
 * @version 1.0
 *
 * @brief Graphic drivers abtraction.
 * 
 * @details Graphic driver abtraction layer. The functions of this module allows
 * to abtract the use of any supported graphic driver and the selection of the
 * desired driver.
 * 
 * @copyright Alexy Torres Aurora Dugo
 ******************************************************************************/

#ifndef __IO_GRAPHIC_H_
#define __IO_GRAPHIC_H_

#include <lib/stdint.h> /* Generic int types */
#include <lib/stddef.h> /* Standard definition */

/*******************************************************************************
 * DEFINITIONS
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
 * STRUCTURES
 ******************************************************************************/

/**
 * @brief Screen cursor representation for the driver. The structures contains 
 * the required data to keep track of the current cursor's position.
 */
struct cursor
{
    /** @brief The x position of the cursor. */
    uint32_t x;
    /** @brief The y position of the cursor. */
    uint32_t y;
};

/** 
 * @brief Defines cursor_t type as a shorcut for struct cursor.
 */
typedef struct cursor cursor_t;

/**
 * @brief Scroll direction enumeration, enumerates all the possible scrolling
 * direction supported by the driver.
 */
enum SCROLL_DIRECTION
{
    /** @brief Scroll down direction. */
    SCROLL_DOWN,
    /** @brief Scroll up direction. */
    SCROLL_UP
};

/** 
 * @brief Defines SCROLL_DIRECTION_E type as a shorcut for enum 
 * SCROLL_DIRECTION.
 */
typedef enum SCROLL_DIRECTION SCROLL_DIRECTION_E;

/**
 * @brief Screen color scheme representation. Keeps the different display format
 * such as background color in memory.
 */
struct colorscheme
{
    /** @brief The foreground color to be used when outputing data. */
    uint32_t foreground;
    /** @brief The background color to be used when outputing data. */
    uint32_t background;

    /** 
     * @brief Set to 1 if using the VGA color designation for foreground and
     * background. If set to 0, then regular 32 bits RGBA designation is used.
     */
    uint32_t vga_color;
};

/** 
 * @brief Defines colorscheme_t type as a shorcut for struct colorscheme.
 */
typedef struct colorscheme colorscheme_t;

/** 
 * @brief The kernel's graphic driver abstraction.
 */
struct kernel_graphic_driver
{
    /**
     * @brief Clears the screen, the background color is set to black.
     */
    void (*clear_screen)(void);

    /** 
     * @brief Places the cursor to the coordinates given as parameters.
     * 
     * @details The function places the screen cursor at the desired coordinates
     * based on the line and column parameter.
     *
     * @param[in] line The line index where to place the cursor.
     * @param[in] column The column index where to place the cursor.
     * 
     * @return The success state or the error code.
     * - OS_NO_ERR is returned if no error is encountered.  
     * - OS_ERR_OUT_OF_BOUND is returned if the parameters are out of bound.
     */
    OS_RETURN_E (*put_cursor_at)(const uint32_t line, const uint32_t column);

    /**
     * @brief Saves the cursor attributes in the buffer given as paramter.
     * 
     * @details Fills the buffer given s parameter with the current value of the 
     * cursor.
     *
     * @param[out] buffer The cursor buffer in which the current cursor position 
     * is going to be saved.
     * 
     * @return The success state or the error code. 
     * - OS_NO_ERR is returned if no error is encountered. 
     * - OS_ERR_NULL_POINTER is returned if the buffer pointer is NULL.
     */
    OS_RETURN_E (*save_cursor)(cursor_t* buffer);

    /**
     * @brief Restores the cursor attributes from the buffer given as parameter.
     *
     * @details The function will restores the cursor attributes from the buffer 
     * given as parameter.
     * 
     * @param[in] buffer The buffer containing the cursor's attributes.
     * 
     * @return The success state or the error code. 
     * - OS_NO_ERR is returned if no error is encountered. 
     * - OS_ERR_OUT_OF_BOUND is returned if the positions of the buffer are out 
     * of bound.
     */
    OS_RETURN_E (*restore_cursor)(const cursor_t buffer);

    /**
     * @brief Scrolls in the desired direction of lines_count lines.
     * 
     * @details The function will use the driver to scroll of lines_count line 
     * in the desired direction.
     *
     * @param[in] direction The direction to which the screen should be 
     * scrolled.
     * @param[in] lines_count The number of lines to scroll.
     */
    void (*scroll)(const SCROLL_DIRECTION_E direction,
                   const uint32_t lines_count);

    /**
     * @brief Sets the color scheme of the screen.
     * 
     * @details Replaces the curent color scheme used t output data with the new
     * one given as parameter.
     *
     * @param[in] color_scheme The new color scheme to apply to the screen.
     */
    void (*set_color_scheme)(const colorscheme_t color_scheme);

    /** 
     * @brief Saves the color scheme in the buffer given as parameter.
     * 
     * @details Fills the buffer given as parameter with the current screen's 
     * color scheme value.
     *
     * @param[out] buffer The buffer that will receive the current color scheme 
     * used by the screen.
     * 
     * @return The success state or the error code. 
     * - OS_NO_ERR is returned if no error is encountered. 
     * - OS_ERR_NULL_POINTER is returned if the buffer pointer is NULL.
     */
    OS_RETURN_E (*save_color_scheme)(colorscheme_t* buffer);

    /**
     * ­@brief Put a string to screen.
     * 
     * @details The function will display the string given as parameter to the 
     * screen using the selected driver.
     *
     * @param[in] str The string to display on the screen.
     * 
     * @warning string must be NULL terminated.
     */
    void (*put_string)(const char* str);

    /**
     * ­@brief Put a character to screen.
     * 
     * @details The function will display the character given as parameter to 
     * the screen using the selected driver.
     * 
     * @param[in] character The char to display on the screen.
     */
    void (*put_char)(const char character);

    /**
     * @brief Used by the kernel to display strings on the screen from a 
     * keyboard input.
     * 
     * @details Display a character from the keyboard input. This allows 
     * the kernel to know these character can be backspaced later.
     *
     * @param[in] str The string to display on the screen from a keybaord input.
     * @param[in] len The length of the string to display.
     */
    void (*console_write_keyboard)(const char* str, const size_t len);
};

/** 
 * @brief Defines kernel_graphic_driver_t type as a shorcut for struct 
 * kernel_graphic_driver.
 */
typedef struct kernel_graphic_driver kernel_graphic_driver_t;

/*******************************************************************************
 * FUNCTIONS
 ******************************************************************************/

/**
 * @brief Sets the current selected driver.
 *
 * @details Changes the current selected driver to ouput data with the new one
 * as defined by the parameter driver.
 * 
 * @param[in] driver The driver to select.
 * 
 * @return The success state or the error code.
 * - OS_NO_ERR is returned if no error is encountered.
 * - OS_ERR_NULL_POINTER if the graphic driver is NULL or has NULL function 
 * pointers. 
 */
OS_RETURN_E graphic_set_selected_driver(const kernel_graphic_driver_t* driver);

/**
 * @brief Returns the current graphic driver used in the kernel.
 *
 * @details Returns the current graphic driver used in the kernel.
 * 
 * @return The current graphic driver used in the kernel.
 */
const kernel_graphic_driver_t* graphic_get_selected_driver(void);

/**
 * @brief Clears the screen, the background color is set to black.
 */
void graphic_clear_screen(void);

/** 
 * @brief Places the cursor to the coordinates given as parameters.
 * 
 * @details The function places the screen cursor at the desired coordinates
 * based on the line and column parameter.
 *
 * @param[in] line The line index where to place the cursor.
 * @param[in] column The column index where to place the cursor.
 * 
 * @return The success state or the error code.
 * - OS_NO_ERR is returned if no error is encountered.  
 * - OS_ERR_OUT_OF_BOUND is returned if the parameters are out of bound.
 */
OS_RETURN_E graphic_put_cursor_at(const uint32_t line, const uint32_t column);

/**
 * @brief Saves the cursor attributes in the buffer given as paramter.
 * 
 * @details Fills the buffer given s parameter with the current value of the 
 * cursor.
 *
 * @param[out] buffer The cursor buffer in which the current cursor position is
 * going to be saved.
 * 
 * @return The success state or the error code. 
 * - OS_NO_ERR is returned if no error is encountered. 
 * - OS_ERR_NULL_POINTER is returned if the buffer pointer is NULL.
 */
OS_RETURN_E graphic_save_cursor(cursor_t* buffer);

/**
 * @brief Restores the cursor attributes from the buffer given as parameter.
 *
 * @details The function will restores the cursor attributes from the buffer 
 * given as parameter.
 * 
 * @param[in] buffer The buffer containing the cursor's attributes.
 * 
 * @return The success state or the error code. 
 * - OS_NO_ERR is returned if no error is encountered. 
 * - OS_ERR_UNAUTHORIZED_ACTION is returned if the positions of the buffer are
 * out of bound.
 */
OS_RETURN_E graphic_restore_cursor(const cursor_t buffer);

/**
 * @brief Scrolls in the desired direction of lines_count lines.
 * 
 * @details The function will use the driver to scroll of lines_count line in
 * the desired direction.
 *
 * @param[in] direction The direction to which the screen should be scrolled.
 * @param[in] lines_count The number of lines to scroll.
 */
void graphic_scroll(const SCROLL_DIRECTION_E direction,
                    const uint32_t lines_count);

/**
 * @brief Sets the color scheme of the screen.
 * 
 * @details Replaces the curent color scheme used t output data with the new
 * one given as parameter.
 *
 * @param[in] color_scheme The new color scheme to apply to the screen.
 */
void graphic_set_color_scheme(const colorscheme_t color_scheme);

/** 
 * @brief Saves the color scheme in the buffer given as parameter.
 * 
 * @details Fills the buffer given as parameter with the current screen's 
 * color scheme value.
 *
 * @param[out] buffer The buffer that will receive the current color scheme used 
 * by the screen.
 * 
 * @return The success state or the error code. 
 * - OS_NO_ERR is returned if no error is encountered. 
 * - OS_ERR_NULL_POINTER is returned if the buffer pointer is NULL.
 */
OS_RETURN_E graphic_save_color_scheme(colorscheme_t* buffer);

/**
 * ­@brief Put a string to screen.
 * 
 * @details The function will display the string given as parameter to the 
 * screen using the selected driver.
 *
 * @param[in] str The string to display on the screen.
 * 
 * @warning string must be NULL terminated.
 */
void graphic_put_string(const char* str);

/**
 * ­@brief Put a character to screen.
 * 
 * @details The function will display the character given as parameter to the 
 * screen using the selected driver.
 * 
 * @param[in] character The char to display on the screen.
 */
void graphic_put_char(const char character);

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
void graphic_console_write_keyboard(const char* str, const size_t len);

#endif /* #ifndef __IO_GRAPHIC_H_ */
