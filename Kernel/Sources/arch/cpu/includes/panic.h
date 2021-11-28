/*******************************************************************************
 * @file panic.h
 *
 * @see panic.c
 *
 * @author Alexy Torres Aurora Dugo
 *
 * @date 28/02/2021
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

#ifndef __CPU_PANIC_H_
#define __CPU_PANIC_H_

#include <cpu_settings.h> /* CPU structures */
#include <stdint.h>       /* Generic int types */
#include <stddef.h>       /* Pointer types */

/*******************************************************************************
 * CONSTANTS
 ******************************************************************************/

/* None */

/*******************************************************************************
 * STRUCTURES
 ******************************************************************************/

/* None */

/*******************************************************************************
 * FUNCTIONS
 ******************************************************************************/

/** 
 * @brief Raises a kernel panic with error code and collect other data.
 *
 * @param[in] ERROR The error code forthe panic.
 */
#define KERNEL_PANIC(ERROR) { 				  \
    kernel_panic(ERROR, __FILE__, __LINE__);  \
}

/**
 * @brief Displays the kernel panic screen.
 *
 * @details Displays the kernel panic screen. This sreen dumps the CPU registers
 * and the stack state before the panic occured.
 *
 * @param[in, out] cpu_state The cpu registers structure.
 * @param[in] int_id The interrupt number.
 * @param[in,out] stack_state The stack state before the interrupt.
 *
 * @warning Panic should never be called, it must only be used as an interrupt
 * handler.
 */
void panic(cpu_state_t* cpu_state, 
           uintptr_t int_id, 
           stack_state_t* stack_state);

/**
 * @brief Calls the panic interrupt line.
 *
 * @details Causes a kernel panic by raising the kernel panic interrupt line.
 *
 * @param[in] error_code The error code to display on the kernel panic's screen.
 * @param[in] file The name of the source file where the panic was called.
 * @param[in] line The line at which the panic was called.
 */
void kernel_panic(const uint32_t error_code, 
                  const char* file, 
                  const uint32_t line);

#endif /* #ifndef __CPU_PANIC_H_ */