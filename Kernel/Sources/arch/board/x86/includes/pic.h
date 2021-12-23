/*******************************************************************************
 * @file pic.h
 *
 * @see pic.c
 *
 * @author Alexy Torres Aurora Dugo
 *
 * @date 08/03/2021
 *
 * @version 1.0
 *
 * @brief PIC (programmable interrupt controler) driver.
 *
 * @details PIC (programmable interrupt controler) driver. Allows to remmap
 * the PIC IRQ, set the IRQs mask and manage EoI for the X86 PIC.
 *
 * @copyright Alexy Torres Aurora Dugo
 ******************************************************************************/

#ifndef __X86_PIC_H_
#define __X86_PIC_H_

/*******************************************************************************
 * INCLUDES
 ******************************************************************************/

#include <stdint.h>       /* Generic int types */
#include <interrupts.h>   /* Interrupt management */
#include <kernel_error.h> /* Kernel error codes */

/*******************************************************************************
 * CONSTANTS
 ******************************************************************************/

/* None */

/*******************************************************************************
 * STRUCTURES AND TYPES
 ******************************************************************************/

/* None */

/*******************************************************************************
 * MACROS
 ******************************************************************************/

/* None */

/*******************************************************************************
 * GLOBAL VARIABLES
 ******************************************************************************/

/* Imported global variables */
/* None */

/* Exported global variables */
/* None */

/* Static global variables */
/* None */

/*******************************************************************************
 * FUNCTIONS
 ******************************************************************************/

/**
 * @brief Initializes the PIC.
 *
 * @details  Initializes the PIC by remapping the IRQ interrupts.
 * Disable all IRQ by reseting the IRQs mask.
 */
void pic_init(void);

/**
 * @brief Sets the IRQ mask for the desired IRQ number.
 *
 * @details Sets the IRQ mask for the IRQ number given as parameter.
 *
 * @param[in] irq_number The irq number to enable/disable.
 * @param[in] enabled Must be set to TRUE to enable the IRQ or FALSE to disable
 * the IRQ.
 */
void pic_set_irq_mask(const uint32_t irq_number, const bool_t enabled);

/**
 * @brief Acknowleges an IRQ.
 *
 * @details Acknowlege an IRQ by setting the End Of Interrupt bit for this IRQ.
 *
 * @param[in] irq_number The irq number to Acknowlege.
 */
void pic_set_irq_eoi(const uint32_t irq_number);

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
 */
void pic_disable(void);

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

/**
 * @brief Returns the PIC interrupt driver.
 *
 * @details Returns a constant handle to the PIC interrupt driver.
 *
 * @return A constant handle to the PIC interrupt driver is returned.
 */
const interrupt_driver_t* pic_get_driver(void);

#endif /* #ifndef __X86_PIC_H_ */

/* EOF */