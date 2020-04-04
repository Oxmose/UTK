/*******************************************************************************
 * @file pic.h
 *
 * @see pic.c
 *
 * @author Alexy Torres Aurora Dugo
 *
 * @date 17/12/2017
 *
 * @version 1.0
 *
 * @brief PIC (programmable interrupt controler) driver.
 *
 * @details   PIC (programmable interrupt controler) driver. Allows to remmap
 * the PIC IRQ, set the IRQs mask and manage EoI for the X86 PIC.
 *
 * @copyright Alexy Torres Aurora Dugo
 ******************************************************************************/


#ifndef __X86_PIC_H_
#define __X86_PIC_H_

#include <lib/stdint.h>           /* Generic int types */
#include <lib/stddef.h>           /* Standard definitions */
#include <interrupt/interrupts.h> /* Interrupt management */
#include <interrupt_settings.h>   /* Interrupt settings */

/*******************************************************************************
 * CONSTANTS
 ******************************************************************************/

/** @brief Master PIC CPU command port. */
#define PIC_MASTER_COMM_PORT 0x20
/** @brief Master PIC CPU data port. */
#define PIC_MASTER_DATA_PORT 0x21
/** @brief Slave PIC CPU command port. */
#define PIC_SLAVE_COMM_PORT  0xa0
/** @brief Slave PIC CPU data port. */
#define PIC_SLAVE_DATA_PORT  0xa1

/** @brief PIC End of Interrupt command. */
#define PIC_EOI 0x20

/** @brief PIC ICW4 needed flag. */
#define PIC_ICW1_ICW4      0x01
/** @brief PIC single mode flag. */
#define PIC_ICW1_SINGLE    0x02
/** @brief PIC call address interval 4 flag. */
#define PIC_ICW1_INTERVAL4 0x04
/** @brief PIC trigger level flag. */
#define PIC_ICW1_LEVEL     0x08
/** @brief PIC initialization flag. */
#define PIC_ICW1_INIT      0x10

/** @brief PIC 8086/88 (MCS-80/85) mode flag. */
#define PIC_ICW4_8086	    0x01
/** @brief PIC auto (normal) EOI flag. */
#define PIC_ICW4_AUTO	    0x02
/** @brief PIC buffered mode/slave flag. */
#define PIC_ICW4_BUF_SLAVE	0x08
/** @brief PIC buffered mode/master flag. */
#define PIC_ICW4_BUF_MASTER	0x0C
/** @brief PIC special fully nested (not) flag. */
#define PIC_ICW4_SFNM	    0x10

/** @brief Read ISR command value */
#define PIC_READ_ISR 0x0B

/** @brief Master PIC Base interrupt line for the lowest IRQ. */
#define PIC0_BASE_INTERRUPT_LINE INT_PIC_IRQ_OFFSET
/** @brief Slave PIC Base interrupt line for the lowest IRQ. */
#define PIC1_BASE_INTERRUPT_LINE (INT_PIC_IRQ_OFFSET + 8)

/** @brief PIC's minimal IRQ number. */
#define PIC_MIN_IRQ_LINE 0
/** @brief PIC's maximal IRQ number. */
#define PIC_MAX_IRQ_LINE 15

/** @brief PIC's cascading IRQ number. */
#define PIC_CASCADING_IRQ 2

/** @brief The PIC spurious irq mask. */
#define PIC_SPURIOUS_IRQ_MASK 0x80

/** @brief Master PIC spurious IRQ number. */
#define PIC_SPURIOUS_IRQ_MASTER 0x07
/** @brief Slave PIC spurious IRQ number. */
#define PIC_SPURIOUS_IRQ_SLAVE  0x0F

/*******************************************************************************
 * STRUCTURES
 ******************************************************************************/

/** @brief PIC driver instance. */
extern interrupt_driver_t pic_driver;

/*******************************************************************************
 * FUNCTIONS
 ******************************************************************************/

/**
 * @brief Initializes the PIC.
 *
 * @details  Initializes the PIC by remapping the IRQ interrupts.
 * Disable all IRQ by reseting the IRQs mask.
 *
 * @return The success state or the error code.
 * - OS_NO_ERR is returned if no error is encountered.
 * - No other return value is possible.
 */
OS_RETURN_E pic_init(void);

/**
 * @brief Sets the IRQ mask for the desired IRQ number.
 *
 * @details Sets the IRQ mask for the IRQ number given as parameter.
 *
 * @param[in] irq_number The irq number to enable/disable.
 * @param[in] enabled Must be set to 1 to enable the IRQ or 0 to disable the
 * IRQ.
 *
 * @return The success state or the error code.
 * - OS_NO_ERR is returned if no error is encountered.
 * - OS_ERR_NO_SUCH_IRQ_LINE is returned if the IRQ number is not supported.
 */
OS_RETURN_E pic_set_irq_mask(const uint32_t irq_number, const uint32_t enabled);

/**
 * @brief Acknowleges an IRQ.
 *
 * @details Acknowlege an IRQ by setting the End Of Interrupt bit for this IRQ.
 *
 * @param[in] irq_number The irq number to Acknowlege.
 *
 * @return The success state or the error code.
 * - OS_NO_ERR if no error is encountered.
 * - OS_ERR_NO_SUCH_IRQ_LINE is returned if the IRQ number is not supported.
 */
OS_RETURN_E pic_set_irq_eoi(const uint32_t irq_number);

/**
 * @brief Checks if the serviced interrupt is a spurious
 * interrupt. The function also handles the spurious interrupt.
 *
 * @details Checks if the serviced interrupt is a spurious
 * interrupt. The function also handles the spurious interrupt.
 *
 * @param[in] int_number The interrupt number of the interrupt to test.
 *
 * @return The function will return the interrupt type.
 * - INTERRUPT_TYPE_SPURIOUS if the current interrupt is a spurious one.
 * - INTERRUPT_TYPE_REGULAR if the current interrupt is a regular one.
 */
INTERRUPT_TYPE_E pic_handle_spurious_irq(const uint32_t int_number);

/**
 * @brief Disables the PIC.
 *
 * @details Disables the PIC by masking all interrupts.
 *
 * @return The success state or the error code.
 * - OS_NO_ERR if no error is encountered.
 * - No other return value is possible.
 */
OS_RETURN_E pic_disable(void);

/**
 * @brief Returns the interrupt line attached to an IRQ.
 *
 * @details Returns the interrupt line attached to an IRQ. -1 is returned
 * if the IRQ number is not supported by the driver.
 *
 * @return The interrupt line attached to an IRQ. -1 is returned if the IRQ
 * number is not supported by the driver.
 */
int32_t pic_get_irq_int_line(const uint32_t irq_number);

#endif /* #ifndef __X86_PIC_H_ */