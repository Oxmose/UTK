/*******************************************************************************
 * @file cpu_structs.h
 *
 * @see cpu_structs.c
 * 
 * @author Alexy Torres Aurora Dugo
 *
 * @date 14/12/2017
 *
 * @version 1.0
 *
 * @brief i386 CPU structures.
 *
 * @details i386 CPU structures. IDT, GDT and CPU stacks are defined here
 *
 * @copyright Alexy Torres Aurora Dugo
 ******************************************************************************/

#ifndef __I386_CPU_STRUCTS_H_
#define __I386_CPU_STRUCTS_H_

#include <lib/stddef.h> /* Standard definitions */

/* UTK configuration file */
#include <config.h>

/*******************************************************************************
 * DEFINITIONS
 ******************************************************************************/

/** @brief Number of entries in the kernel's GDT. */
#define GDT_ENTRY_COUNT (6 + MAX_CPU_COUNT)

/** @brief Number of entries in the kernel's IDT. */
#define IDT_ENTRY_COUNT 256

/*******************************************************************************
 * STRUCTURES
 ******************************************************************************/

/** 
 * @brief Define the GDT pointer, contains the  address and limit of the GDT.
 */
struct gdt_ptr
{
    /** @brief The GDT size. */
    uint16_t size;

    /** @brief The GDT address. */
    address_t base;
}__attribute__((packed));

/** 
 * @brief Defines the gdt_ptr_t type as a shortcut for struct  gdt_ptr. 
 */
typedef struct gdt_ptr gdt_ptr_t;

/** 
 * @brief Define the IDT pointer, contains the  address and limit of the IDT.
 */
struct idt_ptr
{
    /** @brief The IDT size. */
    uint16_t size;

    /** @brief The IDT address. */
    address_t base;
}__attribute__((packed));

/** 
 * @brief Defines the idt_ptr_t type as a shortcut for struct idt_ptr. 
 */
typedef struct idt_ptr idt_ptr_t;

#endif /* #ifndef __I386_CPU_STRUCTS_H_ */