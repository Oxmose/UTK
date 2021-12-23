/*******************************************************************************
 * @file bios_call.c
 *
 * @see bios_call.h, bios_call_asm.S
 *
 * @author Alexy Torres Aurora Dugo
 *
 * @date 02/01/2018
 *
 * @version 1.0
 *
 * @brief BIOS call manager.
 *
 * @details BIOS call manager, allowing the CPU in protected mode to switch back
 * to real mode and issue an interrupt handled by the BIOS IVT.
 *
 * @copyright Alexy Torres Aurora Dugo
 ******************************************************************************/

#include <stdint.h>   /* Generic int types */
#include <memmgt.h>   /* Memory management */
#include <critical.h> /* Critical sections */

/* UTK configuration file */
#include <config.h>

/* Tests header file */
#include <test_bank.h>

/* Header file */
#include <bios_call.h>

/*******************************************************************************
 * CONSTANTS
 ******************************************************************************/

/* None */

/*******************************************************************************
 * STRUCTURES AND TYPES
 ******************************************************************************/

/* None */

/*******************************************************************************
 * GLOBAL VARIABLES
 ******************************************************************************/

/** @brief BIOS call memory region */
extern uint8_t _KERNEL_BIOS_MEMORY_BASE;

/** @brief BIOS call memory region size */
extern uint8_t _KERNEL_BIOS_MEMORY_SIZE;

/*******************************************************************************
 * STATIC FUNCTIONS DECLARATIONS
 ******************************************************************************/

/**
 * @brief Assembly bios call function.
 *
 * @details Assembly bios call function. Get the interrupt parameter in the
 * register buffer and returns the values in the same buffer.
 *
 * @param[in] intnum The interrupt line to call.
 * @param[int, out] regs The register buffer to get/set the call values and
 * returns.
 */
extern void __bios_call(uint8_t intnum, bios_int_regs_t* regs);

/*******************************************************************************
 * FUNCTIONS
 ******************************************************************************/

void bios_call(uint32_t intnum, bios_int_regs_t* regs)
{
	uint32_t    int_state;

	ENTER_CRITICAL(int_state);
	/* Map the RM core */
	memory_mmap_direct((void*)&_KERNEL_BIOS_MEMORY_BASE,
					   (void*)&_KERNEL_BIOS_MEMORY_BASE,
					   (size_t)&_KERNEL_BIOS_MEMORY_SIZE,
					   0,
					   1,
					   1,
					   1,
					   NULL);

	__bios_call(intnum, regs);

	/* Unmap RM core */
	memory_munmap((void*)&_KERNEL_BIOS_MEMORY_BASE,
	              (size_t)&_KERNEL_BIOS_MEMORY_SIZE,
				  NULL);

	EXIT_CRITICAL(int_state);
}