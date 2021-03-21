/*******************************************************************************
 * @file cpu_structs.c
 *
 * @see cpu_structs.h
 * 
 * @author Alexy Torres Aurora Dugo
 *
 * @date 27/02/2021
 *
 * @version 1.0
 *
 * @brief i386 CPU structures.
 *
 * @details i386 CPU structures. IDT, GDT and CPU stacks are defined here
 *
 * @copyright Alexy Torres Aurora Dugo
 ******************************************************************************/

#include <stdint.h> /* Generic int types */
#include <cpu.h>    /* CPU related structures */

/* UTK configuration file */
#include <config.h>

/* Header */
#include <cpu_structs.h>

/*******************************************************************************
 * GLOBAL VARIABLES
 ******************************************************************************/

/** @brief CPU GDT space in memory. */
uint64_t cpu_gdt[GDT_ENTRY_COUNT] __attribute__((aligned(8)));
/** @brief Kernel GDT structure */
gdt_ptr_t cpu_gdt_ptr __attribute__((aligned(8)));

/** @brief CPU IDT space in memory. */
uint64_t cpu_idt[IDT_ENTRY_COUNT] __attribute__((aligned(8)));
/** @brief Kernel IDT structure */
idt_ptr_t cpu_idt_ptr __attribute__((aligned(8)));

/** @brief CPU TSS structures */
cpu_tss_entry_t cpu_tss[MAX_CPU_COUNT] __attribute__((aligned(8)));