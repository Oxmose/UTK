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

/*******************************************************************************
 * DEFINITIONS
 ******************************************************************************/

/** @brief Minimal customizable accepted exception line. */
#define MIN_EXCEPTION_LINE 0
/** @brief Maximal customizable accepted exception line. */
#define MAX_EXCEPTION_LINE 31

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

/** @brief Scheduler interrupt line. */
#define SCHEDULER_SW_INT_LINE 0x40