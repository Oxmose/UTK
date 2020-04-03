/*******************************************************************************
 * @file panic.h
 *
 * @see panic.c
 *
 * @author Alexy Torres Aurora Dugo
 *
 * @date 16/12/2017
 *
 * @version 1.0
 *
 * @brief Panic feature of the kernel.
 *
 * @details Kernel panic functions. Displays the CPU registers, the faulty
 * instruction, the interrupt ID and cause.
 *
 * @copyright Alexy Torres Aurora Dugo
 ******************************************************************************/

#ifndef __CORE_PANIC_H_
#define __CORE_PANIC_H_

#include <cpu_structs.h> /* CPU structures */
#include <lib/stdint.h>  /* Generic int types */

/*******************************************************************************
 * CONSTANTS
 ******************************************************************************/

/**
 * @brief Defines the NMI Panic code.
 */
#define PANIC_NMI_CODE 0xFFFFFFFF

/*******************************************************************************
 * STRUCTURES
 ******************************************************************************/

/*******************************************************************************
 * FUNCTIONS
 ******************************************************************************/

/**
 * @brief Displays the kernel panic screen.
 *
 * @details Displays the kernel panic screen. This sreen dumps the CPU registers
 * and the stack state before the panic occured (panic is usually called by
 * interrupts).
 *
 * @param[in, out] cpu_state The cpu registers structure.
 * @param[in] int_id The interrupt number, -1 if panic is called by an
 * regular code.
 * @param[in,out] stack_state The stack state before the interrupt that contain cs,
 * eip, error code and the eflags register value.
 *
 * @warning Panic should never be called, it must only be used as an interrupt
 * handler.
 */
void panic(cpu_state_t* cpu_state, uint32_t int_id, stack_state_t* stack_state);

/**
 * @brief Calls the panic interrupt line.
 *
 * @details Causes a kernel panic by raising the kernel panic interrupt line.
 *
 * @param[in] error_code The error code to display on the kernel panic's screen.
 */
void kernel_panic(const uint32_t error_code);

#endif /* #ifndef __CORE_PANIC_H_ */