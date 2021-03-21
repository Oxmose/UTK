/*******************************************************************************
 * @file graphic.c
 *
 * @see graphic.h
 *
 * @author Alexy Torres Aurora Dugo
 *
 * @date 27/02/2021
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

#include <stdint.h>  /* Generic int types */
#include <stddef.h>  /* Standard definitions */

/* UTK configuration file */
#include <config.h>

/* Header */
#include <graphic.h>

/*******************************************************************************
 * GLOBAL VARIABLES
 ******************************************************************************/

/** @brief Stores the currently selected driver */
static kernel_graphic_driver_t graphic_driver = {NULL};

/*******************************************************************************
 * FUNCTIONS
 ******************************************************************************/

OS_RETURN_E graphic_set_selected_driver(const kernel_graphic_driver_t* driver)
{
    if(driver == NULL ||
       driver->clear_screen == NULL ||
       driver->put_string == NULL ||
       driver->put_char == NULL ||
       driver->console_write_keyboard == NULL)
    {
        return OS_ERR_NULL_POINTER;
    }

	graphic_driver = *driver;

    /* On driver change, clear the output */
    graphic_clear_screen();

    return OS_NO_ERR;
}

const kernel_graphic_driver_t* graphic_get_selected_driver(void)
{
    return &graphic_driver;
}

void graphic_clear_screen(void)
{
    if(graphic_driver.clear_screen != NULL)
    {
	    graphic_driver.clear_screen();
    }
}

void graphic_put_cursor_at(const uint32_t line, const uint32_t column)
{
    if(graphic_driver.put_cursor_at != NULL)
    {
	    graphic_driver.put_cursor_at(line, column);
    }
}

void graphic_save_cursor(cursor_t* buffer)
{
	if(graphic_driver.save_cursor != NULL)
    {
	    return graphic_driver.save_cursor(buffer);
    }
}

void graphic_restore_cursor(const cursor_t buffer)
{
	if(graphic_driver.restore_cursor != NULL)
    {
	    return graphic_driver.restore_cursor(buffer);
    }
}

void graphic_scroll(const SCROLL_DIRECTION_E direction,
                    const uint32_t lines_count)
{
	if(graphic_driver.scroll != NULL)
    {
	    graphic_driver.scroll(direction, lines_count);
    }
}

void graphic_set_color_scheme(colorscheme_t color_scheme)
{
	if(graphic_driver.set_color_scheme != NULL)
    {
	    graphic_driver.set_color_scheme(color_scheme);
    }
}

void graphic_save_color_scheme(colorscheme_t* buffer)
{
	if(graphic_driver.save_color_scheme != NULL)
    {
	    return graphic_driver.save_color_scheme(buffer);
    }
}

void graphic_put_string(const char* str)
{
	if(graphic_driver.put_string != NULL)
    {
	    graphic_driver.put_string(str);
    }
}

void graphic_put_char(const char character)
{
    if(graphic_driver.put_char != NULL)
    {
	    graphic_driver.put_char(character);
    }
}

void graphic_console_write_keyboard(const char* str, const size_t len)
{
	if(graphic_driver.console_write_keyboard != NULL)
    {
	    graphic_driver.console_write_keyboard(str, len);
    }
}