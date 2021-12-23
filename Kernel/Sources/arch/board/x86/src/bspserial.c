/*******************************************************************************
 * @file bspserial.c
 *
 * @see bspserial.h
 *
 * @author Alexy Torres Aurora Dugo
 *
 * @date 28/02/2021
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

#include <stdint.h>        /* Generic int types */
#include <string.h>        /* String manipulation */
#include <cpu.h>           /* CPU manipulation */
#include <graphic.h>       /* Graphic definitions */
#include <kernel_output.h> /* Kernel output methods */
#include <uart.h>          /* UART main header */
#include <graphic.h>       /* Graphic driver manager */

/* UTK configuration file */
#include <config.h>

/* Tests header file */
#include <test_bank.h>

/* Header */
#include <uart.h>

/*******************************************************************************
 * CONSTANTS
 ******************************************************************************/

/** @brief Serial COM1 base port ID. */
#define SERIAL_COM1_BASE 0x3F8
/** @brief Serial COM2 base port ID. */
#define SERIAL_COM2_BASE 0x2F8
/** @brief Serial COM3 base port ID. */
#define SERIAL_COM3_BASE 0x3E8
/** @brief Serial COM4 base port ID. */
#define SERIAL_COM4_BASE 0x2E8

/** @brief Redefinition of serial COM1 base port ID for ease of use. */
#define COM1 SERIAL_COM1_BASE
/** @brief Redefinition of serial COM2 base port ID for ease of use. */
#define COM2 SERIAL_COM2_BASE
/** @brief Redefinition of serial COM3 base port ID for ease of use. */
#define COM3 SERIAL_COM3_BASE
/** @brief Redefinition of serial COM4 base port ID for ease of use. */
#define COM4 SERIAL_COM4_BASE

/** @brief Defines the port that is used to print data. */
#define SERIAL_OUTPUT_PORT COM1

/** @brief Serial data length flag: 5 bits. */
#define SERIAL_DATA_LENGTH_5 0x00
/** @brief Serial data length flag: 6 bits. */
#define SERIAL_DATA_LENGTH_6 0x01
/** @brief Serial data length flag: 7 bits. */
#define SERIAL_DATA_LENGTH_7 0x02
/** @brief Serial data length flag: 8 bits. */
#define SERIAL_DATA_LENGTH_8 0x03

/** @brief Serial parity bit flag: 1 bit. */
#define SERIAL_STOP_BIT_1   0x00
/** @brief Serial parity bit flag: 2 bits. */
#define SERIAL_STOP_BIT_2   0x04

/** @brief Serial parity bit settings flag: none. */
#define SERIAL_PARITY_NONE  0x00
/** @brief Serial parity bit settings flag: odd. */
#define SERIAL_PARITY_ODD   0x01
/** @brief Serial parity bit settings flag: even. */
#define SERIAL_PARITY_EVEN  0x03
/** @brief Serial parity bit settings flag: mark. */
#define SERIAL_PARITY_MARK  0x05
/** @brief Serial parity bit settings flag: space. */
#define SERIAL_PARITY_SPACE 0x07

/** @brief Serial break control flag enabled. */
#define SERIAL_BREAK_CTRL_ENABLED  0x40
/** @brief Serial break control flag disabled. */
#define SERIAL_BREAK_CTRL_DISABLED 0x00

/** @brief Serial dlab flag enabled. */
#define SERIAL_DLAB_ENABLED  0x80
/** @brief Serial dlab flag disabled. */
#define SERIAL_DLAB_DISABLED 0x00

/** @brief Serial fifo enable flag. */
#define SERIAL_ENABLE_FIFO       0x01
/** @brief Serial fifo clear receive flag. */
#define SERIAL_CLEAR_RECV_FIFO   0x02
/** @brief Serial fifo clear send flag. */
#define SERIAL_CLEAR_SEND_FIFO   0x04
/** @brief Serial DMA accessed fifo flag. */
#define SERIAL_DMA_ACCESSED_FIFO 0x08

/** @brief Serial fifo depth flag: 14 bits. */
#define SERIAL_FIFO_DEPTH_14     0x00
/** @brief Serial fifo depth flag: 64 bits. */
#define SERIAL_FIFO_DEPTH_64     0x10

/**
 * @brief Computes the data port for the serial port which base port ID is
 * given as parameter.
 *
 * @param[in] port The base port ID of the serial port.
 */
#define SERIAL_DATA_PORT(port)          (port)
/**
 * @brief Computes the aux data port for the serial port which base port ID is
 * given as parameter.
 *
 * @param[in] port The base port ID of the serial port.
 */
#define SERIAL_DATA_PORT_2(port)        (port + 1)
/**
 * @brief Computes the fifo command port for the serial port which base port ID
 * is given as parameter.
 *
 * @param[in] port The base port ID of the serial port.
 */
#define SERIAL_FIFO_COMMAND_PORT(port)  (port + 2)
/**
 * @brief Computes the line command port for the serial port which base port ID
 * is given as parameter.
 *
 * @param[in] port The base port ID of the serial port.
 */
#define SERIAL_LINE_COMMAND_PORT(port)  (port + 3)
/**
 * @brief Computes the modem command port for the serial port which base port ID
 * is given as parameter.
 *
 * @param[in] port The base port ID of the serial port.
 */
#define SERIAL_MODEM_COMMAND_PORT(port) (port + 4)
/**
 * @brief Computes the line status port for the serial port which base port ID
 * is given as parameter.
 *
 * @param[in] port The base port ID of the serial port.
 */
#define SERIAL_LINE_STATUS_PORT(port)   (port + 5)

/*******************************************************************************
 * STRUCTURES AND TYPES
 ******************************************************************************/

/** @brief Serial baudrate enumation. Enumerates all the supported baudrates.
 * The value of the enumeration is the transmission rate divider.
 */
enum SERIAL_BAUDRATE
{
    /** @brief Baudrate 50Bd. */
    BAURDATE_50     = 2304,
    /** @brief Baudrate 75Bd. */
    BAUDRATE_75     = 1536,
    /** @brief Baudrate 150Bd. */
    BAUDRATE_150    = 768,
    /** @brief Baudrate 300Bd. */
    BAUDRATE_300    = 384,
    /** @brief Baudrate 600Bd. */
    BAUDRATE_600    = 192,
    /** @brief Baudrate 1200Bd. */
    BAUDRATE_1200   = 96,
    /** @brief Baudrate 1800Bd. */
    BAUDRATE_1800   = 64,
    /** @brief Baudrate 2400Bd. */
    BAUDRATE_2400   = 48,
    /** @brief Baudrate 4800Bd. */
    BAUDRATE_4800   = 24,
    /** @brief Baudrate 7200Bd. */
    BAUDRATE_7200   = 16,
    /** @brief Baudrate 9600Bd. */
    BAUDRATE_9600   = 12,
    /** @brief Baudrate 14400Bd. */
    BAUDRATE_14400  = 8,
    /** @brief Baudrate 19200Bd. */
    BAUDRATE_19200  = 6,
    /** @brief Baudrate 38400Bd. */
    BAUDRATE_38400  = 3,
    /** @brief Baudrate 57600Bd. */
    BAUDRATE_57600  = 2,
    /** @brief Baudrate 115200Bd. */
    BAUDRATE_115200 = 1,
};

/**
 * @brief Defines SERIAL_BAUDRATE_E type as a shorcut for enum SERIAL_BAUDRATE.
 */
typedef enum SERIAL_BAUDRATE SERIAL_BAUDRATE_E;

/*******************************************************************************
 * GLOBAL VARIABLES
 ******************************************************************************/

/**
 * @brief Serial text driver instance.
 */
static kernel_graphic_driver_t uart_text_driver =
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
 * STATIC FUNCTIONS DECLARATION
 ******************************************************************************/

/**
 * @brief Sets line parameters for the desired port.
 *
 * @details Sets line parameters for the desired port.
 *
 * @param[in] attr The settings for the port's line.
 * @param[in] com The port to set.
 */
static void set_line(const uint8_t attr, const uint8_t com);

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
static void set_buffer(const uint8_t attr, const uint8_t com);

/**
 * @brief Sets the port's baudrate.
 *
 * @details Sets the port's baudrate.
 *
 * @param[in] rate The desired baudrate for the port.
 * @param[in] com The port to set.
 */
static void set_baudrate(SERIAL_BAUDRATE_E rate, const uint8_t com);

/**
 * @brief Writes the data given as patameter on the desired port.
 *
 * @details The function will output the data given as parameter on the selected
 * port. This call is blocking until the data has been sent to the uart port
 * controler.
 *
 * @param[in] port The desired port to write the data to.
 * @param[in] data The byte to write to the uart port.
 */
void uart_write(const uint32_t port, const uint8_t data);

/*******************************************************************************
 * FUNCTIONS
 ******************************************************************************/

static void set_line(const uint8_t attr, const uint8_t com)
{
    cpu_outb(attr, SERIAL_LINE_COMMAND_PORT(com));

    KERNEL_DEBUG(SERIAL_DEBUG_ENABLED,
                 "[SERIAL] Set line attributes of port 0x%04x to %u",
                 com, attr);
}

static void set_buffer(const uint8_t attr, const uint8_t com)
{
    cpu_outb(attr, SERIAL_FIFO_COMMAND_PORT(com));

    KERNEL_DEBUG(SERIAL_DEBUG_ENABLED,
                 "[SERIAL] Set buffer attributes of port 0x%04x to %u",
                 com, attr);
}

static void set_baudrate(SERIAL_BAUDRATE_E rate, const uint8_t com)
{
    cpu_outb(SERIAL_DLAB_ENABLED, SERIAL_LINE_COMMAND_PORT(com));
    cpu_outb((rate >> 8) & 0x00FF, SERIAL_DATA_PORT(com));
    cpu_outb(rate & 0x00FF, SERIAL_DATA_PORT_2(com));

    KERNEL_DEBUG(SERIAL_DEBUG_ENABLED,
                 "[SERIAL] Set baud rate of port 0x%04x to %u",
                 com, rate);
}

void uart_write(const uint32_t port, const uint8_t data)
{
    uint32_t int_state;
    /* Wait for empty transmit */
    ENTER_CRITICAL(int_state);
    while((cpu_inb(SERIAL_LINE_STATUS_PORT(port)) & 0x20) == 0){}
    if(data == '\n')
    {
        uart_write(port, '\r');
        cpu_outb('\n', port);
    }
    else
    {
        cpu_outb(data, port);
    }
    EXIT_CRITICAL(int_state);
}

void uart_init(void)
{
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

        /* Init line */
        set_baudrate(BAUDRATE_9600, com);
        set_line(attr, com);
        set_buffer(0xC0 | SERIAL_ENABLE_FIFO | SERIAL_CLEAR_RECV_FIFO |
                   SERIAL_CLEAR_SEND_FIFO | SERIAL_FIFO_DEPTH_14, com);

        /* Enable interrupt */
        cpu_outb(0x0B, SERIAL_MODEM_COMMAND_PORT(com));
    }

    KERNEL_DEBUG(SERIAL_DEBUG_ENABLED, "[SERIAL] Serial initialization end");

    KERNEL_TEST_POINT(uart_test);
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
    uint32_t int_state;

    /* Wait for data to be received */
    ENTER_CRITICAL(int_state);
    while(uart_received(port) == 0){}


    /* Read available data on port */
    uint8_t val = cpu_inb(SERIAL_DATA_PORT(port));
    EXIT_CRITICAL(int_state);

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

bool_t uart_received(const uint32_t port)
{
    bool_t   ret;
    uint32_t int_state;

    /* Read on LINE status port */
    ENTER_CRITICAL(int_state);
    ret = cpu_inb(SERIAL_LINE_STATUS_PORT(port)) & 0x01;
    EXIT_CRITICAL(int_state);

    return ret;
}

const kernel_graphic_driver_t* uart_get_driver(void)
{
    return &uart_text_driver;
}