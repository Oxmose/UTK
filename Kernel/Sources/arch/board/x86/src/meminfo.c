/*******************************************************************************
 * @file meminfo.c
 *
 * @see meminfo.h
 *
 * @author Alexy Torres Aurora Dugo
 *
 * @date 02/03/2020
 *
 * @version 2.0
 *
 * @brief Kernel memory detector.
 *
 * @details This module is used to detect the memory mapping of the system.
 *
 * @copyright Alexy Torres Aurora Dugo
 ******************************************************************************/

#include <stdint.h>        /* Generic int types */
#include <stddef.h>        /* Standard definitions */
#include <kernel_output.h> /* Kernel output methods */
#include <panic.h>         /* Kernel panic */
#include <multiboot.h>     /* Multiboot specification */

/* UTK configuration file */
#include <config.h>

/* Header file */
#include <meminfo.h>

/*******************************************************************************
 * GLOBAL VARIABLES
 ******************************************************************************/

/** @brief Kernel symbols mapping: Low startup address start. */
extern uint8_t _START_LOW_STARTUP_ADDR;
/** @brief Kernel symbols mapping: Low startup address end. */
extern uint8_t _END_LOW_STARTUP_ADDR;
/** @brief Kernel symbols mapping: High startup address start. */
extern uint8_t _START_HIGH_STARTUP_ADDR;
/** @brief Kernel symbols mapping: High startup address end. */
extern uint8_t _END_HIGH_STARTUP_ADDR;
/** @brief Kernel symbols mapping: Code address start. */
extern uint8_t _START_TEXT_ADDR;
/** @brief Kernel symbols mapping: Code address end. */
extern uint8_t _END_TEXT_ADDR;
/** @brief Kernel symbols mapping: RO data address start. */
extern uint8_t _START_RO_DATA_ADDR;
/** @brief Kernel symbols mapping: RO data address end. */
extern uint8_t _END_RODATA_ADDR;
/** @brief Kernel symbols mapping: Data address start. */
extern uint8_t _START_DATA_ADDR;
/** @brief Kernel symbols mapping: Data address end. */
extern uint8_t _END_DATA_ADDR;
/** @brief Kernel symbols mapping: BSS address start. */
extern uint8_t _START_BSS_ADDR;
/** @brief Kernel symbols mapping: BSS address end. */
extern uint8_t _END_BSS_ADDR;
/** @brief Kernel symbols mapping: Stacks address start. */
extern uint8_t _KERNEL_STACKS_BASE;
/** @brief Kernel symbols mapping: Stacks address end. */
extern uint8_t _KERNEL_STACKS_SIZE;
/** @brief Kernel symbols mapping: Heap address start. */
extern uint8_t _KERNEL_HEAP_BASE;
/** @brief Kernel symbols mapping: Heap address end. */
extern uint8_t _KERNEL_HEAP_SIZE;

/** @brief Multiboot memory pointer, fileld by the bootloader. */
extern multiboot_info_t* _kernel_multiboot_ptr;

/** @brief Memory map storage as an array of range. */
mem_range_t memory_map_data[MAX_MEMORY_REGION_DETECT];

/** @brief Memory map structure's size. */
static size_t hw_map_size;

/*******************************************************************************
 * FUNCTIONS
 ******************************************************************************/
#if 0
static void print_kernel_map(void)
{
    KERNEL_INFO("=== Kernel memory layout\n");
    KERNEL_INFO("Startup low     0x%p -> 0x%p | %17uKB\n",
                    &_START_LOW_STARTUP_ADDR,
                    &_END_LOW_STARTUP_ADDR,
                    ((uintptr_t)&_END_LOW_STARTUP_ADDR - 
                    (uintptr_t)&_START_LOW_STARTUP_ADDR) >> 10);
    KERNEL_INFO("Startup high    0x%p -> 0x%p | %17uKB\n",
                    &_START_HIGH_STARTUP_ADDR,
                    &_END_HIGH_STARTUP_ADDR,
                    ((uintptr_t)&_END_HIGH_STARTUP_ADDR - 
                    (uintptr_t)&_START_HIGH_STARTUP_ADDR) >> 10);
    KERNEL_INFO("Code            0x%p -> 0x%p | %17uKB\n",
                    &_START_TEXT_ADDR,
                    &_END_TEXT_ADDR,
                    ((uintptr_t)&_END_TEXT_ADDR - 
                    (uintptr_t)&_START_TEXT_ADDR) >> 10);
    KERNEL_INFO("RO-Data         0x%p -> 0x%p | %17uKB\n",
                    &_START_RO_DATA_ADDR,
                    &_END_RODATA_ADDR,
                    ((uintptr_t)&_END_RODATA_ADDR - 
                    (uintptr_t)&_START_RO_DATA_ADDR) >> 10);
    KERNEL_INFO("Data            0x%p -> 0x%p | %17uKB\n",
                    &_START_DATA_ADDR,
                    &_END_DATA_ADDR,
                    ((uintptr_t)&_END_DATA_ADDR - 
                    (uintptr_t)&_START_DATA_ADDR) >> 10);
    KERNEL_INFO("BSS             0x%p -> 0x%p | %17uKB\n",
                    &_START_BSS_ADDR,
                    &_END_BSS_ADDR,
                    ((uintptr_t)&_END_BSS_ADDR - 
                    (uintptr_t)&_START_BSS_ADDR) >> 10);
    KERNEL_INFO("Stacks          0x%p -> 0x%p | %17uKB\n",
                    &_KERNEL_STACKS_BASE,
                    &_KERNEL_STACKS_BASE + (uintptr_t)&_KERNEL_STACKS_SIZE,
                    ((uintptr_t)&_KERNEL_STACKS_SIZE) >> 10);
    KERNEL_INFO("Heap            0x%p -> 0x%p | %17uKB\n",
                    &_KERNEL_HEAP_BASE,
                    &_KERNEL_HEAP_BASE + (uintptr_t)&_KERNEL_HEAP_SIZE,
                    ((uintptr_t)&_KERNEL_HEAP_SIZE) >> 10);
}
#endif
OS_RETURN_E memory_map_init(void)
{
    multiboot_memory_map_t* mmap;
    multiboot_memory_map_t* mmap_end;
    uint32_t i;

    /* Print inital memory mapping */
    //print_kernel_map();


    /* Update memory position in high memory */
    _kernel_multiboot_ptr = (multiboot_info_t*)
                            ((uintptr_t)_kernel_multiboot_ptr + 
                             KERNEL_MEM_OFFSET);
    //KERNEL_DEBUG("Reading memory configuration from 0x%p\n", 
    //             _kernel_multiboot_ptr);

    /* Copy multiboot data in upper memory */
    mmap = (multiboot_memory_map_t*)
           (uintptr_t)(_kernel_multiboot_ptr->mmap_addr + KERNEL_MEM_OFFSET);
    mmap_end = (multiboot_memory_map_t*)
               ((uintptr_t)mmap + _kernel_multiboot_ptr->mmap_length);

    i = 0;
    while(mmap < mmap_end)
    {
#ifdef ARCH_32_BITS
        /* Everything over the 4G limit is not registered on 32 bits systems */
        if(mmap->addr > 0xFFFFFFFFULL)
        {
            KERNEL_DEBUG("HM detection, skipped region at 0x%llX\n", mmap->addr);
            mmap = (multiboot_memory_map_t*)
                   ((uintptr_t)mmap + mmap->size + sizeof(mmap->size));

            continue;
        }
#endif
        memory_map_data[i].base  = (uintptr_t)mmap->addr;
        memory_map_data[i].limit = (uintptr_t)mmap->addr + (uintptr_t)mmap->len;
        memory_map_data[i].type  = mmap->type;

        ++i;
        mmap = (multiboot_memory_map_t*)
               ((uintptr_t)mmap + mmap->size + sizeof(mmap->size));
    }
    hw_map_size = i;

    KERNEL_INFO("=== Hardware memory map\n");
    for(i = 0; i < hw_map_size; ++i)
    {
        kernel_info("Area 0x%p -> 0x%p | %02d | %17uKB\n",
                    memory_map_data[i].base,
                    memory_map_data[i].limit - 1,
                    memory_map_data[i].type,
                    (uintptr_t)(memory_map_data[i].limit - 
                                memory_map_data[i].base) >> 10
                    );
    }
    return OS_NO_ERR;
}