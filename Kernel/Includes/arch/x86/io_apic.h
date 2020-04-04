/*******************************************************************************
 * @file io_apic.h
 *
 * @see io_apic.c
 *
 * @author Alexy Torres Aurora Dugo
 *
 * @date 25/12/2017
 *
 * @version 1.0
 *
 * @brief IO-APIC (IO advanced programmable interrupt controler) driver.
 *
 * @details IO-APIC (IO advanced programmable interrupt controler) driver.
 * Allows to remmap the IO-APIC IRQ, set the IRQs mask and manage EoI for the
 * X86 IO-APIC.
 *
 * @warning This driver also use the LAPIC driver to function correctly.
 *
 * @copyright Alexy Torres Aurora Dugo
 ******************************************************************************/

#ifndef __X86_IO_APIC_H_
#define __X86_IO_APIC_H_

#include <lib/stdint.h>           /* Generic int types */
#include <lib/stddef.h>           /* Standard definitions */
#include <interrupt/interrupts.h> /*Interrupt management */

/*******************************************************************************
 * CONSTANTS
 ******************************************************************************/

/** @brief IO-APIC's minimal IRQ number. */
#define IO_APIC_MIN_IRQ_LINE 0
/** @brief IO-APIC's maximal IRQ number. */
#define IO_APIC_MAX_IRQ_LINE 23

/** @brief IO-APIC register selection. */
#define IOREGSEL 0x00
/** @brief IO-APIC Data write register. */
#define IOWIN    0x10

/** @brief IO-APIC ID register. */
#define IOAPICID  0x00
/** @brief IO-APIC version register. */
#define IOAPICVER 0x01
/** @brief IO-APIC arbitration id register. */
#define IOAPICARB 0x02
/** @brief IO-APIC redirection register. */
#define IOREDTBL  0x10

/*******************************************************************************
 * STRUCTURES
 ******************************************************************************/

/** @brief IO-APIC driver instance. */
extern interrupt_driver_t io_apic_driver;

/*******************************************************************************
 * FUNCTIONS
 ******************************************************************************/

/**
 * @brief Initializes the IO-APIC.
 *
 * @details Initializes the IO-APIC by remapping the IRQ interrupts.
 * Disables all IRQ by reseting the IRQs mask.
 *
 * @return The success state or the error code.
 * - OS_NO_ERR is returned if no error is encountered.
 * - OS_ERR_NOT_SUPPORTED if not IO-APIC had been found in the system or the
 * user disabled IO-APIC.
 * - No other return value is possible.
 */
OS_RETURN_E io_apic_init(void);

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
OS_RETURN_E io_apic_set_irq_mask(const uint32_t irq_number,
                                 const uint32_t enabled);

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
OS_RETURN_E io_apic_set_irq_eoi(const uint32_t irq_number);

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
INTERRUPT_TYPE_E io_apic_handle_spurious_irq(const uint32_t int_number);

/**
 * @brief Returns the interrupt line attached to an IRQ.
 *
 * @details Returns the interrupt line attached to an IRQ. -1 is returned
 * if the IRQ number is not supported by the driver.
 *
 * @return The interrupt line attached to an IRQ. -1 is returned if the IRQ
 * number is not supported by the driver.
 */
int32_t io_apic_get_irq_int_line(const uint32_t irq_number);

/** 
 * @brief Returns the IO-APIC availability.
 * 
 * @details Returns is the IO APIC is available (1) or not (0).
 * 
 * @return Returns is the IO APIC is available (1) or not (0).
 */
uint8_t io_apic_capable(void);

#endif /* #ifndef __X86_IO_APIC_H_ */