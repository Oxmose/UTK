/*******************************************************************************
 * @file interrupt_settings.h
 *
 * @author Alexy Torres Aurora Dugo
 *
 * @date 02/04/2020
 *
 * @version 2.0
 *
 * @brief i386 interrupt settings.
 * 
 * @details i386 interrupt settings. Stores the interrupt settings such as the
 * interrupt lines.
 * 
 * @copyright Alexy Torres Aurora Dugo
 ******************************************************************************/

#ifndef __X86_INTERRUPT_SETTINGS_
#define __X86_INTERRUPT_SETTINGS_

#include <cpu_structs.h> /* CPU structures */

/*******************************************************************************
 * DEFINITIONS
 ******************************************************************************/

/** @brief Minimal customizable accepted interrupt line. */
#define MIN_INTERRUPT_LINE     0x20
/** @brief Maximal customizable accepted interrupt line. */
#define MAX_INTERRUPT_LINE     (IDT_ENTRY_COUNT - 1)

/** @brief Defines the number of possible interrupt on the i386 processor. */
#define INT_ENTRY_COUNT IDT_ENTRY_COUNT

/** @brief Minimal customizable accepted exception line. */
#define MIN_EXCEPTION_LINE 0x0
/** @brief Maximal customizable accepted exception line. */
#define MAX_EXCEPTION_LINE 0x1F

/** @brief Divide by zero exception line. */
#define DIV_BY_ZERO_LINE 0

/** @brief Device not found exception. */
#define DEVICE_NOT_FOUND_LINE 7

/** @brief Page fault exception */
#define PAGE_FAULT_LINE 14

/** @brief Defines the panic interrupt line. */
#define PANIC_INT_LINE 0x2A

/** @brief Serial COM2-4 IRQ number. */
#define SERIAL_2_4_IRQ_LINE       0x03
/** @brief Serial COM1-3 IRQ number. */
#define SERIAL_1_3_IRQ_LINE       0x04

/** @brief Master PIC spurious IRQ number. */
#define PIC_SPURIOUS_IRQ_MASTER 0x07
/** @brief Slave PIC spurious IRQ number. */
#define PIC_SPURIOUS_IRQ_SLAVE  0x0F

/** @brief Offset of the first line of an IRQ interrupt from PIC. */
#define INT_PIC_IRQ_OFFSET     0x30
/** @brief Offset of the first line of an IRQ interrupt from IO-APIC. */
#define INT_IOAPIC_IRQ_OFFSET  0x40

/** @brief Scheduler interrupt line. */
#define SCHEDULER_SW_INT_LINE 0x40

/** @brief LAPIC spurious interrupt vector. */
#define LAPIC_SPURIOUS_INT_LINE MAX_INTERRUPT_LINE

/*******************************************************************************
 * STRUCTURES
 ******************************************************************************/

/* None */

/*******************************************************************************
 * FUNCTIONS
 ******************************************************************************/

/* None */

#endif /* #ifndef __X86_INTERRUPT_SETTINGS_ */