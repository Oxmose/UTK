/*******************************************************************************
 * @file graphic.c
 *
 * @see graphic.h
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

#include <lib/stdint.h>  /* Generic int types */
#include <lib/stddef.h>  /* Standard definitions */
#include <serial.h>      /* Serial drivers */

/* UTK configuration file */
#include <config.h>

/* Header */
#include <io/graphic.h>

/*******************************************************************************
 * GLOBAL VARIABLES
 ******************************************************************************/

/** @brief Stores the currently selected driver. Default is serial text driver */
static kernel_graphic_driver_t graphic_driver = 
{
    .clear_screen = serial_clear_screen,
    .put_cursor_at = serial_put_cursor_at,
    .save_cursor = serial_save_cursor,
    .restore_cursor = serial_restore_cursor,
    .scroll = serial_scroll,
    .set_color_scheme = serial_set_color_scheme,
    .save_color_scheme = serial_save_color_scheme,
    .put_string = serial_put_string,
    .put_char = serial_put_char,
    .console_write_keyboard = serial_console_write_keyboard
};

/*******************************************************************************
 * FUNCTIONS
 ******************************************************************************/

OS_RETURN_E graphic_set_selected_driver(const kernel_graphic_driver_t* driver)
{
    if(driver == NULL ||
       driver->clear_screen == NULL ||
       driver->put_cursor_at == NULL ||
       driver->save_cursor == NULL ||
       driver->restore_cursor == NULL ||
       driver->scroll == NULL ||
       driver->set_color_scheme == NULL ||
       driver->save_color_scheme == NULL ||
       driver->put_string == NULL ||
       driver->put_char == NULL ||
       driver->console_write_keyboard == NULL)
    {
        return OS_ERR_NULL_POINTER;
    }

	graphic_driver = *driver;

    return OS_NO_ERR;
}

const kernel_graphic_driver_t* graphic_get_selected_driver(void)
{
    return &graphic_driver;
}

void graphic_clear_screen(void)
{
	graphic_driver.clear_screen();
}

OS_RETURN_E graphic_put_cursor_at(const uint32_t line, const uint32_t column)
{
	return graphic_driver.put_cursor_at(line, column);
}

OS_RETURN_E graphic_save_cursor(cursor_t* buffer)
{
	return graphic_driver.save_cursor(buffer);
}

OS_RETURN_E graphic_restore_cursor(const cursor_t buffer)
{
	return graphic_driver.restore_cursor(buffer);
}

void graphic_scroll(const SCROLL_DIRECTION_E direction,
                    const uint32_t lines_count)
{
	graphic_driver.scroll(direction, lines_count);
}

void graphic_set_color_scheme(colorscheme_t color_scheme)
{
	graphic_driver.set_color_scheme(color_scheme);
}

OS_RETURN_E graphic_save_color_scheme(colorscheme_t* buffer)
{
	return graphic_driver.save_color_scheme(buffer);
}

void graphic_put_string(const char* str)
{
	graphic_driver.put_string(str);
}

void graphic_put_char(const char character)
{
    graphic_driver.put_char(character);
}

void graphic_console_write_keyboard(const char* str, const size_t len)
{
	graphic_driver.console_write_keyboard(str, len);
}