/*******************************************************************************
 * @file cpu_structs.c
 *
 * @see cpu_structs.h
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

#include <lib/stdint.h>       /* Generic int types */

/* UTK configuration file */
#include <config.h>

/* Header */
#include <cpu_structs.h>

/*******************************************************************************
 * GLOBAL VARIABLES
 ******************************************************************************/

/** @brief CPU GDT space in memory. */
uint64_t cpu_gdt[GDT_ENTRY_COUNT] __attribute__((aligned(16)));
/** @brief Kernel GDT structure */
gdt_ptr_t cpu_gdt_ptr __attribute__((aligned(16)));

/** @brief CPU IDT space in memory. */
uint64_t cpu_idt[IDT_ENTRY_COUNT] __attribute__((aligned(16)));
/** @brief Kernel IDT structure */
idt_ptr_t cpu_idt_ptr;

/** @brief Kernel stacks */
uint8_t cpu_stacks[MAX_CPU_COUNT][KERNEL_STACK_SIZE] __attribute__((aligned(8)));

/** @brief Booted CPU count */
uint32_t init_cpu_count;