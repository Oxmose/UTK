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

#include <stdint.h>               /* Generic int types */
#include <string.h>               /* Memcpy */
#include <stddef.h>               /* Standard definitions */
#include <kernel_output.h>        /* Kernel output methods */
#include <panic.h>                /* Kernel panic */
#include <multiboot.h>            /* Multiboot specification */
#include <queue.h>                /* Queue structures */
#include <kheap.h>                /* Kernel heap allocator */
#include <scheduler.h>            /* Scheduler */
#include <arch_memmgt.h>          /* Paging information */
#include <exceptions.h>           /* Exception management */
#include <interrupt_settings.h>   /* Interrupt settings */
#include <critical.h>             /* Critical sections */
#include <cpu.h>                  /* CPU management */
#include <ctrl_block.h>           /* Kernel process structure */

/* UTK configuration file */
#include <config.h>

#ifdef TEST_MODE_ENABLED
#include <test_bank.h>
#endif

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

/** @brief Free kernel pages map storage linked list. */
static queue_t* free_kernel_pages;

/** @brief Stores the total available memory */
static uintptr_t available_memory;

/** @brief Kernel page directory array. */
static uintptr_t kernel_pgdir[KERNEL_PGDIR_SIZE] __attribute__((aligned(4096)));

/** @brief Kernel reserved page tables. */
static uintptr_t min_pgtable[KERNEL_RESERVED_PAGING][KERNEL_PGDIR_SIZE]
                                                __attribute__((aligned(4096)));

/** @brief Tells if paging is initialized. */
static uint8_t init = 0;

/** @brief Tells if paging is enabled. */
static uint8_t enabled = 0;

/** @brief Stores the frame reference table directory */
static uintptr_t frame_ref_dir[FRAME_REF_DIR_SIZE];

/*******************************************************************************
 * FUNCTIONS
 ******************************************************************************/

static void memory_acquire_ref(uintptr_t phys_addr)
{
    uint16_t   dir_entry;
    uint16_t   table_entry;
    uint32_t   int_state;
    uintptr_t* current_table;

    ENTER_CRITICAL(int_state);

    dir_entry   = phys_addr >> FRAME_REF_DIR_ENTRY_OFFSET;
    table_entry = (phys_addr >> FRAME_REF_TABLE_ENTRY_OFFSET) &
                  FRAME_REF_TABLE_ENTRY_OFFSET_MASK;

    current_table = (uintptr_t*)frame_ref_dir[dir_entry];

    if(current_table == NULL)
    {
        KERNEL_ERROR("Tried to acquire reference on non existing memory 0x%p\n",
                     phys_addr);
        KERNEL_PANIC(OS_ERR_UNAUTHORIZED_ACTION);
    }

    if(current_table[table_entry] == (uintptr_t)NULL)
    {
        KERNEL_ERROR("Tried to acquire reference on non existing memory 0x%p\n",
                     phys_addr);
        KERNEL_PANIC(OS_ERR_UNAUTHORIZED_ACTION);
    }

    /* Update reference count */
    ++current_table[table_entry];
    if((current_table[table_entry] & FRAME_REF_COUNT_MASK) == 
       FRAME_REF_COUNT_MASK)
    {
        KERNEL_ERROR("Exceeded reference count reached\n");
        KERNEL_PANIC(OS_ERR_UNAUTHORIZED_ACTION);
    }

    KERNEL_DEBUG(MEMMGT_DEBUG_ENABLED, 
                 "[MEMMGT] Acquired reference 0x%p", 
                 phys_addr);

    EXIT_CRITICAL(int_state);
}

static void memory_release_ref(uintptr_t phys_addr)
{
    uint16_t   dir_entry;
    uint16_t   table_entry;
    uint32_t   int_state;
    uintptr_t* current_table;

    ENTER_CRITICAL(int_state);

    dir_entry   = phys_addr >> FRAME_REF_DIR_ENTRY_OFFSET;
    table_entry = (phys_addr >> FRAME_REF_TABLE_ENTRY_OFFSET) &
                  FRAME_REF_TABLE_ENTRY_OFFSET_MASK;

    current_table = (uintptr_t*)frame_ref_dir[dir_entry];

    if(current_table == NULL)
    {
        KERNEL_ERROR("Tried to release reference on non existing memory 0x%p\n", 
                     phys_addr);
        KERNEL_PANIC(OS_ERR_UNAUTHORIZED_ACTION);
    }

    if(current_table[table_entry] == (uintptr_t)NULL)
    {
        KERNEL_ERROR("Tried to release reference on non existing memory 0x%p\n", 
                     phys_addr);
        KERNEL_PANIC(OS_ERR_UNAUTHORIZED_ACTION);
    }

    /* Update reference count */
    if((current_table[table_entry] & FRAME_REF_COUNT_MASK) == 0)
    {
        KERNEL_ERROR("Tried to release reference on free memory 0x%p\n", 
                     phys_addr);
        KERNEL_PANIC(OS_ERR_UNAUTHORIZED_ACTION);
    }

    --current_table[table_entry];

    KERNEL_DEBUG(MEMMGT_DEBUG_ENABLED, 
                 "[MEMMGT] Released reference 0x%p", 
                 phys_addr);

    /* Check if we should release the frame */
    if((current_table[table_entry] & FRAME_REF_COUNT_MASK) == 0 &&
       (current_table[table_entry] & FRAME_REF_IS_HW) == 0)
    {
        memory_free_frames((void*)phys_addr, 1);
    }

    EXIT_CRITICAL(int_state);
}

static void init_frame_ref_table(uintptr_t next_free_mem)
{
    queue_node_t* cursor;
    mem_range_t*  mem_range;
    uintptr_t     current_addr;
    uintptr_t     current_limit;
    uintptr_t     flags;
    uint16_t      dir_entry;
    uint16_t      table_entry;
    uintptr_t*    current_table;

    /* Align next free meme to next frame */
    next_free_mem += KERNEL_FRAME_SIZE - 
                     (next_free_mem & (KERNEL_FRAME_SIZE - 1));

    memset(frame_ref_dir, 0, FRAME_REF_DIR_SIZE);
    
    /* Walk the detected memory and create the reference directory */
    cursor = hw_memory_map->head;
    while(cursor != NULL)
    {
        mem_range = (mem_range_t*)cursor->data;

        /* Hardware should me declared by drivers, skip */
        if(mem_range->type != MULTIBOOT_MEMORY_AVAILABLE)
        {
            cursor = cursor->next;
            continue;
        }

        KERNEL_DEBUG(MEMMGT_DEBUG_ENABLED, 
                     "[MEMMGT] Adding region 0x%p -> 0x%p to reference table", 
                     mem_range->base, mem_range->limit);

        /* Check alignement */
        if((mem_range->base & (KERNEL_FRAME_SIZE - 1)) != 0 ||
           (mem_range->limit & (KERNEL_FRAME_SIZE - 1)) != 0)
        {
            KERNEL_ERROR("[MEMMGT] Memory manager cannot manage unaligned "
                         "memory 0x%p -> 0x%p, aligning to frame size\n",
                         mem_range->base,
                         mem_range->limit);
        }

        current_addr  = mem_range->base & (~(KERNEL_FRAME_SIZE - 1)); 
        current_limit = mem_range->limit & (~(KERNEL_FRAME_SIZE - 1));       
        while(current_addr < current_limit)
        {
            flags = FRAME_REF_PRESENT;
            /* If under 1MB or now available, set as hardware, ref count is 1 
             * since the kernel will always have access to hardware, even if not 
             * mapped.
             */
            if(current_addr <= KERNEL_MEM_START)
            {
                flags |= FRAME_REF_IS_HW;
            }
            else
            {
                /* If under the free memory head, we have 1 reference, else 0 
                 * since we are initializing the memory and no process was 
                 * already created.
                 */
                if(current_addr < next_free_mem)
                {
                    flags |= 1;
                }
            }

            /* Get the entries */
            dir_entry   = current_addr >> FRAME_REF_DIR_ENTRY_OFFSET;
            table_entry = (current_addr >> FRAME_REF_TABLE_ENTRY_OFFSET) &
                          FRAME_REF_TABLE_ENTRY_OFFSET_MASK;

            if(frame_ref_dir[dir_entry] == (uintptr_t)NULL)
            {
                frame_ref_dir[dir_entry] = 
                    (uintptr_t)kmalloc(FRAME_REF_TABLE_SIZE);
                if(frame_ref_dir[dir_entry] == (uintptr_t)NULL)
                {
                    KERNEL_ERROR("Could not allocate frame reference table\n");
                    KERNEL_PANIC(OS_ERR_MALLOC);
                }
                memset((void*)frame_ref_dir[dir_entry], 
                       0, 
                       FRAME_REF_TABLE_SIZE);
            }
            current_table = (uintptr_t*)frame_ref_dir[dir_entry];

            if(current_table[table_entry] != (uintptr_t)NULL)
            {
                KERNEL_ERROR("Reference table cannot have multiple ref 0x%p\n",
                             current_addr);
                KERNEL_PANIC(OS_ERR_UNAUTHORIZED_ACTION);
            }

            current_table[table_entry] = flags;

            current_addr += KERNEL_FRAME_SIZE;
        }
        cursor = cursor->next;
    }
}

/**
 * @brief Retrieves the start and end address of the kernel high startup 
 * section.
 * 
 * @details Retrieves the start and end address of the kernel high startup 
 * section. The addresses are stored in the buffer given as parameter. The 
 * function has no effect if the buffer are NULL.
 * 
 * @param[out] start The start address of the section.
 * @param[out] end The end address of the section.
 */
static void memory_get_khighstartup_range(uintptr_t* start, uintptr_t* end)
{
    if(start != NULL)
    {
        *start = (uintptr_t)&_START_HIGH_STARTUP_ADDR;
    }
    if(end != NULL)
    {
        *end = (uintptr_t)&_END_HIGH_STARTUP_ADDR;
    }
}

/**
 * @brief Retrieves the start and end address of the kernel text section.
 * 
 * @details Retrieves the start and end address of the kernel text section.
 * The addresses are stored in the buffer given as parameter. The function has
 * no effect if the buffer are NULL.
 * 
 * @param[out] start The start address of the section.
 * @param[out] end The end address of the section.
 */
static void memory_get_ktext_range(uintptr_t* start, uintptr_t* end)
{
    if(start != NULL)
    {
        *start = (uintptr_t)&_START_TEXT_ADDR;
    }
    if(end != NULL)
    {
        *end = (uintptr_t)&_END_TEXT_ADDR;
    }
}

/**
 * @brief Retrieves the start and end address of the kernel read only data
 * section.
 * 
 * @details Retrieves the start and end address of the kernel read only data
 * section. The addresses are stored in the buffer given as parameter. The 
 * function has no effect if the buffer are NULL.
 * 
 * @param[out] start The start address of the section.
 * @param[out] end The end address of the section.
 */
static void memory_get_krodata_range(uintptr_t* start, uintptr_t* end)
{
    if(start != NULL)
    {
        *start = (uintptr_t)&_START_RO_DATA_ADDR;
    }
    if(end != NULL)
    {
        *end = (uintptr_t)&_END_RODATA_ADDR;
    }
}

/**
 * @brief Retrieves the start and end address of the kernel data section.
 * 
 * @details Retrieves the start and end address of the kernel data section.
 * The addresses are stored in the buffer given as parameter. The function has
 * no effect if the buffer are NULL.
 * 
 * @param[out] start The start address of the section.
 * @param[out] end The end address of the section.
 */
static void memory_get_kdata_range(uintptr_t* start, uintptr_t* end)
{
    if(start != NULL)
    {
        *start = (uintptr_t)&_START_DATA_ADDR;
    }
    if(end != NULL)
    {
        *end = (uintptr_t)&_END_DATA_ADDR;
    }
}

/**
 * @brief Retrieves the start and end address of the kernel bss section.
 * 
 * @details Retrieves the start and end address of the kernel bss section.
 * The addresses are stored in the buffer given as parameter. The function has
 * no effect if the buffer are NULL.
 * 
 * @param[out] start The start address of the section.
 * @param[out] end The end address of the section.
 */
static void memory_get_kbss_range(uintptr_t* start, uintptr_t* end)
{
    if(start != NULL)
    {
        *start = (uintptr_t)&_START_BSS_ADDR;
    }
    if(end != NULL)
    {
        *end = (uintptr_t)&_END_BSS_ADDR;
    }
}

/**
 * @brief Retrieves the start and end address of the kernel stacks section.
 * 
 * @details Retrieves the start and end address of the kernel stacks section.
 * The addresses are stored in the buffer given as parameter. The function has
 * no effect if the buffer are NULL.
 * 
 * @param[out] start The start address of the section.
 * @param[out] end The end address of the section.
 */
static void memory_get_kstacks_range(uintptr_t* start, uintptr_t* end)
{
    if(start != NULL)
    {
        *start = (uintptr_t)&_KERNEL_STACKS_BASE;
    }
    if(end != NULL)
    {
        *end = (uintptr_t)&_KERNEL_STACKS_BASE + 
               (uintptr_t)&_KERNEL_STACKS_SIZE;
    }
}

/**
 * @brief Retrieves the start and end address of the kernel heap section.
 * 
 * @details Retrieves the start and end address of the kernel heap section.
 * The addresses are stored in the buffer given as parameter. The function has
 * no effect if the buffer are NULL.
 * 
 * @param[out] start The start address of the section.
 * @param[out] end The end address of the section.
 */
static void memory_get_kheap_range(uintptr_t* start, uintptr_t* end)
{
    if(start != NULL)
    {
        *start = (uintptr_t)&_KERNEL_HEAP_BASE;
    }
    if(end != NULL)
    {
        *end = (uintptr_t)&_KERNEL_HEAP_BASE + (uintptr_t)&_KERNEL_HEAP_SIZE;
    }
}

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
    mem_range_t*            mem_range2;
    OS_RETURN_E             error;
    queue_node_t*           node;
    queue_node_t*           node2;

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
        KERNEL_PANIC(error);
    }
    free_memory_map = queue_create_queue(QUEUE_ALLOCATOR(kmalloc, kfree), 
                                         &error);
    if(error != OS_NO_ERR)
    {
        KERNEL_ERROR("Could not allocate free memory map queue\n");
        KERNEL_PANIC(error);
    }
    
    available_memory = 0;
    while(mmap < mmap_end)
    {
        /* Everything over the 4G limit is not registered on 32 bits systems */
        if(mmap->addr > 0xFFFFFFFFULL)
        {
            KERNEL_DEBUG(MEMMGT_DEBUG_ENABLED, 
                         "[MEMMGT] HM detection, skipped region at 0x%llX", 
                         mmap->addr);
            mmap = (multiboot_memory_map_t*)
                   ((uintptr_t)mmap + mmap->size + sizeof(mmap->size));

            continue;
        }

        mem_range = kmalloc(sizeof(mem_range_t));
        if(mem_range == NULL)
        {
            KERNEL_ERROR("Could not allocate memory range structure\n");
            KERNEL_PANIC(OS_ERR_MALLOC);
        }
        node = queue_create_node(mem_range, 
                                 QUEUE_ALLOCATOR(kmalloc, kfree), 
                                 &error);
        if(error != OS_NO_ERR)
        {
            KERNEL_ERROR("Could not allocate memory range node\n");
            KERNEL_PANIC(error);
        }
        mem_range->base  = (uintptr_t)mmap->addr;
        mem_range->limit = (uintptr_t)mmap->addr + (uintptr_t)mmap->len;
        mem_range->type  = mmap->type;

        /* Low memory is treated as HW */
        if(mmap->type == MULTIBOOT_MEMORY_AVAILABLE && 
           mmap->addr >= KERNEL_MEM_START)
        {
            mem_range2 = kmalloc(sizeof(mem_range_t));
            if(mem_range2 == NULL)
            {
                KERNEL_ERROR("Could not allocate memory range structure\n");
                KERNEL_PANIC(OS_ERR_MALLOC);
            }
            node2 = queue_create_node(mem_range2, 
                                      QUEUE_ALLOCATOR(kmalloc, kfree), 
                                      &error);
            if(error != OS_NO_ERR)
            {
                KERNEL_ERROR("Could not allocate memory range node\n");
                KERNEL_PANIC(error);
            }
            mem_range2->base  = (uintptr_t)mmap->addr;
            mem_range2->limit = (uintptr_t)mmap->addr + (uintptr_t)mmap->len;
            mem_range2->type  = mmap->type;

            error = queue_push_prio(node2, free_memory_map, mem_range2->base);
            if(error != OS_NO_ERR)
            {
                KERNEL_ERROR("Could not enqueue memory range node\n");
                KERNEL_PANIC(error);
            }
            available_memory += (uintptr_t)mmap->len;
        }

        error = queue_push_prio(node, hw_memory_map, mem_range->base);
        if(error != OS_NO_ERR)
        {
            KERNEL_ERROR("Could not enqueue memory range node\n");
            KERNEL_PANIC(error);
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
    OS_RETURN_E   error;
    queue_node_t* node;

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
    
    cursor = free_memory_map->tail;
    while(cursor != NULL)
    {
        mem_range = (mem_range_t*)cursor->data;
        if(mem_range->base >= KERNEL_MEM_START)
        {
            if(mem_range->base > free_mem_head || 
               mem_range->limit < free_mem_head)
            {
                KERNEL_ERROR("Kernel was not loaded in the first available"
                             " memory region");
                KERNEL_PANIC(OS_ERR_OUT_OF_BOUND);
            }   
            break;
        }
        cursor = cursor->prev;
    }
    if(cursor == NULL)
    {
        KERNEL_ERROR("Kernel was not loaded in the first available"
                     " memory region");
        KERNEL_PANIC(OS_ERR_OUT_OF_BOUND);
    }

    /* Remove static kernel size from first region */
    mem_range = (mem_range_t*)free_memory_map->head->data;
    mem_range->base = free_mem_head;
    if(mem_range->base > mem_range->limit)
    {
        KERNEL_ERROR("Kernel was loaded on different regions\n");
        KERNEL_PANIC(OS_ERR_UNAUTHORIZED_ACTION);
    } 

    /* Initialize the frame reference table */
    init_frame_ref_table(free_mem_head);

    /* Initialize kernel pages */
    free_kernel_pages = queue_create_queue(QUEUE_ALLOCATOR(kmalloc, kfree), 
                                           &error);
    if(error != OS_NO_ERR)
    {
        KERNEL_ERROR("Could not initialize free kernel pages queue\n");
        KERNEL_PANIC(error);
    }
    mem_range = kmalloc(sizeof(mem_range_t));
    if(mem_range == NULL)
    {
        KERNEL_ERROR("Could not allocate kernel page range structure\n");
        KERNEL_PANIC(OS_ERR_MALLOC);
    }
    node = queue_create_node(mem_range, 
                                QUEUE_ALLOCATOR(kmalloc, kfree), 
                                &error);
    if(error != OS_NO_ERR)
    {
        KERNEL_ERROR("Could not initialize free kernel pages node\n");
        KERNEL_PANIC(error);
    }
    mem_range->base  = free_mem_head + KERNEL_MEM_OFFSET;
    mem_range->limit = (uintptr_t)PAGING_RECUR_PG_TABLE;
    mem_range->type  = MULTIBOOT_MEMORY_AVAILABLE;

    error = queue_push_prio(node, free_kernel_pages, free_mem_head);
    if(error != OS_NO_ERR)
    {
        KERNEL_ERROR("Could not enqueue free kernel pages node\n");
        KERNEL_PANIC(error);
    }

    /* Update free memory */
    KERNEL_DEBUG(MEMMGT_DEBUG_ENABLED, 
                 "[MEMMGT] Kernel physical memory end: 0x%p", 
                 free_mem_head);
    KERNEL_DEBUG(MEMMGT_DEBUG_ENABLED, 
                 "[MEMMGT] Kernel virtual memory end: 0x%p", 
                 free_mem_head + KERNEL_MEM_OFFSET);

    available_memory -= free_mem_head - KERNEL_MEM_START;
}

static void* get_block(queue_t* list, 
                       const size_t length, 
                       const MEM_ALLOC_START_E start_pt,
                       OS_RETURN_E* err)
{
    queue_node_t* cursor;
    queue_node_t* selected;
    mem_range_t*  range;

    uintptr_t   address;

    if(start_pt == MEM_ALLOC_BEGINING)
    {
        /* Search for the next block with this size */
        cursor = list->head;
        selected = NULL;
        while(cursor)
        {
            range = (mem_range_t*)cursor->data;
            if(range->limit - range->base >= length * KERNEL_FRAME_SIZE)
            {
                selected = cursor;
                break;
            }
            cursor = cursor->next;
        }
    }
    else 
    {
        /* Search for the next block with this size */
        cursor = list->tail;
        selected = NULL;
        while(cursor)
        {
            range = (mem_range_t*)cursor->data;
            if(range->limit - range->base >= length * KERNEL_FRAME_SIZE)
            {
                selected = cursor;
                break;
            }
            cursor = cursor->prev;
        }
    }
    if(selected == NULL)
    {
        if(err != NULL)
        {
            *err = OS_ERR_NO_MORE_FREE_MEM;
        }
        return NULL;
    }

    if(start_pt == MEM_ALLOC_BEGINING)
    {

        /* Save the block address */
        address = range->base;

        /* Modify the block */
        range->base += length * KERNEL_FRAME_SIZE;
    }
    else 
    {
        /* Save the block address */
        address = range->limit - length * KERNEL_FRAME_SIZE;

        /* Modify the block */
        range->limit = address;
    }

    if(range->base == range->limit)
    {
        /* Free node's data and delete node */
        kfree(selected->data);
        queue_remove(list, selected);
        queue_delete_node(&selected);
    }

    if(err != NULL)
    {
        *err = OS_NO_ERR;
    }
    return (void*)address;
}

static void add_block(queue_t* list, uintptr_t first_frame, const size_t length)
{
    queue_node_t* cursor;
    queue_node_t* new_node;
    queue_node_t* last_cursor;
    queue_node_t* save_cursor;
    mem_range_t*  range;
    uintptr_t     limit;
    OS_RETURN_E   err;

    if(list == NULL)
    {
        KERNEL_ERROR("Tried to add a memory block to a NULL list\n");
        KERNEL_PANIC(OS_ERR_NULL_POINTER);
    }

    limit = first_frame + length * KERNEL_FRAME_SIZE;


    cursor = list->head;
    last_cursor = NULL;
    while(cursor != NULL)
    {
        range = (mem_range_t*)cursor->data;
        /* Try to merge blocks */
        if(range->base == limit)
        {
            range->base = first_frame;
            
            /* After merge, maybe we can merge the next region */
            if(cursor->next != NULL)
            {
                save_cursor = cursor->next;
                if(((mem_range_t*)save_cursor->data)->limit == range->base)
                {
                    range->base = ((mem_range_t*)save_cursor->data)->base;
                    kfree(save_cursor->data);
                    queue_remove(list, save_cursor);
                    queue_delete_node(&save_cursor);
                }
            }

            break;
        }
        else if(range->limit == first_frame)
        {
            range->limit = limit;

            /* After merge, maybe we can merge the last region */
            if(last_cursor != NULL)
            {
                if(((mem_range_t*)last_cursor->data)->base == range->limit)
                {
                    range->limit = ((mem_range_t*)last_cursor->data)->limit;
                    kfree(last_cursor->data);
                    queue_remove(list, last_cursor);
                    queue_delete_node(&last_cursor);
                }
            }

            break;
        }
        else if(range->base <= first_frame && range->limit > first_frame)
        {
            KERNEL_ERROR("Tried to free an already free block\n");
            KERNEL_PANIC(OS_ERR_UNAUTHORIZED_ACTION);
        }
        else if(range->limit < first_frame)
        {
            /* Blocks are ordered by decerasing address, if the limit is ower 
             * than the block we add, there is no other range that can be merged
             */
            cursor = NULL;
            break;
        }
        last_cursor = cursor;
        cursor = cursor->next;
    }

    /* We did not find any range to merge */
    if(cursor == NULL)
    {
        range = kmalloc(sizeof(mem_range_t));
        if(range == NULL)
        {
            KERNEL_ERROR("Could not create node data in memory manager\n");
            KERNEL_PANIC(OS_ERR_MALLOC);
        }
        range->base    = first_frame;
        range->limit   = limit;
        range->type    = MULTIBOOT_MEMORY_AVAILABLE;

        new_node = queue_create_node(range, 
                                     QUEUE_ALLOCATOR(kmalloc, kfree), 
                                     &err);
        if(err != OS_NO_ERR)
        {
            KERNEL_ERROR("Could not crate queue node in memory manager\n");
            KERNEL_PANIC(err);
        }

        queue_push_prio(new_node, list, first_frame);        
    }
}


#define INVAL_PAGE(virt_addr)                       \
{                                                   \
    __asm__ __volatile__(                           \
        "invlpg (%0)": :"r"(virt_addr) : "memory"   \
    );                                              \
}

#define INVAL_TLB()                                         \
{                                                           \
    __asm__ __volatile__(                                   \
        "mov %%cr3, %%eax\n\tmov %%eax, %%cr3": : : "eax"   \
    );                                                      \
}

/** 
 * @brief Maps a kernel section to the memory.
 * 
 * @details Maps a kernel section to the memory. No frame are allocated as the 
 * memory should already be populated.
 * 
 * @param[in] start_addr The start address of the section.
 * @param[in] end_addr The end address of the section.
 * @param[in] read_only Set to 1 if the section is read only.
 * @param[in] exec Set to 1 if the section is executable.
 */
static void map_kernel_section(uintptr_t start_addr, uintptr_t end_addr,
                               const uint8_t read_only)
{
    uint16_t pg_dir_entry;
    uint16_t pg_table_entry;
    uint16_t min_pgtable_entry;

    /* Align start addr */
    start_addr = (uintptr_t)start_addr & PAGE_ALIGN_MASK;

    KERNEL_DEBUG(MEMMGT_DEBUG_ENABLED, 
                 "[MEMMGT] Mapping kernel section at 0x%p -> 0x%p", 
                 start_addr, 
                 end_addr);
    while(start_addr < end_addr)
    {
        /* Get entry indexes */
        pg_dir_entry      = (uintptr_t)start_addr >> PG_DIR_OFFSET;
        pg_table_entry    = ((uintptr_t)start_addr >> PG_TABLE_OFFSET) & 
                             PG_TABLE_OFFSET_MASK;
        min_pgtable_entry = (((uintptr_t)start_addr - KERNEL_MEM_OFFSET) >> 
                            PG_DIR_OFFSET) & PG_TABLE_OFFSET_MASK;
        /* Create the page table */
        min_pgtable[min_pgtable_entry][pg_table_entry] = 
            (start_addr - KERNEL_MEM_OFFSET) |
            PAGE_FLAG_SUPER_ACCESS |
            (read_only ? PAGE_FLAG_READ_ONLY : PAGE_FLAG_READ_WRITE) |
            PAGE_FLAG_CACHE_WB |
            PAGE_FLAG_PRESENT;

        /* Set the page directory */
        kernel_pgdir[pg_dir_entry] = 
            ((uintptr_t)&min_pgtable[min_pgtable_entry] - 
                KERNEL_MEM_OFFSET) |
            PG_DIR_FLAG_PAGE_SIZE_4KB |
            PG_DIR_FLAG_PAGE_SUPER_ACCESS |
            PG_DIR_FLAG_PAGE_READ_WRITE |
            PG_DIR_FLAG_PAGE_PRESENT;

        start_addr += KERNEL_PAGE_SIZE;
    }
}

 /**
 * @brief Handle a page fault exception.
 *
 * @details Handle a page fault exception raised by the cpu. The corresponding
 * registered handler will be called. If no handler is available, a panic is
 * raised.
 *
 * @param[in] cpu_state The cpu registers structure.
 * @param[in] int_id The exception number.
 * @param[in] stack_state The stack state before the exception that contain cs, eip,
 * error code and the eflags register value.
 */
static void paging_fault_general_handler(cpu_state_t* cpu_state, uintptr_t int_id,
                                         stack_state_t* stack_state)
{
    uintptr_t fault_address;

    /* If the exception line is not right */
    if(int_id != PAGE_FAULT_LINE)
    {
        KERNEL_ERROR("Page fault handler in wrong exception line.\n");
        panic(cpu_state, int_id, stack_state);
    }

    __asm__ __volatile__ (
        "mov %%cr2, %%eax\n\t"
        "mov %%eax, %0\n\t"
        : "=m" (fault_address)
        : /* no input */
        : "%eax"
    );

#ifdef TEST_MODE_ENABLED
    kernel_printf("[TESTMODE] Page fault at 0x%p\n", fault_address);
    kill_qemu();
#endif

    KERNEL_DEBUG(MEMMGT_DEBUG_ENABLED, 
                 "[MEMMGT] Page fault at 0x%p", 
                 fault_address);

    /* Kernel cannot handle page fault at the moment */
    panic(cpu_state, int_id, stack_state);
}

/**
 * @brief Tells if a memory region is already mapped in the current page tables.
 * 
 * @details Tells if a memory region is already mapped in the current page 
 * tables. Returns 0 if the region is not mapped, 1 otherwise.
 * 
 * @return Returns 0 if the region is not mapped, 1 otherwise.
 */
static uint8_t is_mapped(const uintptr_t start_addr, const size_t size)
{
    uintptr_t start_align;
    uintptr_t to_check;
    uint32_t  found;
    size_t    pgdir_entry;
    size_t    pgtable_entry;
    uint32_t* pgdir_rec_addr; 
    uint32_t* pgtable;

     /* Align addresses */
    start_align = start_addr & PAGE_ALIGN_MASK;

    /* Get mapping size */
    to_check = size + (start_addr - start_align);

    found = 0;
    while(to_check)
    {
        /* Get entries */
        pgdir_entry   = (start_align >> PG_DIR_OFFSET);
        pgtable_entry = (start_align >> PG_TABLE_OFFSET) & PG_TABLE_OFFSET_MASK;

        /* Check page directory presence and allocate if not present */
        pgdir_rec_addr = (uint32_t*)PAGING_RECUR_PG_DIR;
        if((pgdir_rec_addr[pgdir_entry] & PG_DIR_FLAG_PAGE_PRESENT) != 0)
        {
             /* Check present in page table */
            pgtable = (uint32_t*)(PAGING_RECUR_PG_TABLE + 
                                  KERNEL_PAGE_SIZE * 
                                  pgdir_entry);
            if((pgtable[pgtable_entry] & PG_DIR_FLAG_PAGE_PRESENT) != 0)
            {
                found = 1;
                break;
            }
        }

        start_align += KERNEL_PAGE_SIZE;
        if(to_check >= KERNEL_PAGE_SIZE)
        {
            to_check -= KERNEL_PAGE_SIZE;
        }
        else 
        {
            to_check = 0;
        }
    }

    return found;
}

/**
 * @brief Copies the free page table of the current process and return the copy.
 * 
 * @details Copies the free page table of the current process and return the 
 * copy. This function performs a deep copy of the table. Meaning that the two 
 * instance to the table are totally independant.
 * 
 * @return A deep copy of the currentp process free page table is returned.
 */
static queue_t* paging_copy_free_page_table(void)
{
    OS_RETURN_E   err;
    queue_t*      new_table;
    queue_t*      current_table;
    queue_node_t* cursor;
    queue_node_t* new_node;
    mem_range_t*  range;

    /* Create the new table */
    new_table = queue_create_queue(QUEUE_ALLOCATOR(kmalloc, kfree), &err);
    if(err != OS_NO_ERR)
    {
        KERNEL_ERROR("Could not create new free page table list[%d]\n", err);
        KERNEL_PANIC(err);
    }

    current_table = sched_get_current_process()->free_page_table;
    cursor = current_table->head;
    while(cursor)
    {
        /* Create range and node */
        range = kmalloc(sizeof(mem_range_t));
        if(range == NULL)
        {
            KERNEL_ERROR("Could not allocate new free page table range\n");
            KERNEL_PANIC(OS_ERR_MALLOC);
        }
        memcpy(range, cursor->data, sizeof(mem_range_t));
        new_node = queue_create_node(range, 
                                     QUEUE_ALLOCATOR(kmalloc, kfree), 
                                     &err);
        if(err != OS_NO_ERR)
        {
            KERNEL_ERROR("Could not create free page table node[%d]\n", err);
            KERNEL_PANIC(err);
        }

        /* Add range to list */
        err = queue_push(new_node, new_table);
        if(err != OS_NO_ERR)
        {
            KERNEL_ERROR("Could not push free page table node[%d]\n", err);
            KERNEL_PANIC(err);
        }

        /* Next entry */
        cursor = cursor->next;
    }

    return new_table;
}

/** 
 * @brief Maps a virtual address to the corresponding physical address.
 * 
 * @details Maps a virtual address to the corresponding physical address.
 * The allocation should be done prior to using this function as all it 
 * does is mapping the addresses together.
 * 
 * @param[in] virt_addr The virtual start address of the region to map.
 * @param[in] phys_addr The physical start address of the region to map.
 * @param[in] mapping_size The size of the region to map.
 * @param[in] read_only Set to 1 if the region should be read only.
 * @param[in] exec Set to 1 if the region contains executable code.
 * @param[in] cache_enabled Set to 1 if the cache should be enabled for this 
 * region.
 * @param[in] hardware Set to 1 if this region is memory mapped hardware.
 */
static void kernel_mmap_internal(const void* virt_addr,
                                 const void* phys_addr,
                                 const size_t mapping_size,
                                 const uint8_t read_only,
                                 const uint8_t exec,
                                 const uint8_t cache_enabled,
                                 const uint8_t hardware)
{
    (void)exec;

    uintptr_t   virt_align;
    uintptr_t   phys_align;
    size_t      pgdir_entry;
    size_t      pgtable_entry;
    size_t      to_map;
    uint32_t*   pgdir_rec_addr; 
    uint32_t*   pgtable;
    uint32_t    i;

    /* Align addresses */
    virt_align = (uintptr_t)virt_addr & PAGE_ALIGN_MASK;
    phys_align = (uintptr_t)phys_addr & PAGE_ALIGN_MASK;

    /* Get mapping size */
    to_map = mapping_size + ((uintptr_t)virt_addr - virt_align);

    /* Check for existing mapping */
    if(is_mapped(virt_align, to_map))
    {
        KERNEL_ERROR("Trying to remap memory\n");
        KERNEL_PANIC(OS_ERR_MAPPING_ALREADY_EXISTS);
    }

    while(to_map)
    {
        /* Get entries */
        pgdir_entry   = (virt_align >> PG_DIR_OFFSET);
        pgtable_entry = (virt_align >> PG_TABLE_OFFSET) & PG_TABLE_OFFSET_MASK;

        /* Check page directory presence and allocate if not present */
        pgdir_rec_addr = (uint32_t*)PAGING_RECUR_PG_DIR;
        if((pgdir_rec_addr[pgdir_entry] & PG_DIR_FLAG_PAGE_PRESENT) == 0)
        {
            pgtable = memory_alloc_frames(1);

            /* Map page */
            pgdir_rec_addr[pgdir_entry] = 
                (uintptr_t)pgtable |
                PG_DIR_FLAG_PAGE_SIZE_4KB |
                PG_DIR_FLAG_PAGE_SUPER_ACCESS |
                PG_DIR_FLAG_PAGE_READ_WRITE |
                PG_DIR_FLAG_PAGE_PRESENT;

            /* Get recursive virtual address */
            pgtable = (uint32_t*)(PAGING_RECUR_PG_TABLE + 
                                  KERNEL_PAGE_SIZE * 
                                  pgdir_entry);

            /* Zeroize entry */
            for(i = 0; i < KERNEL_PGDIR_SIZE; ++i)
            {
                pgtable[i] = 0;
            }
        }
        else 
        {
            /* Get recursive virtual address */
            pgtable = (uint32_t*)(PAGING_RECUR_PG_TABLE + 
                                  KERNEL_PAGE_SIZE * 
                                  pgdir_entry);
        }

        /* Map the entry */
        pgtable[pgtable_entry] = 
            phys_align |
            PAGE_FLAG_SUPER_ACCESS |
            (read_only ? PAGE_FLAG_READ_ONLY : PAGE_FLAG_READ_WRITE) |
            (cache_enabled ? PAGE_FLAG_CACHE_WB : PAGE_FLAG_CACHE_DISABLED) |
            (hardware ? PAGE_FLAG_HARDWARE : 0) |
            PAGE_FLAG_PRESENT;

        memory_acquire_ref((uintptr_t)phys_addr);

        KERNEL_DEBUG(MEMMGT_DEBUG_ENABLED, 
                     "[MEMMGT] Mapped page at 0x%p -> 0x%p", 
                     virt_align, 
                     phys_align);

        /* Update addresses and size */
        virt_align += KERNEL_PAGE_SIZE;
        phys_align += KERNEL_PAGE_SIZE;
        if(to_map >= KERNEL_PAGE_SIZE)
        {
            to_map -= KERNEL_PAGE_SIZE;
        }
        else 
        {
            to_map = 0;
        }
        
    }
}
/**
 * @brief Initializes paging structures for the kernel.
 *
 * @details Initializes paging structures for the kernel. This function will
 * select an available memory region to allocate the memory required for the
 * kernel. Then the kernel will be mapped to memory and paging is enabled for
 * the kernel.
 *
 * @return The success state or the error code.
 * - OS_NO_ERR is returned if no error is encountered.
 * - OS_ERR_NO_MORE_FREE_MEM is returned if there is not enough memory available
 *   to map the kernel.
 * 
 * @warning This function assumes the kernel is setup with basic paging.
 */

static OS_RETURN_E paging_init(void)
{
    uint32_t    i;
    uintptr_t   start_addr;
    uintptr_t   end_addr;
    OS_RETURN_E err;

    KERNEL_DEBUG(MEMMGT_DEBUG_ENABLED, "[MEMMGT] Initializing paging");

    /* Initialize kernel page directory */
    for(i = 0; i < KERNEL_PGDIR_SIZE; ++i)
    {
        kernel_pgdir[i] = 0;
    }

    /* Set recursive mapping */
    kernel_pgdir[KERNEL_PGDIR_SIZE - 1] = 
        ((uintptr_t)(kernel_pgdir) - KERNEL_MEM_OFFSET) |
        PG_DIR_FLAG_PAGE_SIZE_4KB |
        PG_DIR_FLAG_PAGE_SUPER_ACCESS |
        PG_DIR_FLAG_PAGE_READ_WRITE |
        PG_DIR_FLAG_PAGE_PRESENT;

    /* Map kernel code */
    memory_get_khighstartup_range(&start_addr, &end_addr);
    map_kernel_section(start_addr, end_addr, 1);
    memory_get_ktext_range(&start_addr, &end_addr);
    map_kernel_section(start_addr, end_addr, 1);

    /* Map kernel data */
    memory_get_krodata_range(&start_addr, &end_addr);
    map_kernel_section(start_addr, end_addr, 1);
    memory_get_kdata_range(&start_addr, &end_addr);
    map_kernel_section(start_addr, end_addr, 0);
    memory_get_kbss_range(&start_addr, &end_addr);
    map_kernel_section(start_addr, end_addr, 0);
    memory_get_kstacks_range(&start_addr, &end_addr);
    map_kernel_section(start_addr, end_addr, 0);
    memory_get_kheap_range(&start_addr, &end_addr);
    map_kernel_section(start_addr, end_addr, 0);

    /* Add page fault exception */
    err = kernel_exception_register_handler(PAGE_FAULT_LINE,
                                            paging_fault_general_handler);
                                            
    /* Set CR3 register */
    __asm__ __volatile__("mov %%eax, %%cr3": :"a"((uintptr_t)kernel_pgdir -
                                                  KERNEL_MEM_OFFSET));

    init    = 1;
    memory_paging_enable();

#ifdef TEST_MODE_ENABLED
    paging_test();
#endif

    return err;
}

OS_RETURN_E memory_manager_init(void)
{
    queue_node_t* cursor;
    mem_range_t*  mem_range;
    /* Print inital memory mapping */
    print_kernel_map();

    /* Update memory position in high memory */
    _kernel_multiboot_ptr = (multiboot_info_t*)
                            ((uintptr_t)_kernel_multiboot_ptr + 
                             KERNEL_MEM_OFFSET);
    KERNEL_DEBUG(MEMMGT_DEBUG_ENABLED, 
                 "[MEMMGT] Reading memory configuration from 0x%p", 
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
                    mem_range->limit - 1,
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
                    mem_range->limit - 1,
                    (uintptr_t)(mem_range->limit - mem_range->base) >> 10);
        cursor = cursor->next;
    }
    KERNEL_INFO("Total available memory: " PRIPTR "KB\n",
                 available_memory >> 10);

#ifdef TEST_MODE_ENABLED
    memmgr_test();
    memmgr_test2();
    memmgr_test3();
    memmgr_test4();
#endif

    return paging_init();
}

void* memory_alloc_frames(const size_t frame_count)
{
    uint32_t    int_state;
    void*       address;
    OS_RETURN_E err;

    ENTER_CRITICAL(int_state);

    address = get_block(free_memory_map, frame_count, MEM_ALLOC_BEGINING, &err);
    if(err != OS_NO_ERR)
    {
        KERNEL_ERROR("Could not allocate new frame\n");
        KERNEL_PANIC(err);
    }


    KERNEL_DEBUG(MEMMGT_DEBUG_ENABLED, 
                 "[MEMMGT] Allocated %u frames, at 0x%p", 
                 frame_count, address);

    available_memory -= KERNEL_FRAME_SIZE * frame_count;

    EXIT_CRITICAL(int_state);
    return (void*)address;
}

void memory_free_frames(void* frame_addr, const size_t frame_count)
{
    uint32_t      int_state;
    queue_node_t* cursor;
    mem_range_t*  mem_range;

    ENTER_CRITICAL(int_state);

    /* Check if the frame actually exists in free memory */
    cursor = hw_memory_map->head;
    while(cursor != NULL)
    {
        mem_range = (mem_range_t*)cursor->data;
        if(mem_range->type == MULTIBOOT_MEMORY_AVAILABLE &&
           mem_range->base <= (uintptr_t)frame_addr && 
           mem_range->limit >= (uintptr_t)frame_addr + 
                                frame_count * KERNEL_FRAME_SIZE)
        {
            break;
        }
        cursor = cursor->next;
    }
    if(cursor == NULL)
    {
        KERNEL_ERROR("Tried to free non existent frame\n");
        KERNEL_PANIC(OS_ERR_UNAUTHORIZED_ACTION);        
    }

    add_block(free_memory_map, (uintptr_t)frame_addr, frame_count);

    KERNEL_DEBUG(MEMMGT_DEBUG_ENABLED, 
                 "[MEMMGT] Deallocated %u frames, at 0x%p", 
                 frame_count, frame_addr);

    EXIT_CRITICAL(int_state);
}

void* memory_alloc_pages(const size_t page_count, 
                         const MEM_ALLOC_START_E start_pt)
{
    void*             address;
    uint32_t          int_state;
    OS_RETURN_E       err;    
    kernel_process_t* current_proc;
    queue_t*          table;

    ENTER_CRITICAL(int_state);

    current_proc = sched_get_current_process();
    if(current_proc == NULL)
    {
        /* No current process, use kernel page table */
        table = free_kernel_pages;
    }
    else 
    {
        table = current_proc->free_page_table;
    }

    address = get_block(table, page_count, start_pt, &err);
    if(err != OS_NO_ERR)
    {
        KERNEL_ERROR("Could not allocate new page\n");
        KERNEL_PANIC(err);
    }
    
    KERNEL_DEBUG(MEMMGT_DEBUG_ENABLED, 
                 "[MEMMGT] Allocated %u pages, at 0x%p", 
                 page_count, 
                 address);

    EXIT_CRITICAL(int_state);
    return (void*)address;
}

void memory_free_pages(void* page_addr, const size_t page_count)
{
    uint32_t          int_state;
    queue_t*          table;
    kernel_process_t* current_proc;

    ENTER_CRITICAL(int_state);

    current_proc = sched_get_current_process();
    if(current_proc == NULL)
    {
        /* No current process, use kernel page table */
        table = free_kernel_pages;
    }
    else 
    {
        table = current_proc->free_page_table;
    }

    add_block(table, (uintptr_t)page_addr, page_count);

    KERNEL_DEBUG(MEMMGT_DEBUG_ENABLED, 
                 "[MEMMGT] Deallocated %u pages, at 0x%p", 
                 page_count, 
                 page_addr);

    EXIT_CRITICAL(int_state);
}

queue_t* memory_create_free_page_table(OS_RETURN_E* err)
{
    queue_t*      new_queue;
    queue_node_t* node;
    mem_range_t*  mem_range;

    if(err == NULL)
    {
        return NULL;
    }
    
    /* Initialize kernel pages */
    new_queue = queue_create_queue(QUEUE_ALLOCATOR(kmalloc, kfree), 
                                   err);
    if(*err != OS_NO_ERR)
    {
        KERNEL_ERROR("Could not initialize free pages queue\n");
        return NULL;
    }
    mem_range = kmalloc(sizeof(mem_range_t));
    if(mem_range == NULL)
    {
        queue_delete_queue(&new_queue);
        KERNEL_ERROR("Could not allocate page range structure\n");
        *err = OS_ERR_MALLOC;
        return NULL;
    }
    node = queue_create_node(mem_range, 
                             QUEUE_ALLOCATOR(kmalloc, kfree), 
                             err);
    if(*err != OS_NO_ERR)
    {
        queue_delete_queue(&new_queue);
        kfree(mem_range);
        KERNEL_ERROR("Could not initialize free pages node\n");
        return NULL;
    }
    mem_range->base  = PROCESS_START_VIRT_SPACE;
    mem_range->limit = KERNEL_MEM_OFFSET;
    mem_range->type  = MULTIBOOT_MEMORY_AVAILABLE;

    *err = queue_push_prio(node, new_queue, PROCESS_START_VIRT_SPACE);
    if(*err != OS_NO_ERR)
    {
        queue_delete_node(&node);
        queue_delete_queue(&new_queue);
        kfree(mem_range);
        KERNEL_ERROR("Could not enqueue free pages node\n");
        return NULL;
    }

    return new_queue;
}

uintptr_t memory_alloc_stack(const size_t stack_size)
{
    uintptr_t pages;
    uint32_t  int_state;

    if((stack_size & (KERNEL_PAGE_SIZE - 1)) != 0)
    {
        KERNEL_ERROR("Tried to allocated non aligned stack\n");
        KERNEL_PANIC(OS_ERR_UNAUTHORIZED_ACTION);
    }

    ENTER_CRITICAL(int_state);

    /* Allocated frames and pages */
    pages  = (uintptr_t)memory_alloc_pages(stack_size / KERNEL_PAGE_SIZE, 
                                           MEM_ALLOC_END);

    KERNEL_DEBUG(MEMMGT_DEBUG_ENABLED, 
                 "[MEMMGT] Allocated stack at 0x%p", 
                 pages);

    /* Add mapping */
    memory_mmap((void*)pages, stack_size, 0, 0);

    EXIT_CRITICAL(int_state);

    return pages;
}

void memory_free_stack(uintptr_t virt_addr, const size_t stack_size)
{
    uint32_t  int_state;

    ENTER_CRITICAL(int_state);
    
    memory_free_pages((void*)virt_addr, stack_size / KERNEL_PAGE_SIZE);

    memory_munmap((void*)virt_addr, stack_size);

    EXIT_CRITICAL(int_state);
}

OS_RETURN_E memory_paging_enable(void)
{
    uint32_t int_state;

    if(init == 0)
    {
        return OS_ERR_PAGING_NOT_INIT;
    }

    if(enabled == 1)
    {
        return OS_NO_ERR;
    }

    ENTER_CRITICAL(int_state);

    /* Enable paging, write protect and PCID */
    __asm__ __volatile__("mov %%cr0, %%eax\n\t"
                         "or $0x80010000, %%eax\n\t"
                         "mov %%eax, %%cr0\n\t"
                         : : : "eax");

    if(cpu_is_pcid_capable() == 1)
    {
        __asm__ __volatile__("mov %%cr4, %%eax\n\t"
                            "or $0x00020000, %%eax\n\t"
                            "mov %%eax, %%cr4\n\t"
                            : : : "eax");
    }

    KERNEL_DEBUG(MEMMGT_DEBUG_ENABLED, "[MEMMGT] Paging enabled");

    enabled = 1;

    EXIT_CRITICAL(int_state);

    return OS_NO_ERR;
}

OS_RETURN_E memory_paging_disable(void)
{
    uint32_t int_state;

    if(init == 0)
    {
        return OS_ERR_PAGING_NOT_INIT;
    }

    if(enabled == 0)
    {
        return OS_NO_ERR;
    }

    ENTER_CRITICAL(int_state);

    /* Disable paging and write protect */
    __asm__ __volatile__("mov %%cr0, %%eax\n\t"
                         "and $0x7FF7FFFF, %%eax\n\t"
                         "mov %%eax, %%cr0" : : : "eax");

    KERNEL_DEBUG(MEMMGT_DEBUG_ENABLED, "[MEMMGT] Paging disabled");

    enabled = 0;

    EXIT_CRITICAL(int_state);

    return OS_NO_ERR;
}

void memory_mmap(const void* virt_addr,
                  const size_t mapping_size,
                  const uint8_t read_only,
                  const uint8_t exec)
{
    uint32_t    int_state;
    void*       phys_addr;
    uintptr_t   virt_align;
    size_t      to_map;

    /* Align addresses */
    virt_align = (uintptr_t)virt_addr & PAGE_ALIGN_MASK;

    /* Get mapping size */
    to_map = mapping_size + ((uintptr_t)virt_addr - virt_align);

    ENTER_CRITICAL(int_state);

    /* Allocate a new frame */
    phys_addr = memory_alloc_frames(to_map / KERNEL_FRAME_SIZE);

    kernel_mmap_internal(virt_addr, 
                         phys_addr, 
                         to_map, 
                         read_only, 
                         exec, 
                         1, 
                         0);

    EXIT_CRITICAL(int_state);
}

void memory_mmap_direct(const void* virt_addr,
                         const void* phys_addr,
                         const size_t mapping_size,
                         const uint8_t read_only,
                         const uint8_t exec,
                         const uint8_t is_hw)
{
    uint32_t    int_state;

    ENTER_CRITICAL(int_state);

    kernel_mmap_internal(virt_addr, 
                         phys_addr, 
                         mapping_size, 
                         read_only, 
                         exec, 
                         is_hw, 
                         is_hw);

    EXIT_CRITICAL(int_state);
}

void memory_munmap(const void* virt_addr, const size_t mapping_size)
{
    uintptr_t   end_map;
    uintptr_t   start_map;
    uint32_t    pgdir_entry;
    uint32_t    pgtable_entry;
    uint32_t*   pgdir_rec_addr;
    uint32_t*   pgtable;
    size_t      to_unmap;
    uint32_t    i;
    uint32_t    acc;
    uint32_t    int_state;

    KERNEL_DEBUG(MEMMGT_DEBUG_ENABLED, 
                 "[MEMMGT] Request unmappping at 0x%p (%uB)",
                 virt_addr, 
                 mapping_size);

    ENTER_CRITICAL(int_state);

    /* Compute physical memory size */
    end_map   = (uintptr_t)virt_addr + mapping_size;
    start_map = (uintptr_t)virt_addr & PAGE_ALIGN_MASK;

    if(end_map % KERNEL_PAGE_SIZE)
    {
        end_map &= PAGE_ALIGN_MASK;
        end_map += KERNEL_PAGE_SIZE;
    }
    to_unmap = end_map - start_map;

    while(to_unmap)
    {
        /* Get entries */
        pgdir_entry   = (start_map >> PG_DIR_OFFSET);
        pgtable_entry = (start_map >> PG_TABLE_OFFSET) & PG_TABLE_OFFSET_MASK;

        /* Check page directory presence and allocate if not present */
        pgdir_rec_addr = (uint32_t*)PAGING_RECUR_PG_DIR;
        if((pgdir_rec_addr[pgdir_entry] & PG_DIR_FLAG_PAGE_PRESENT) != 0)
        {
            /* Get recursive virtual address */
            pgtable = (uint32_t*)(PAGING_RECUR_PG_TABLE + 
                                  KERNEL_PAGE_SIZE * 
                                  pgdir_entry);

            if((pgtable[pgtable_entry] & PAGE_FLAG_PRESENT) != 0)
            {
                /* Unmap */
                KERNEL_DEBUG(MEMMGT_DEBUG_ENABLED, 
                             "[MEMMGT] Unmapped page at 0x%p", 
                             start_map);
                             
                /* Decrement the ref count and potentialy free frame */
                memory_release_ref(pgtable[pgtable_entry] & PG_ENTRY_MASK);
                pgtable[pgtable_entry] = 0;
                INVAL_PAGE(start_map);
            }

            /* If pagetable is empty, remove from pg dir */
            acc = 0;
            for(i = 0; i < KERNEL_PGDIR_SIZE; ++i)
            {
                acc |= pgtable[i] & PAGE_FLAG_PRESENT;
                if(acc)
                {
                    break;
                }
            }
            if(acc == 0)
            {
                memory_free_frames(
                    (void*)(pgdir_rec_addr[pgdir_entry] & PG_ENTRY_MASK), 1);
                pgdir_rec_addr[pgdir_entry] = 0;
            }
        }

        start_map += KERNEL_PAGE_SIZE;
        if(to_unmap >= KERNEL_PAGE_SIZE)
        {
            to_unmap -= KERNEL_PAGE_SIZE;
        }
        else 
        {
            to_unmap = 0;
        }
    }

    EXIT_CRITICAL(int_state);
}

OS_RETURN_E memory_copy_self_mapping(kernel_process_t* dst_process)
{
    uintptr_t*  new_pgdir_frame;
    uintptr_t*  new_pgdir_page;  
    uintptr_t*  new_pgtable_frame; 
    uintptr_t*  new_pgtable_page;
    uintptr_t*  current_pgdir;
    uintptr_t*  current_pgtable;
    uint32_t    i;
    uint32_t    j;

    if(dst_process == NULL)
    {
        return OS_ERR_NULL_POINTER;
    }

    /* Create a new page directory and map for kernel */
    new_pgdir_frame = (uintptr_t*)memory_alloc_frames(1);
    new_pgdir_page = (uintptr_t*)memory_alloc_pages(1, MEM_ALLOC_BEGINING);

    memory_mmap_direct(new_pgdir_page, 
                        new_pgdir_frame, 
                        KERNEL_PAGE_SIZE, 
                        0, 
                        0, 
                        0);

    /* Create temporary pages */
    new_pgtable_page = memory_alloc_pages(1, MEM_ALLOC_BEGINING);

    /* The current page directory is always recursively mapped */
    current_pgdir = (uintptr_t*)PAGING_RECUR_PG_DIR;

    /* Copy the page directory kernel entries, minus the recursive entry */    
    for(i = KERNEL_FIRST_PGDIR_ENTRY; i < KERNEL_PGDIR_SIZE - 1; ++i)
    {
        new_pgdir_page[i] = current_pgdir[i];
    }

    /* Set the recursive entry on the new parge directory */
    new_pgdir_page[KERNEL_PGDIR_SIZE - 1] = (uintptr_t)new_pgdir_frame    |
                                            PG_DIR_FLAG_PAGE_SIZE_4KB     |
                                            PG_DIR_FLAG_PAGE_SUPER_ACCESS |
                                            PG_DIR_FLAG_PAGE_READ_WRITE   |
                                            PG_DIR_FLAG_PAGE_PRESENT;
    memory_acquire_ref((uintptr_t)new_pgdir_frame);

    /* Copy the rest of the page table and set copy on write */
    for(i = 0; i < KERNEL_FIRST_PGDIR_ENTRY; ++i)
    {
        if((current_pgdir[i] & PG_DIR_FLAG_PAGE_PRESENT) != 0)
        {
            /* Get recursive virtual address */
            current_pgtable = (uintptr_t*)(PAGING_RECUR_PG_TABLE + 
                                           KERNEL_PAGE_SIZE * 
                                           i);

            /* Create new page table */
            new_pgtable_frame = memory_alloc_frames(1);     
            
            memory_mmap_direct(new_pgtable_page, 
                                new_pgtable_frame, 
                                KERNEL_PAGE_SIZE, 
                                0, 
                                0,
                                0);

            new_pgdir_page[i] = (current_pgtable[i] & ~PG_ENTRY_MASK) | 
                                (uintptr_t)new_pgtable_frame;
            memory_acquire_ref((uintptr_t)new_pgtable_frame);

            for(j = 0; j < KERNEL_PGDIR_SIZE; ++j)
            {
                if((current_pgtable[j] & PG_DIR_FLAG_PAGE_PRESENT) != 0)
                {
                    /* Copy mapping and set as read only / COW here the current
                     * process is also set as RO. Hardware mapping is copied as 
                     * is.
                     */
                    if((current_pgtable[j] & PAGE_FLAG_READ_WRITE) != 0 &&
                       (current_pgtable[j] & PAGE_FLAG_HARDWARE) == 0)
                    {
                        current_pgtable[j] = (current_pgtable[j] & 
                                              ~PAGE_FLAG_READ_WRITE) | 
                                             PAGE_FLAG_READ_ONLY     |
                                             PAGE_FLAG_COPY_ON_WRITE; 
                    }
                    new_pgtable_page[j] = current_pgtable[j];
                    /* Increment the reference count */
                    memory_acquire_ref(new_pgtable_page[j] & PG_ENTRY_MASK);
                }
                else 
                {
                    new_pgtable_page[j] = 0;
                }
            }

            memory_munmap(new_pgtable_page, KERNEL_PAGE_SIZE);
        }
        
    }

    /* Unmap the new page directory for the kernel */
    memory_munmap(new_pgdir_page, KERNEL_PAGE_SIZE);
    memory_free_pages(new_pgdir_page, 1);
    memory_free_pages(new_pgtable_page, 1);

    /* Set the destination process data */
    dst_process->page_dir = (uintptr_t)new_pgdir_frame;
    dst_process->free_page_table = paging_copy_free_page_table();

    return OS_NO_ERR;
}

uintptr_t memory_get_phys_addr(const uintptr_t virt_addr)
{
    size_t    pgdir_entry;
    size_t    pgtable_entry;
    uint32_t* pgdir_rec_addr; 
    uint32_t* pgtable;

    /* Get entries */
    pgdir_entry   = (virt_addr >> PG_DIR_OFFSET);
    pgtable_entry = (virt_addr >> PG_TABLE_OFFSET) & PG_TABLE_OFFSET_MASK;

    /* Check page directory presence and allocate if not present */
    pgdir_rec_addr = (uint32_t*)PAGING_RECUR_PG_DIR;
    if((pgdir_rec_addr[pgdir_entry] & PG_DIR_FLAG_PAGE_PRESENT) != 0)
    {
            /* Check present in page table */
        pgtable = (uint32_t*)(PAGING_RECUR_PG_TABLE + 
                                KERNEL_PAGE_SIZE * 
                                pgdir_entry);
        if((pgtable[pgtable_entry] & PG_DIR_FLAG_PAGE_PRESENT) != 0)
        {
            return (pgtable[pgtable_entry] & PG_ENTRY_MASK);
        }
    }

    return (uintptr_t)NULL;
}

OS_RETURN_E memory_declare_hw(const uintptr_t phys_addr, const size_t size)
{
    uintptr_t   current_addr;
    uintptr_t   flags;
    uint16_t    dir_entry;
    uint16_t    table_entry;
    uintptr_t*  current_table;
    uint32_t    i;
    uint32_t    int_state;

    OS_RETURN_E err;

    err = OS_NO_ERR;

    /* Align */
    current_addr = phys_addr & ~(KERNEL_FRAME_SIZE - 1);

    ENTER_CRITICAL(int_state);

    KERNEL_DEBUG(MEMMGT_DEBUG_ENABLED, 
                 "[MEMMGT] Adding region 0x%p -> 0x%p to reference table", 
                current_addr, current_addr + size);

    while(current_addr < phys_addr + size)
    {
        /* Check if not already declared */
        flags = FRAME_REF_PRESENT | FRAME_REF_IS_HW;

        /* Get the entries */
        dir_entry   = current_addr >> FRAME_REF_DIR_ENTRY_OFFSET;
        table_entry = (current_addr >> FRAME_REF_TABLE_ENTRY_OFFSET) &
                        FRAME_REF_TABLE_ENTRY_OFFSET_MASK;

        if(frame_ref_dir[dir_entry] == (uintptr_t)NULL)
        {
            frame_ref_dir[dir_entry] = 
                (uintptr_t)kmalloc(FRAME_REF_TABLE_SIZE);
            if(frame_ref_dir[dir_entry] == (uintptr_t)NULL)
            {
                KERNEL_ERROR("Could not allocate frame reference table\n");
                err = OS_ERR_MALLOC;
                break;
            }
            memset((void*)frame_ref_dir[dir_entry], 0, FRAME_REF_TABLE_SIZE);
        }
        current_table = (uintptr_t*)frame_ref_dir[dir_entry];

        if(current_table[table_entry] != (uintptr_t)NULL)
        {
            KERNEL_ERROR("Reference table cannot have multiple ref 0x%p\n",
                            current_addr);
            err = OS_ERR_UNAUTHORIZED_ACTION;
            current_addr -= KERNEL_FRAME_SIZE;
            break;
        }

        current_table[table_entry] = flags;

        current_addr += KERNEL_FRAME_SIZE;
    }

    /* Clean the memory if an error occured */
    if(err != OS_NO_ERR)
    {
        while(current_addr >= phys_addr)
        {
            dir_entry   = current_addr >> FRAME_REF_DIR_ENTRY_OFFSET;
            table_entry = (current_addr >> FRAME_REF_TABLE_ENTRY_OFFSET) &
                          FRAME_REF_TABLE_ENTRY_OFFSET_MASK;

            current_table = (uintptr_t*)frame_ref_dir[dir_entry];
            current_table[table_entry] = 0;

            /* We may need to free the table if empty */
            for(i = 0; i < FRAME_REF_TABLE_SIZE; ++i)
            {
                if(current_table[table_entry] != 0)
                {
                    break;
                }
            }
            if(i == FRAME_REF_TABLE_SIZE)
            {
                kfree((void*)frame_ref_dir[dir_entry]);
                frame_ref_dir[dir_entry] = (uintptr_t)NULL;
            }
            current_addr -= KERNEL_FRAME_SIZE;
        }
    }
    EXIT_CRITICAL(int_state);

    return err;
}

/* Test Mode */
#ifdef TEST_MODE_ENABLED
queue_t* paging_get_free_frames(void)
{
    return free_memory_map;
}

queue_t* paging_get_free_pages(void)
{
    return free_kernel_pages;
}

queue_t* test_page = NULL;
void testmode_paging_add_page(uintptr_t start, size_t size)
{
    OS_RETURN_E error;
    if(test_page == NULL)
    {
        test_page = queue_create_queue(QUEUE_ALLOCATOR(kmalloc, kfree), 
                                           &error);
        if(error != OS_NO_ERR)
        {
            KERNEL_ERROR("Could not initialize free kernel pages queue\n");
            KERNEL_PANIC(error);
        }
    }
    add_block(test_page, start, size);
}

queue_t* testmode_paging_get_area(void)
{
    return test_page;
}
#endif