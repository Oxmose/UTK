/*******************************************************************************
 * @file graphic.h
 *
 * @see graphic.c
 *
 * @author Alexy Torres Aurora Dugo
 *
 * @date 28/02/2021
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

/*******************************************************************************
 * INCLUDES
 ******************************************************************************/

#include <stdint.h>       /* Generic int types */
#include <stddef.h>       /* Standard definition */
#include <kernel_error.h> /* Kernel error codes */

/*******************************************************************************
 * CONSTANTS
 ******************************************************************************/

/* None */

/*******************************************************************************
 * STRUCTURES AND TYPES
 ******************************************************************************/

/**
 * @brief Scroll direction enumeration, enumerates all the possible scrolling
 * direction supported by the driver.
 */
typedef enum
{
    /** @brief Scroll down direction. */
    SCROLL_DOWN,
    /** @brief Scroll up direction. */
    SCROLL_UP
} SCROLL_DIRECTION_E;

/**
 * @brief Screen cursor representation for the driver. The structures contains
 * the required data to keep track of the current cursor's position.
 */
typedef struct
{
    /** @brief The x position of the cursor. */
    uint32_t x;
    /** @brief The y position of the cursor. */
    uint32_t y;
} cursor_t;

/**
 * @brief Screen color scheme representation. Keeps the different display format
 * such as background color in memory.
 */
typedef struct
{
    /** @brief The foreground color to be used when outputing data. */
    uint32_t foreground;
    /** @brief The background color to be used when outputing data. */
    uint32_t background;

    /**
     * @brief Set to 1 if using the VGA color designation for foreground and
     * background. If set to 0, then regular 32 bits RGBA designation is used.
     */
    bool_t vga_color;
} colorscheme_t;

/**
 * @brief The kernel's graphic driver abstraction.
 */
typedef struct
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
     */
    void (*put_cursor_at)(const uint32_t line, const uint32_t column);

    /**
     * @brief Saves the cursor attributes in the buffer given as paramter.
     *
     * @details Fills the buffer given s parameter with the current value of the
     * cursor.
     *
     * @param[out] buffer The cursor buffer in which the current cursor position
     * is going to be saved.
     */
    void (*save_cursor)(cursor_t* buffer);

    /**
     * @brief Restores the cursor attributes from the buffer given as parameter.
     *
     * @details The function will restores the cursor attributes from the buffer
     * given as parameter.
     *
     * @param[in] buffer The buffer containing the cursor's attributes.
     */
    void (*restore_cursor)(const cursor_t buffer);

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
     */
    void (*save_color_scheme)(colorscheme_t* buffer);

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
} kernel_graphic_driver_t;

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
 */
void graphic_put_cursor_at(const uint32_t line, const uint32_t column);

/**
 * @brief Saves the cursor attributes in the buffer given as paramter.
 *
 * @details Fills the buffer given s parameter with the current value of the
 * cursor.
 *
 * @param[out] buffer The cursor buffer in which the current cursor position is
 * going to be saved.
 */
void graphic_save_cursor(cursor_t* buffer);

/**
 * @brief Restores the cursor attributes from the buffer given as parameter.
 *
 * @details The function will restores the cursor attributes from the buffer
 * given as parameter.
 *
 * @param[in] buffer The buffer containing the cursor's attributes.
 */
void graphic_restore_cursor(const cursor_t buffer);

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
 */
void graphic_save_color_scheme(colorscheme_t* buffer);

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

/************************************ EOF *************************************/