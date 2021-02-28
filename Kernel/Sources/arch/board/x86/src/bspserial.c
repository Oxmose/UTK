/*******************************************************************************
 * @file bspserial.c
 *
 * @see bspserial.h
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

#include <stddef.h>        /* Standard definitions */
#include <stdint.h>        /* Generic int types */
#include <string.h>        /* String manipulation */
#include <cpu.h>           /* CPU manipulation */
#include <graphic.h>       /* Graphic definitions */
#include <kernel_output.h> /* Kernel output methods */
#include <uart.h>          /* UART main header */

/* UTK configuration file */
#include <config.h>

/* Tests header file */
#if TEST_MODE_ENABLED
#include <Tests/test_bank.h>
#endif

/* Header */
#include <bspserial.h>

/*******************************************************************************
 * GLOBAL VARIABLES
 ******************************************************************************/

/** @brief Stores the serial initialization state. */
static uint8_t serial_init_done = 0;

/**
 * @brief Serial text driver instance.
 */
kernel_graphic_driver_t uart_text_driver = 
{
    .clear_screen = uart_clear_screen,
    .put_cursor_at = NULL,
    .save_cursor = NULL,
    .restore_cursor = NULL,
    .scroll = uart_scroll,
    .set_color_scheme = NULL,
    .save_color_scheme = NULL,
    .put_string = uart_put_string,
    .put_char = uart_put_char,
    .console_write_keyboard = uart_console_write_keyboard
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

    KERNEL_DEBUG("[SERIAL] Set line attributes of port 0x%04x to %u\n", 
                 com, attr);

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

    KERNEL_DEBUG("[SERIAL] Set buffer attributes of port 0x%04x to %u\n", 
                  com, attr);

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

    KERNEL_DEBUG("[SERIAL] Set baud rate of port 0x%04x to %u\n", com, rate);

    return OS_NO_ERR;
}

OS_RETURN_E uart_init(void)
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

    KERNEL_DEBUG("[SERIAL] Serial initialization end\n");

#if TEST_MODE_ENABLED
    uart_test();
#endif

    return err;
}

void uart_write(const uint32_t port, const uint8_t data)
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
    while((cpu_inb(SERIAL_LINE_STATUS_PORT(port)) & 0x20) == 0);
    if(data == '\n')
    {
        uart_write(port, '\r');
        cpu_outb('\n', port);
    }
    else
    {
        cpu_outb(data, port);
    }

    while((cpu_inb(SERIAL_LINE_STATUS_PORT(port)) & 0x20) == 0);
}


void uart_clear_screen(void)
{
    uint8_t i;
    /* On 80x25 screen, just print 25 line feed. */
    for(i = 0; i < 25; ++i)
    {
        uart_write(SERIAL_OUTPUT_PORT, '\n');
    }
}

void uart_scroll(const SCROLL_DIRECTION_E direction,
                 const uint32_t lines_count)
{
    uint32_t i;
    if(direction == SCROLL_DOWN)
    {
        /* Just print lines_count line feed. */
        for(i = 0; i < lines_count; ++i)
        {
            uart_write(SERIAL_OUTPUT_PORT, '\n');
        }
    }    
}

void uart_console_write_keyboard(const char* str, const size_t len)
{
    size_t i;
    for(i = 0; i < len; ++i)
    {
        uart_write(SERIAL_OUTPUT_PORT, str[i]);
    }
}

uint8_t uart_read(const uint32_t port)
{
    /* Wait for data to be received */
    while (uart_received(port) == 0);

    /* Read available data on port */
    uint8_t val = cpu_inb(SERIAL_DATA_PORT(port));

    return val;
}

void uart_put_string(const char* string)
{
    size_t i;
    for(i = 0; i < strlen(string); ++i)
    {
        uart_write(SERIAL_OUTPUT_PORT, string[i]);
    }
}

void uart_put_char(const char character)
{
    uart_write(SERIAL_OUTPUT_PORT, character);
}

uint8_t uart_received(const uint32_t port)
{
    /* Read on LINE status port */
    return cpu_inb(SERIAL_LINE_STATUS_PORT(port)) & 0x01;
}
