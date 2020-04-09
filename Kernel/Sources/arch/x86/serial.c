/*******************************************************************************
 * @file serial.c
 *
 * @see serial.h
 *
 * @author Alexy Torres Aurora Dugo
 *
 * @date 25/12/2017
 *
 * @version 1.0
 *
 * @brief Serial communication driver.
 *
 * @details Serial communication driver. Initializes the serial ports as in and
 * output. The serial can be used to output data or communicate with other
 * prepherals that support this communication method. Only COM1 to COM4 are
 * supported by this driver.
 *
 * @copyright Alexy Torres Aurora Dugo
 *
 * @warning Only COM1 and COM2 are initialized for input.
 ******************************************************************************/

#include <lib/stddef.h>       /* Standard definitions */
#include <lib/stdint.h>       /* Generic int types */
#include <lib/string.h>       /* String manipulation */
#include <cpu.h>              /* CPU manipulation */
#include <io/graphic.h>       /* Graphic definitions */
#include <io/kernel_output.h> /* Kernel output methods */

/* UTK configuration file */
#include <config.h>

/* Tests header file */
#if TEST_MODE_ENABLED
#include <Tests/test_bank.h>
#endif

/* Header */
#include <serial.h>

/*******************************************************************************
 * GLOBAL VARIABLES
 ******************************************************************************/

/** @brief Stores the serial initialization state. */
static uint8_t serial_init_done = 0;

/**
 * @brief Serial text driver instance.
 */
kernel_graphic_driver_t serial_text_driver = 
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

/**
 * @brief Sets line parameters for the desired port.
 *
 * @details Sets line parameters for the desired port.
 *
 * @param[in] attr The settings for the port's line.
 * @param[in] com The port to set.
 *
 * @return OS_NO_ERR on success, no other value is returned.
 */
static OS_RETURN_E set_line(const uint8_t attr, const uint8_t com)
{
    cpu_outb(attr, SERIAL_LINE_COMMAND_PORT(com));

#if SERIAL_KERNEL_DEBUG == 1
    kernel_serial_debug("[SERIAL] Set line attributes of port 0x%04x to %u\n", 
                         com, attr);
#endif

    return OS_NO_ERR;
}

/**
 * @brief Sets buffer parameters for the desired port.
 *
 * @details Sets buffer parameters for the desired port.
 *
 * @param[in] attr The settings for the port's buffer.
 * @param[in] com The port to set.
 *
 * @return OS_NO_ERR on success, no other value is returned.
 */
static OS_RETURN_E set_buffer(const uint8_t attr, const uint8_t com)
{
    cpu_outb(attr, SERIAL_FIFO_COMMAND_PORT(com));

#if SERIAL_KERNEL_DEBUG == 1
    kernel_serial_debug("[SERIAL] Set buffer attributes of port 0x%04x to %u\n", 
                         com, attr);
#endif

    return OS_NO_ERR;
}

/**
 * @brief Sets the port's baudrate.
 *
 * @details Sets the port's baudrate.
 *
 * @param[in] rate The desired baudrate for the port.
 * @param[in] com The port to set.
 *
 * @return OS_NO_ERR on success, no other value is returned.
 */
static OS_RETURN_E set_baudrate(SERIAL_BAUDRATE_E rate, const uint8_t com)
{
    cpu_outb(SERIAL_DLAB_ENABLED, SERIAL_LINE_COMMAND_PORT(com));
    cpu_outb((rate >> 8) & 0x00FF, SERIAL_DATA_PORT(com));
    cpu_outb(rate & 0x00FF, SERIAL_DATA_PORT_2(com));

#if SERIAL_KERNEL_DEBUG == 1
    kernel_serial_debug("[SERIAL] Set baud rate of port 0x%04x to %u\n", 
                         com, rate);
#endif

    return OS_NO_ERR;
}

OS_RETURN_E serial_init(void)
{
    OS_RETURN_E err;
    uint8_t i;

    /* Init all comm ports */
    for(i = 0; i < 4; ++i)
    {
        uint8_t  attr;
        uint32_t com;

        if(i == 0)
        {
            com = SERIAL_COM1_BASE;
        }
        else if(i == 1)
        {
            com = SERIAL_COM2_BASE;
        }
        else if(i == 2)
        {
            com = SERIAL_COM3_BASE;
        }
        else if(i == 3)
        {
            com = SERIAL_COM4_BASE;
        }
        else
        {
            com = SERIAL_COM1_BASE;
        }

        attr = SERIAL_DATA_LENGTH_8 | SERIAL_STOP_BIT_1;

        /* Enable interrupt on recv for COM1 and COM2 */
        if(com == SERIAL_COM1_BASE || com == SERIAL_COM2_BASE)
        {
            cpu_outb(0x01, SERIAL_DATA_PORT_2(com));
        }
        else
        {
            cpu_outb(0x00, SERIAL_DATA_PORT_2(com));
        }

        /* Init baud rate */
        err = set_baudrate(BAUDRATE_9600, com);
        if(err != OS_NO_ERR)
        {
            return err;
        }

        /* Configure the line */
        err = set_line(attr, com);
        if(err != OS_NO_ERR)
        {
            return err;
        }

        err = set_buffer(0xC0 | SERIAL_ENABLE_FIFO | SERIAL_CLEAR_RECV_FIFO |
                         SERIAL_CLEAR_SEND_FIFO | SERIAL_FIFO_DEPTH_14,
                         com);
        if(err != OS_NO_ERR)
        {
            return err;
        }

        /* Enable interrupt */
        cpu_outb(0x0B, SERIAL_MODEM_COMMAND_PORT(com));
    }

    serial_init_done = 1;

#if SERIAL_KERNEL_DEBUG == 1
    kernel_serial_debug("[SERIAL] Serial initialization end\n");
#endif

#if TEST_MODE_ENABLED
    serial_test();
#endif

    return err;
}

void serial_write(const uint32_t port, const uint8_t data)
{
    if(serial_init_done == 0)
    {
        return;
    }
    if(port != COM1 && port != COM2 && port != COM3 && port != COM4)
    {
        return;
    }

    /* Wait for empty transmit */
    while((cpu_inb(SERIAL_LINE_STATUS_PORT(port)) & 0x20) == 0)
    {}

    if(data == '\n')
    {
        serial_write(port, '\r');
        cpu_outb('\n', port);
    }
    else
    {
        cpu_outb(data, port);
    }

    while((cpu_inb(SERIAL_LINE_STATUS_PORT(port)) & 0x20) == 0)
    {}
}


void serial_clear_screen(void)
{
    uint8_t i;
    /* On 80x25 screen, just print 25 line feed. */
    for(i = 0; i < 25; ++i)
    {
        serial_write(SERIAL_DEBUG_PORT, '\n');
    }
}

OS_RETURN_E serial_put_cursor_at(const uint32_t line, const uint32_t column)
{
    (void)line;
    (void)column;
    /* Nothing to do here */
    return OS_ERR_NOT_SUPPORTED;
}

OS_RETURN_E serial_save_cursor(cursor_t* buffer)
{
    (void)buffer;
    /* Nothing to do here */
    return OS_ERR_NOT_SUPPORTED;
}

OS_RETURN_E serial_restore_cursor(const cursor_t buffer)
{
    (void)buffer;
    /* Nothing to do here */
    return OS_ERR_NOT_SUPPORTED;
}

void serial_scroll(const SCROLL_DIRECTION_E direction,
                   const uint32_t lines_count)
{
    uint32_t i;
    if(direction == SCROLL_DOWN)
    {
        /* Just print lines_count line feed. */
        for(i = 0; i < lines_count; ++i)
        {
            serial_write(SERIAL_DEBUG_PORT, '\n');
        }
    }    
}

void serial_set_color_scheme(const colorscheme_t color_scheme)
{
    (void)color_scheme;
}

OS_RETURN_E serial_save_color_scheme(colorscheme_t* buffer)
{
    (void)buffer;

    return OS_ERR_NOT_SUPPORTED;
}

void serial_console_write_keyboard(const char* str, const size_t len)
{
    size_t i;
    for(i = 0; i < len; ++i)
    {
        serial_write(SERIAL_DEBUG_PORT, str[i]);
    }
}

uint8_t serial_read(const uint32_t port)
{
    /* Wait for data to be received */
    while (serial_received(port) == 0);

    /* Read available data on port */
    uint8_t val = cpu_inb(SERIAL_DATA_PORT(port));

    return val;
}

void serial_put_string(const char* string)
{
    size_t i;
    for(i = 0; i < strlen(string); ++i)
    {
        serial_write(SERIAL_DEBUG_PORT, string[i]);
    }
}

void serial_put_char(const char character)
{
    serial_write(SERIAL_DEBUG_PORT, character);
}

uint8_t serial_received(const uint32_t port)
{
    /* Read on LINE status port */
    return cpu_inb(SERIAL_LINE_STATUS_PORT(port)) & 0x01;
}
