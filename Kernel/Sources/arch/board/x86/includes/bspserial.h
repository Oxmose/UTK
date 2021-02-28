/*******************************************************************************
 * @file bspserial.h
 * 
 * @see bspserial.c
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

#ifndef __BSPSERIAL_H_
#define __BSPSERIAL_H_

#include <stdint.h>  /* Generic int types */
#include <stddef.h>  /* Standard definitions */
#include <graphic.h> /* Graphic definitions */
#include <uart.h>    /* UART main header */

/*******************************************************************************
 * DEFINITIONS
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
 * STRUCTURES
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

#endif /* #ifndef __BSPSERIAL_H_ */
