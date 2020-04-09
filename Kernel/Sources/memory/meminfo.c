/*******************************************************************************
 * @file meminfo.c
 *
 * @see meminfo.h
 *
 * @author Alexy Torres Aurora Dugo
 *
 * @date 25/01/2018
 *
 * @version 2.0
 *
 * @brief Kernel memory detector.
 *
 * @details This module is used to detect the memory mapping of the system.
 *
 * @copyright Alexy Torres Aurora Dugo
 ******************************************************************************/

#include <lib/stdint.h>       /* Generic int types */
#include <lib/stddef.h>       /* Standard definitions */
#include <core/multiboot.h>   /* Multiboot definitions */
#include <io/kernel_output.h> /* Kernel output methods */
#include <core/panic.h>            /* Kernel panic */

/* UTK configuration file */
#include <config.h>

/* Header file */
#include <memory/meminfo.h>

/*******************************************************************************
 * GLOBAL VARIABLES
 ******************************************************************************/

/** @brief Memory map structure's size. */
uint32_t          memory_map_size;

/** @brief Memory map storage as an array of range. */
mem_range_t       memory_map_data[100];

/** @brief Multiboot memory pointer, fileld by the bootloader. */
multiboot_info_t* multiboot_data_ptr;

/** @brief Kernel memory start (physical). */
extern uint8_t _kernel_start_phys;

/** @brief Kernel memory start (virtual). */
extern uint8_t _kernel_start;

/** @brief Kernel code start (virtual). */
extern uint8_t _kernel_code_start;

/** @brief Kernel code end (virtual). */
extern uint8_t _kernel_code_end;

/** @brief Kernel read only data start (virtual). */
extern uint8_t _kernel_rodata_start;

/** @brief Kernel read only data end (virtual). */
extern uint8_t _kernel_rodata_end;

/** @brief Kernel data start (virtual). */
extern uint8_t _kernel_data_start;

/** @brief Kernel data end (virtual). */
extern uint8_t _kernel_data_end;

/** @brief Kernel bss start (virtual). */
extern uint8_t _kernel_bss_start;

/** @brief Kernel bss end (virtual). */
extern uint8_t _kernel_bss_end;

/** @brief Kernel structures start (virtual). */
extern uint8_t _kernel_struct_start;

/** @brief Kernel structures end (virtual). */
extern uint8_t _kernel_struct_end;

/** @brief Kernel static memory end (virtual). */
extern uint8_t _kernel_static_limit;

/** @brief Kernel heap start (virtual). */
extern uint8_t _kernel_heap_start;

/** @brief Kernel heap end (virtual). */
extern uint8_t _kernel_heap_end;

/** @brief Kernel memory end (virtual). */
extern uint8_t _kernel_end;

/** @brief Total ammount of memory in the system. */
static uint64_t total_memory;

/** @brief Static memory used by the kernel. */
static uint64_t static_used_memory;

/** @brief Kernel's heap used memory data. */
extern uint64_t kheap_mem_used;

/*******************************************************************************
 * FUNCTIONS
 ******************************************************************************/

OS_RETURN_E memory_map_init(void)
{
#if MEMORY_KERNEL_DEBUG
    kernel_serial_debug("Reading memory configuration from 0x%p\n", 
                        multiboot_data_ptr);
#endif 
    multiboot_memory_map_t* mmap;
    multiboot_memory_map_t* mmap_end;
    uint32_t i;

    /* Update memory poisition */
    multiboot_data_ptr = (multiboot_info_t*)
                            ((uint8_t*)multiboot_data_ptr + KERNEL_MEM_OFFSET);

    /* Copy multiboot data in upper memory */
    mmap = (multiboot_memory_map_t*)(uintptr_t)(multiboot_data_ptr->mmap_addr +
                                     KERNEL_MEM_OFFSET);
    mmap_end = (multiboot_memory_map_t*)((uintptr_t)mmap +
                                         multiboot_data_ptr->mmap_length);
    i = 0;
    while(mmap < mmap_end)
    {
        /* Everything over the 4G limit is not registered on 32 bits systems */
        if(sizeof(uintptr_t) <= 4 && i != 0 && (uintptr_t)mmap->addr == 0)
        {
            break;
        }

        total_memory += mmap->len;

        memory_map_data[i].base  = (uintptr_t)mmap->addr;
        memory_map_data[i].limit = (uintptr_t)mmap->addr + (uintptr_t)mmap->len;
        memory_map_data[i].type  = mmap->type;

        ++i;
        mmap = (multiboot_memory_map_t*)
               ((uintptr_t)mmap + mmap->size + sizeof(mmap->size));
    }
    memory_map_size = i;


    kernel_info("=== Kernel memory map\n");
    for(i = 0; i < memory_map_size; ++i)
    {
        kernel_info("Area 0x%p -> 0x%p | %02d | %17uKB\n",
                    memory_map_data[i].base,
                    memory_map_data[i].limit,
                    memory_map_data[i].type,
                    (uintptr_t)(memory_map_data[i].limit - 
                                memory_map_data[i].base) >> 10
                    );
    }

    kernel_info("=== Kernel memory layout\n");
    kernel_info("Code    0x%p -> 0x%p | %17uKB\n",
                    &_kernel_code_start,
                    &_kernel_code_end,
                    ((uintptr_t)&_kernel_code_end - 
                    (uintptr_t)&_kernel_code_start) >> 10);
    kernel_info("RO-Data 0x%p -> 0x%p | %17uKB\n",
                    &_kernel_rodata_start,
                    &_kernel_rodata_end,
                    ((uintptr_t)&_kernel_rodata_end - 
                    (uintptr_t)&_kernel_rodata_start) >> 10);
    kernel_info("Data    0x%p -> 0x%p | %17uKB\n",
                    &_kernel_data_start,
                    &_kernel_data_end,
                    ((uintptr_t)&_kernel_data_end - 
                    (uintptr_t)&_kernel_data_start) >> 10);
    kernel_info("BSS     0x%p -> 0x%p | %17uKB\n",
                    &_kernel_bss_start,
                    &_kernel_bss_end,
                    ((uintptr_t)&_kernel_bss_end - 
                    (uintptr_t)&_kernel_bss_start) >> 10);
    kernel_info("Config  0x%p -> 0x%p | %17uKB\n",
                    &_kernel_struct_start,
                    &_kernel_struct_end,
                    ((uintptr_t)&_kernel_struct_end - 
                    (uintptr_t)&_kernel_struct_start) >> 10);
    kernel_info("Heap    0x%p -> 0x%p | %17uKB\n",
                    &_kernel_heap_start,
                    &_kernel_heap_end,
                    ((uintptr_t)&_kernel_heap_end - 
                    (uintptr_t)&_kernel_heap_start) >> 10);


    return OS_NO_ERR;
}

uint64_t meminfo_kernel_heap_usage(void)
{
    return kheap_mem_used;
}

uint64_t meminfo_kernel_heap_size(void)
{
    return (uintptr_t)&_kernel_heap_end - (uintptr_t)&_kernel_heap_start;
}

uint64_t meminfo_kernel_memory_usage(void)
{
    return static_used_memory + meminfo_kernel_heap_usage();
}

uint64_t meminfo_kernel_total_size(void)
{
    return (uintptr_t)&_kernel_end - KERNEL_MEM_OFFSET;
}

uint64_t meminfo_get_memory_size(void)
{
    return total_memory;
}