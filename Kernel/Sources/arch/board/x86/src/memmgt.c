/*******************************************************************************
 * @file memmgt.c
 *
 * @see memmgt.h
 *
 * @author Alexy Torres Aurora Dugo
 *
 * @date 02/03/2020
 *
 * @version 2.0
 *
 * @brief Kernel physical memory manager.
 *
 * @details This module is used to detect the memory mapping of the system and 
 * manage physical memory.
 *
 * @copyright Alexy Torres Aurora Dugo
 ******************************************************************************/

#include <stdint.h>        /* Generic int types */
#include <stddef.h>        /* Standard definitions */
#include <kernel_output.h> /* Kernel output methods */
#include <panic.h>         /* Kernel panic */
#include <multiboot.h>     /* Multiboot specification */
#include <queue.h>         /* Queue structures */
#include <kheap.h>         /* Kernel heap allocator */

/* UTK configuration file */
#include <config.h>

/* Header file */
#include <memmgt.h>

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
/** @brief Kernel memory end address. */
extern uint8_t _KERNEL_MEMORY_END;

/** @brief Multiboot memory pointer, fileld by the bootloader. */
extern multiboot_info_t* _kernel_multiboot_ptr;

/** @brief Hardware memory map storage linked list. */
static queue_t* hw_memory_map;

/** @brief Free memory map storage linked list. */
static queue_t* free_memory_map;

/** @brief Stores the total available memory */
static uintptr_t available_memory;
/*******************************************************************************
 * FUNCTIONS
 ******************************************************************************/

static void print_kernel_map(void)
{
    KERNEL_INFO("=== Kernel memory layout\n");
    KERNEL_INFO("Startup low     0x%p -> 0x%p | "PRIPTR"KB\n",
                    &_START_LOW_STARTUP_ADDR,
                    &_END_LOW_STARTUP_ADDR,
                    ((uintptr_t)&_END_LOW_STARTUP_ADDR - 
                    (uintptr_t)&_START_LOW_STARTUP_ADDR) >> 10);
    KERNEL_INFO("Startup high    0x%p -> 0x%p | "PRIPTR"KB\n",
                    &_START_HIGH_STARTUP_ADDR,
                    &_END_HIGH_STARTUP_ADDR,
                    ((uintptr_t)&_END_HIGH_STARTUP_ADDR - 
                    (uintptr_t)&_START_HIGH_STARTUP_ADDR) >> 10);
    KERNEL_INFO("Code            0x%p -> 0x%p | "PRIPTR"KB\n",
                    &_START_TEXT_ADDR,
                    &_END_TEXT_ADDR,
                    ((uintptr_t)&_END_TEXT_ADDR - 
                    (uintptr_t)&_START_TEXT_ADDR) >> 10);
    KERNEL_INFO("RO-Data         0x%p -> 0x%p | "PRIPTR"KB\n",
                    &_START_RO_DATA_ADDR,
                    &_END_RODATA_ADDR,
                    ((uintptr_t)&_END_RODATA_ADDR - 
                    (uintptr_t)&_START_RO_DATA_ADDR) >> 10);
    KERNEL_INFO("Data            0x%p -> 0x%p | "PRIPTR"KB\n",
                    &_START_DATA_ADDR,
                    &_END_DATA_ADDR,
                    ((uintptr_t)&_END_DATA_ADDR - 
                    (uintptr_t)&_START_DATA_ADDR) >> 10);
    KERNEL_INFO("BSS             0x%p -> 0x%p | "PRIPTR"KB\n",
                    &_START_BSS_ADDR,
                    &_END_BSS_ADDR,
                    ((uintptr_t)&_END_BSS_ADDR - 
                    (uintptr_t)&_START_BSS_ADDR) >> 10);
    KERNEL_INFO("Stacks          0x%p -> 0x%p | "PRIPTR"KB\n",
                    &_KERNEL_STACKS_BASE,
                    &_KERNEL_STACKS_BASE + (uintptr_t)&_KERNEL_STACKS_SIZE,
                    ((uintptr_t)&_KERNEL_STACKS_SIZE) >> 10);
    KERNEL_INFO("Heap            0x%p -> 0x%p | "PRIPTR"KB\n",
                    &_KERNEL_HEAP_BASE,
                    &_KERNEL_HEAP_BASE + (uintptr_t)&_KERNEL_HEAP_SIZE,
                    ((uintptr_t)&_KERNEL_HEAP_SIZE) >> 10);
}

static void detect_memory(void)
{
    multiboot_memory_map_t* mmap;
    multiboot_memory_map_t* mmap_end;
    mem_range_t*            mem_range;
    OS_RETURN_E             error;
    queue_node_t*           node;
    uint32_t                curr_prio;

    /* Copy multiboot data in upper memory */
    mmap = (multiboot_memory_map_t*)
           (uintptr_t)(_kernel_multiboot_ptr->mmap_addr + KERNEL_MEM_OFFSET);
    mmap_end = (multiboot_memory_map_t*)
               ((uintptr_t)mmap + _kernel_multiboot_ptr->mmap_length);

    hw_memory_map = queue_create_queue(QUEUE_ALLOCATOR(kmalloc, kfree), 
                                       &error);
    if(error != OS_NO_ERR)
    {
        KERNEL_ERROR("Could not allocate HW memory map queue\n");
        kernel_panic(error);
    }
    free_memory_map = queue_create_queue(QUEUE_ALLOCATOR(kmalloc, kfree), 
                                         &error);
    if(error != OS_NO_ERR)
    {
        KERNEL_ERROR("Could not allocate free memory map queue\n");
        kernel_panic(error);
    }
    
    curr_prio = (uint32_t)-1;
    available_memory = 0;
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
        mem_range = kmalloc(sizeof(mem_range_t));
        if(mem_range == NULL)
        {
            KERNEL_ERROR("Could not allocate memory range structure\n");
            kernel_panic(OS_ERR_MALLOC);
        }
        node = queue_create_node(mem_range, 
                                 QUEUE_ALLOCATOR(kmalloc, kfree), 
                                 &error);
        mem_range->base  = (uintptr_t)mmap->addr;
        mem_range->limit = (uintptr_t)mmap->addr + (uintptr_t)mmap->len;
        mem_range->type  = mmap->type;

        /* Low memory is treated as HW */
        if(mmap->type == MULTIBOOT_MEMORY_AVAILABLE && 
           mmap->addr >= KERNEL_MEM_START)
        {

            error = queue_push_prio(node, free_memory_map, curr_prio--);
            if(error != OS_NO_ERR)
            {
                KERNEL_ERROR("Could not enqueue memory range node\n");
                kernel_panic(error);
            }
            available_memory += (uintptr_t)mmap->len;
        }
        else 
        {
            error = queue_push_prio(node, hw_memory_map, curr_prio--);
            if(error != OS_NO_ERR)
            {
                KERNEL_ERROR("Could not enqueue memory range node\n");
                kernel_panic(error);
            }
        }

        mmap = (multiboot_memory_map_t*)
               ((uintptr_t)mmap + mmap->size + sizeof(mmap->size));
    }
}

static void setup_mem_table(void)
{
    uintptr_t     free_mem_head;
    mem_range_t*  mem_range;
    queue_node_t* cursor;
    /* The first regions we should use is above 1MB (this is where the kernel
     * should be loaded). We should set this regions as active. We also set the
     * first address that is free in this region. This should be just after the 
     * end of the kernel.
     */
    free_mem_head = (uintptr_t)&_KERNEL_MEMORY_END - KERNEL_MEM_OFFSET;
    if(free_mem_head % KERNEL_FRAME_SIZE != 0)
    {
        free_mem_head = (free_mem_head % KERNEL_FRAME_SIZE) + KERNEL_FRAME_SIZE;
    }
    
    cursor = free_memory_map->head;
    while(cursor != NULL)
    {
        mem_range = (mem_range_t*)cursor->data;
        if(mem_range->base >= KERNEL_MEM_START)
        {
            if(mem_range->base > free_mem_head || mem_range->limit < free_mem_head)
            {
                KERNEL_ERROR("Kernel was not loaded in the first available"
                             " memory region");
                kernel_panic(OS_ERR_OUT_OF_BOUND);
            }   
            break;
        }
        cursor = cursor->next;
    }
    if(cursor == NULL)
    {
        KERNEL_ERROR("Kernel was not loaded in the first available"
                     " memory region");
        kernel_panic(OS_ERR_OUT_OF_BOUND);
    }

    /* Update free memory */
    KERNEL_DEBUG("Kernel memory end: 0x%p\n", free_mem_head);
    available_memory -= free_mem_head - KERNEL_MEM_START;

}

OS_RETURN_E memory_map_init(void)
{
    queue_node_t* cursor;
    mem_range_t*  mem_range;
    /* Print inital memory mapping */
    print_kernel_map();

    /* Update memory position in high memory */
    _kernel_multiboot_ptr = (multiboot_info_t*)
                            ((uintptr_t)_kernel_multiboot_ptr + 
                             KERNEL_MEM_OFFSET);
    KERNEL_DEBUG("Reading memory configuration from 0x%p\n", 
                 _kernel_multiboot_ptr);

    /* Detect memory */
    detect_memory();

    /* Setup the memory table */
    setup_mem_table();

    /* Print detected memory inforation */
    KERNEL_INFO("=== Hardware memory map\n");
    cursor = hw_memory_map->head;
    while(cursor != NULL)
    {
        mem_range = (mem_range_t*)cursor->data;
        KERNEL_INFO("Area 0x%p -> 0x%p | %02d | " PRIPTR "KB\n",
                    mem_range->base,
                    mem_range->limit,
                    mem_range->type,
                    (uintptr_t)(mem_range->limit - mem_range->base) >> 10);
        cursor = cursor->next;
    }
    KERNEL_INFO("=== Free memory map\n");
    cursor = free_memory_map->head;
    while(cursor != NULL)
    {
        mem_range = (mem_range_t*)cursor->data;
        KERNEL_INFO("Area 0x%p -> 0x%p | " PRIPTR "KB\n",
                    mem_range->base,
                    mem_range->limit,
                    (uintptr_t)(mem_range->limit - mem_range->base) >> 10);
        cursor = cursor->next;
    }
    KERNEL_INFO("Total available memory: " PRIPTR "KB\n",
                 available_memory >> 10);
    return OS_NO_ERR;
}