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
    uintptr_t base;
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
    uintptr_t base;
}__attribute__((packed));

/** 
 * @brief Defines the idt_ptr_t type as a shortcut for struct idt_ptr. 
 */
typedef struct idt_ptr idt_ptr_t;

/** @brief Holds the CPU register values */
struct cpu_state
{
    /** @brief CPU's esp register. */
    uint32_t esp;
    /** @brief CPU's ebp register. */
    uint32_t ebp;
    /** @brief CPU's edi register. */
    uint32_t edi;
    /** @brief CPU's esi register. */
    uint32_t esi;
    /** @brief CPU's edx register. */
    uint32_t edx;
    /** @brief CPU's ecx register. */
    uint32_t ecx;
    /** @brief CPU's ebx register. */
    uint32_t ebx;
    /** @brief CPU's eax register. */
    uint32_t eax;
    
    /** @brief CPU's ss register. */
    uint32_t ss;
    /** @brief CPU's gs register. */
    uint32_t gs;
    /** @brief CPU's fs register. */
    uint32_t fs;
    /** @brief CPU's es register. */
    uint32_t es;
    /** @brief CPU's ds register. */
    uint32_t ds;
} __attribute__((packed));

/** 
 * @brief Defines cpu_state_t type as a shorcut for struct cpu_state.
 */
typedef struct cpu_state cpu_state_t;

/** @brief Hold the stack state before the interrupt */
struct stack_state
{
    /** @brief Interrupt's error code. */
    uint32_t error_code;
    /** @brief EIP of the faulting instruction. */
    uint32_t eip;
    /** @brief CS before the interrupt. */
    uint32_t cs;
    /** @brief EFLAGS before the interrupt. */
    uint32_t eflags;
} __attribute__((packed));

/** 
 * @brief Defines stack_state_t type as a shorcut for struct stack_state.
 */
typedef struct stack_state stack_state_t;

/**  
 * @brief CPU TSS abstraction structure. This is the representation the kernel 
 * has of an intel's TSS entry.
 */
struct cpu_tss_entry 
{
    uint32_t prev_tss;
    uint32_t esp0;
    uint32_t ss0;
    uint32_t esp1;
    uint32_t ss1;
    uint32_t esp2;
    uint32_t ss2;
    uint32_t cr3;
    uint32_t eip;
    uint32_t eflags;
    uint32_t eax;
    uint32_t ecx;
    uint32_t edx;
    uint32_t ebx;
    uint32_t esp;
    uint32_t ebp;
    uint32_t esi;
    uint32_t edi;
    uint32_t es;
    uint32_t cs;
    uint32_t ss;
    uint32_t ds;
    uint32_t fs;
    uint32_t gs;
    uint32_t ldt;
    uint16_t trap;
    uint16_t iomap_base;
} __attribute__((__packed__));

/** 
 * @brief Defines the cpu_tss_entry_t type as a shortcut for struct 
 * cpu_tss_entry. 
 */
typedef struct cpu_tss_entry cpu_tss_entry_t;

/**
 * @brief Defines he virtual CPU context for the i386 CPU.
 */
struct virtual_cpu_context
{
    /** @brief Thread's specific ESP registers. */
    uint32_t esp;
    /** @brief Thread's specific EBP registers. */
    uint32_t ebp;
    /** @brief Thread's specific EIP registers. */
    uint32_t eip;

     /** @brief Thread's CR3 page directory pointer. */
    uint32_t cr3;    
};

/** @brief Shortcut name for the struct virtual_cpu_context structure. */
typedef struct virtual_cpu_context virtual_cpu_context_t;

struct kernel_thread;
typedef struct kernel_thread kernel_thread_t;

/*******************************************************************************
 * GLOBAL VARIABLES
 ******************************************************************************/

/** @brief CPU GDT space in memory. */
extern uint64_t cpu_gdt[GDT_ENTRY_COUNT];
/** @brief Kernel GDT structure */
extern gdt_ptr_t cpu_gdt_ptr;

/** @brief CPU IDT space in memory. */
extern uint64_t cpu_idt[IDT_ENTRY_COUNT];
/** @brief Kernel IDT structure */
extern idt_ptr_t cpu_idt_ptr;

/** @brief CPU TSS structures */
extern cpu_tss_entry_t cpu_tss[MAX_CPU_COUNT];

/** @brief Kernel stacks */
extern uint8_t cpu_stacks[MAX_CPU_COUNT][KERNEL_STACK_SIZE];

#endif /* #ifndef __I386_CPU_STRUCTS_H_ */