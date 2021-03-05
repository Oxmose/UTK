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
#include <paging.h>   /* Memory management */
#include <panic.h>    /* Kernel panic */
#include <critical.h> /* Critical sections */

/* UTK configuration file */
#include <config.h>

/* Header file */
#include <bios_call.h>

/*******************************************************************************
 * GLOBAL VARIABLES
 ******************************************************************************/

/** @brief BIOS call memory region */
extern uint8_t _KERNEL_BIOS_MEMORY_BASE;

/** @brief BIOS call memory region size */
extern uint8_t _KERNEL_BIOS_MEMORY_SIZE;

/*******************************************************************************
 * FUNCTIONS
 ******************************************************************************/

/* Assemly function */
extern void __bios_call(uint8_t intnum, bios_int_regs_t* regs);


OS_RETURN_E bios_call(uint32_t intnum, bios_int_regs_t* regs)
{
	OS_RETURN_E err;
	uint32_t    int_state;

	/* Map the RM core */
	err = paging_kmmap_hw((void*)&_KERNEL_BIOS_MEMORY_BASE, 
	                      (void*)&_KERNEL_BIOS_MEMORY_BASE, 
	                      (size_t)&_KERNEL_BIOS_MEMORY_SIZE, 
						  0, 
						  1);
	if(err != OS_NO_ERR && err != OS_ERR_MAPPING_ALREADY_EXISTS)
	{
		return err;
	}

    ENTER_CRITICAL(int_state);

	__bios_call(intnum, regs);

    EXIT_CRITICAL(int_state);

	/* Unmap RM core */
	err = paging_kmunmap((void*)&_KERNEL_BIOS_MEMORY_BASE, 
	                    (size_t)&_KERNEL_BIOS_MEMORY_SIZE);
	if(err != OS_NO_ERR)
	{
		return err;
	}

	return OS_NO_ERR;
}