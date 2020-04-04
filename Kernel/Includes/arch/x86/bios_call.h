/*******************************************************************************
 * @file bios_call.h
 * 
 * @see bios_call.c, bios_call_asm.S
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

#ifndef __X86_BIOS_CALL_H_
#define __X86_BIOS_CALL_H_

#include <lib/stdint.h> /* Generic int types */
#include <lib/stddef.h> /* Standard definitions */

/*******************************************************************************
 * CONSTANTS
 ******************************************************************************/

/*******************************************************************************
 * STRUCTURES
 ******************************************************************************/

/** @brief BIOS call CPU abstraction. Used to store the CPU registers value. */
struct bios_int_regs
{
    /** @brief CPU di register */
    uint16_t di;
    /** @brief CPU si register */
    uint16_t si;
    /** @brief CPU bp register */
    uint16_t bp;
    /** @brief CPU sp register */
    uint16_t sp;
    /** @brief CPU bx register */
    uint16_t bx;
    /** @brief CPU dx register */
    uint16_t dx;
    /** @brief CPU cx register */
    uint16_t cx;
    /** @brief CPU ax register */
    uint16_t ax;
    /** @brief CPU gs register */
    uint16_t gs;
    /** @brief CPU fs register */
    uint16_t fs;
    /** @brief CPU es register */
    uint16_t es;
    /** @brief CPU ds register */
    uint16_t ds;
    /** @brief CPU eflags register */
    uint16_t eflags;
} __attribute__((__packed__));

/** 
 * @brief Defines bios_int_regs_t type as a shorcut for struct bios_int_regs.
 */
typedef struct bios_int_regs bios_int_regs_t;


/*******************************************************************************
 * FUNCTIONS
 ******************************************************************************/

/**
 * @brief Issue a bios interrupt.
 *
 * @details Switch the CPU to real mode and raise an interrupt. This interrupt
 * should be handled by the BIOS IVT.
 * 
 * @param[in] intnum The interrupt number to raise.
 * @param[in] regs The array containing the registers values for the call.
 * 
 * @return OS_NO_ERR is returned in case of success. Otherwise an error code is
 * returned.
 */
OS_RETURN_E bios_call(const uint32_t intnum, bios_int_regs_t* regs);

#endif /* #ifndef __X86_BIOS_CALL_H_  */