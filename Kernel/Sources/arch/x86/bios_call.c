/***************************************************************************//**
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

#include <lib/stdint.h>    /* Generic int types */
#include <memory/paging.h> /* Memory management */
#include <core/panic.h>    /* Kernel panic */

/* UTK configuration file */
#include <config.h>

/* Header file */
#include <bios_call.h>

/*******************************************************************************
 * GLOBAL VARIABLES
 ******************************************************************************/

/** @brief BIOS call memory region */
extern uint8_t bios_call_memory;

/*******************************************************************************
 * FUNCTIONS
 ******************************************************************************/

/* Assemly function */
extern void __bios_call(uint8_t intnum, bios_int_regs_t* regs);


OS_RETURN_E bios_call(uint32_t intnum, bios_int_regs_t* regs)
{
	OS_RETURN_E err;

	/* Map the RM core */
	err = kernel_mmap_hw((void*)&bios_call_memory, (void*)&bios_call_memory, 
	                     0x1000, 0, 1);
	if(err != OS_NO_ERR && err != OS_ERR_MAPPING_ALREADY_EXISTS)
	{
		return err;
	}

	__bios_call(intnum, regs);

	/* Unmap RM core */
	err = kernel_munmap((void*)&bios_call_memory, 0x1000);
	if(err != OS_NO_ERR)
	{
		return err;
	}

	return OS_NO_ERR;
}