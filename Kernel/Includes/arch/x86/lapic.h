/*******************************************************************************
 * @file lapic.h
 *
 * @see lapic.c
 *
 * @author Alexy Torres Aurora Dugo
 *
 * @date 25/12/2017
 *
 * @version 1.0
 *
 * @brief Local APIC (Advanced programmable interrupt controler) driver.
 *
 * @details Local APIC (Advanced programmable interrupt controler) driver.
 * Manages x86 IRQs from the IO-APIC. The driver also allow the use of the LAPIC
 * timer as a timer source. IPI (inter processor interrupt) are also possible
 * thanks to the driver.
 *
 * @warning This driver uses the PIT (Programmable interval timer) to initialize
 * the LAPIC timer. the PIC must be present and initialized to use this driver.
 *
 * @copyright Alexy Torres Aurora Dugo
 ******************************************************************************/


#ifndef __X86_LAPIC_H_
#define __X86_LAPIC_H_

#include <lib/stdint.h>           /* Generic int types */
#include <lib/stddef.h>           /* Standard definitions */
#include <time/time_management.h> /* Timer factory */

/*******************************************************************************
 * CONSTANTS
 ******************************************************************************/

/** @brief LAPIC ID register's offset. */
#define LAPIC_ID                        0x0020
/** @brief LAPIC version register's offset. */
#define LAPIC_VER                       0x0030
/** @brief LAPIC trask priority register's offset. */
#define LAPIC_TPR                       0x0080
/** @brief LAPIC arbitration policy register's offset. */
#define LAPIC_APR                       0x0090
/** @brief LAPIC processor priority register's offset. */
#define LAPIC_PPR                       0x00A0
/** @brief LAPIC EOI register's offset. */
#define LAPIC_EOI                       0x00B0
/** @brief LAPIC remote read register's offset. */
#define LAPIC_RRD                       0x00C0
/** @brief LAPIC logical destination register's offset. */
#define LAPIC_LDR                       0x00D0
/** @brief LAPIC destination format register's offset. */
#define LAPIC_DFR                       0x00E0
/** @brief LAPIC Spurious interrupt vector register's offset. */
#define LAPIC_SVR                       0x00F0
/** @brief LAPIC in service register's offset. */
#define LAPIC_ISR                       0x0100
/** @brief LAPIC trigger mode register's offset. */
#define LAPIC_TMR                       0x0180
/** @brief LAPIC interrupt request register's offset. */
#define LAPIC_IRR                       0x0200
/** @brief LAPIC error status register's offset. */
#define LAPIC_ESR                       0x0280
/** @brief LAPIC interrupt command (low) register's offset. */
#define LAPIC_ICRLO                     0x0300
/** @brief LAPIC interrupt command (high) register's offset. */
#define LAPIC_ICRHI                     0x0310
/** @brief LAPIC local vector table timer register's offset. */
#define LAPIC_TIMER                     0x0320
/** @brief LAPIC local vector table thermal sensor register's offset. */
#define LAPIC_THERMAL                   0x0330
/** @brief LAPIC local vector table PMC register's offset. */
#define LAPIC_PERF                      0x0340
/** @brief LAPIC local vector table lint0 register's offset. */
#define LAPIC_LINT0                     0x0350
/** @brief LAPIC local vector table lint1 register's offset. */
#define LAPIC_LINT1                     0x0360
/** @brief LAPIC local vector table error register's offset. */
#define LAPIC_ERROR                     0x0370
/** @brief LAPIC timer initial count register's offset. */
#define LAPIC_TICR                      0x0380
/** @brief LAPIC timer current count register's offset. */
#define LAPIC_TCCR                      0x0390
/** @brief LAPIC timer devide configuration register's offset. */
#define LAPIC_TDCR                      0x03E0

/* Delivery Mode */
/** @brief LAPIC delivery mode fixed. */
#define ICR_FIXED                       0x00000000
/** @brief LAPIC delivery mode lowest priority. */
#define ICR_LOWEST                      0x00000100
/** @brief LAPIC delivery mode SMI. */
#define ICR_SMI                         0x00000200
/** @brief LAPIC delivery mode NMI. */
#define ICR_NMI                         0x00000400
/** @brief LAPIC delivery mode init IPI. */
#define ICR_INIT                        0x00000500
/** @brief LAPIC delivery mode startup IPI. */
#define ICR_STARTUP                     0x00000600
/** @brief LAPIC delivery mode external. */
#define ICR_EXTERNAL                    0x00000700

/** @brief LAPIC destination mode physical. */
#define ICR_PHYSICAL                    0x00000000
/** @brief LAPIC destination mode logical. */
#define ICR_LOGICAL                     0x00000800

/** @brief LAPIC Delivery status idle. */
#define ICR_IDLE                        0x00000000
/** @brief LAPIC Delivery status pending. */
#define ICR_SEND_PENDING                0x00001000

/** @brief LAPIC Level deassert enable flag. */
#define ICR_DEASSERT                    0x00000000
/** @brief LAPIC Level deassert disable flag. */
#define ICR_ASSERT                      0x00004000

/** @brief LAPIC trigger mode edge. */
#define ICR_EDGE                        0x00000000
/** @brief LAPIC trigger mode level. */
#define ICR_LEVEL                       0x00008000

/** @brief LAPIC destination shorthand none. */
#define ICR_NO_SHORTHAND                0x00000000
/** @brief LAPIC destination shorthand self only. */
#define ICR_SELF                        0x00040000
/** @brief LAPIC destination shorthand all and self. */
#define ICR_ALL_INCLUDING_SELF          0x00080000
/** @brief LAPIC destination shorthand all but self. */
#define ICR_ALL_EXCLUDING_SELF          0x000C0000

/** @brief LAPIC destination flag shift. */
#define ICR_DESTINATION_SHIFT           24

/** @brief LAPIC Timer mode flag: periodic. */
#define LAPIC_TIMER_MODE_PERIODIC       0x20000
/** @brief LAPIC Timer divider value. */
#define LAPIC_DIVIDER_16                0x3
/** @brief LAPIC Timer initial frequency. */
#define LAPIC_INIT_FREQ                 100
/** @brief LAPIC Timer vector interrupt masked. */
#define LAPIC_LVT_INT_MASKED            0x10000

/*******************************************************************************
 * STRUCTURES
 ******************************************************************************/

/** @brief LAPIC Timer driver instance. */
extern kernel_timer_t lapic_timer_driver;

/*******************************************************************************
 * FUNCTIONS
 ******************************************************************************/

/**
 * @brief Initializes the CPU Local APIC.
 *
 * @details Initializes the CPU Local APIC. The function initializes the LAPIC
 * interrupts (enables them), set the LAPIC destination mode and sets the
 * spurious vector.
 *
 * @return The success state or the error code.
 * - OS_NO_ERR is returned if no error is encountered.
 * - OS_ERR_NOT_SUPPORTED is returned if the LAPIC is not supported by the
 * current system.
 */
OS_RETURN_E lapic_init(void);

/**
 * @brief Initializes the CPU Local APIC Timer.
 *
 * @details Initializes the CPU Local APIC Timer. The function initializes the
 * LAPIC Timer and its interrupt. The Timer is then set to its initial frequency
 * by the driver.
 *
 * @warning This function is not to be used by Application Processors, please
 * use lapic_ap_timer_init for this purpose.
 *
 * @return The success state or the error code.
 * - OS_NO_ERR is returned if no error is encountered.
 * - OS_ERR_NOT_SUPPORTED is returned if the LAPIC is not supported by the
 * current system.
 */
OS_RETURN_E lapic_timer_init(void);

/**
 * @brief Initializes the CPU Local APIC Timer for application processor.
 *
 * @details Initializes the CPU Local APIC Timer. The function initializes the
 * LAPIC Timer and its interrupt. The Timer is then set to its initial frequency
 * by the driver. This function is to be used for Application Processors only.
 *
 * @warning This function is to be used by Application Processors only.
 *
 * @return The success state or the error code.
 * - OS_NO_ERR is returned if no error is encountered.
 * - OS_ERR_NOT_SUPPORTED is returned if the LAPIC is not supported by the
 * current system.
 */
OS_RETURN_E lapic_ap_timer_init(void);

/**
 * @brief Returns the current CPU Local APIC ID.
 *
 * @details Returns the current CPU Local APIC ID. The current CPU is the CPU on
 * which the function is called.
 *
 * @returns The current CPU Local APIC ID. -1 is returned on error.
 */
int32_t lapic_get_id(void);

/**
 * @brief Send an INIT IPI to the corresponding LAPIC.
 *
 * @details Sends an INIT IPI to the CPU deisgned by the Local APIC ID given as
 * parameter.The ID is checked before sending the IPI.
 *
 * @param[in] lapic_id The Local APIC ID of the CPU to send the IPI to.
 *
 * @return The success state or the error code.
 * - OS_NO_ERR is returned if no error is encountered.
 * - OS_ERR_NOT_SUPPORTED is returned if the LAPIC is not supported by the
 * current system.
 * - OS_ERR_NO_SUCH_LAPIC_ID is returned if the given LAPIC ID does not exist.
 */
OS_RETURN_E lapic_send_ipi_init(const uint32_t lapic_id);

/**
 * @brief Sends a STARTUP IPI to the corresponding LAPIC.
 *
 * @details Sends an STARTUP IPI to the CPU deisgned by the Local APIC ID given
 * as parameter. The ID is checked before sending the IPI.
 *
 * @param[in] lapic_id The Local APIC ID of the CPU to send the IPI to.
 * @param[in] vector The startup IPI vector.
 *
 * @return The success state or the error code.
 * - OS_NO_ERR is returned if no error is encountered.
 * - OS_ERR_NOT_SUPPORTED is returned if the LAPIC is not supported by the
 * current system.
 * - OS_ERR_NO_SUCH_LAPIC_ID is returned if the given LAPIC ID does not exist.
 */
OS_RETURN_E lapic_send_ipi_startup(const uint32_t lapic_id,
                                   const uint32_t vector);

/**
 * @brief Sends an IPI to the corresponding LAPIC.
 *
 * @details Send an IPI to the CPU deisgned by the Local APIC ID given as
 * parameter.The ID is checked before sending the IPI.
 *
 * @param[in] lapic_id The Local APIC ID of the CPU to send the IPI to.
 * @param[in] vector The IPI vector.
 *
 * @return The success state or the error code.
 * - OS_NO_ERR is returned if no error is encountered.
 * - OS_ERR_NOT_SUPPORTED is returned if the LAPIC is not supported by the
 * current system.
 * - OS_ERR_NO_SUCH_LAPIC_ID is returned if the given LAPIC ID does not exist.
 */
OS_RETURN_E lapic_send_ipi(const uint32_t lapic_id,
                           const uint32_t vector);

/**
 * @brief Sets END OF INTERRUPT for the current CPU Local APIC.
 *
 * @details Sets END OF INTERRUPT for the current CPU Local APIC.
 *
 * @param[in] interrupt_line The intrrupt line for which the EOI should be set.
 *
 * @return The success state or the error code.
 * - OS_NO_ERR is returned if no error is encountered.
 * - OS_ERR_NOT_SUPPORTED is returned if the LAPIC is not supported by the
 * current system.
 * - OS_ERR_NO_SUCH_IRQ_LINE is returned if the interurpt number is not
 * supported.
 */
OS_RETURN_E lapic_set_int_eoi(const uint32_t interrupt_line);

/**
 * @brief Enables LAPIC Timer ticks.
 *
 * @details Enables LAPIC Timer ticks by clearing the LAPIC Timer's IRQ mask.
 *
 * @return The success state or the error code.
 * - OS_NO_ERR is returned if no error is encountered.
 * - OS_ERR_NO_SUCH_IRQ_LINE is returned if the IRQ number of the LAPIC Timer is
 * not supported.
 */
OS_RETURN_E lapic_timer_enable(void);

/**
 * @brief Disables LAPIC Timer ticks.
 *
 * @details Disables LAPIC Timer ticks by setting the LAPIC Timer's IRQ mask.
 *
 * @return The success state or the error code.
 * - OS_NO_ERR is returned if no error is encountered.
 * - OS_ERR_NO_SUCH_IRQ_LINE is returned if the IRQ number of the LAPIC Timer is
 * not supported.
 */
OS_RETURN_E lapic_timer_disable(void);

/**
 * @brief Sets the LAPIC Timer's tick frequency.
 *
 * @details Sets the LAPIC Timer's tick frequency. The value must be between
 * 20Hz and 8000Hz.
 *
 * @warning The value must be between 20Hz and 8000Hz
 *
 * @param[in] freq The new frequency to be set to the LAPIC Timer.
 *
 * @return The success state or the error code.
 * - OS_NO_ERR is returned if no error is encountered.
 * - OS_ERR_OUT_OF_BOUND is returned if the frequency is out of bounds.
 * - OS_ERR_NO_SUCH_IRQ_LINE is returned if the IRQ number of the LAPIC Timer is
 * not supported.
 */
OS_RETURN_E lapic_timer_set_frequency(const uint32_t freq);

/**
 * @brief Returns the LAPIC Timer tick frequency in Hz.
 *
 * @details Returns the LAPIC Timer tick frequency in Hz.
 *
 * @return The LAPIC Timer tick frequency in Hz.
 */
uint32_t lapic_timer_get_frequency(void);

/**
 * @brief Sets the LAPIC Timer tick handler.
 *
 * @details Sets the LAPIC Timer tick handler. This function will be called at
 * each LAPIC Timer tick received.
 *
 * @param[in] handler The handler of the LAPIC Timer interrupt.
 *
 * @return The success state or the error code.
 * - OS_NO_ERR is returned if no error is encountered.
 * - OS_ERR_NULL_POINTER is returned if the handler is NULL.
  * - OR_ERR_UNAUTHORIZED_INTERRUPT_LINE is returned if the LAPIC Timer
  * interrupt line is not allowed.
 * - OS_ERR_NULL_POINTER is returned if the pointer to the handler is NULL.
 * - OS_ERR_INTERRUPT_ALREADY_REGISTERED is returned if a handler is already
 * registered for the LAPIC Timer.
 */
OS_RETURN_E lapic_timer_set_handler(void(*handler)(
                                    cpu_state_t*,
                                    uintptr_t,
                                    stack_state_t*
                                    ));

/**
 * @brief Removes the LAPIC Timer tick handler.
 *
 * @details Removes the LAPIC Timer tick handler.
 *
 * @return The success state or the error code.
 * - OS_NO_ERR is returned if no error is encountered.
 * - OR_ERR_UNAUTHORIZED_INTERRUPT_LINE is returned if the LAPIC Timer interrupt
 * line is not allowed.
 * - OS_ERR_INTERRUPT_NOT_REGISTERED is returned if the LAPIC Timer line has no
 * handler attached.
 */
OS_RETURN_E lapic_timer_remove_handler(void);

/**
 * @brief Returns the LAPIC Timer IRQ number.
 *
 * @details Returns the LAPIC Timer IRQ number.
 *
 * @return The LAPIC Timer IRQ number.
 */
uint32_t lapic_timer_get_irq(void);

#endif /* #ifndef __X86_LAPIC_H_ */