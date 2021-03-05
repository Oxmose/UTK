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

/** @brief Free kernel pages map storage linked list. */
static queue_t* free_kernel_pages;

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
        kernel_panic(error);
    }
    free_memory_map = queue_create_queue(QUEUE_ALLOCATOR(kmalloc, kfree), 
                                         &error);
    if(error != OS_NO_ERR)
    {
        KERNEL_ERROR("Could not allocate free memory map queue\n");
        kernel_panic(error);
    }
    
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
        if(error != OS_NO_ERR)
        {
            KERNEL_ERROR("Could not allocate memory range node\n");
            kernel_panic(error);
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
                kernel_panic(OS_ERR_MALLOC);
            }
            node2 = queue_create_node(mem_range2, 
                                      QUEUE_ALLOCATOR(kmalloc, kfree), 
                                      &error);
            if(error != OS_NO_ERR)
            {
                KERNEL_ERROR("Could not allocate memory range node\n");
                kernel_panic(error);
            }
            mem_range2->base  = (uintptr_t)mmap->addr;
            mem_range2->limit = (uintptr_t)mmap->addr + (uintptr_t)mmap->len;
            mem_range2->type  = mmap->type;

            error = queue_push_prio(node2, free_memory_map, mem_range2->base);
            if(error != OS_NO_ERR)
            {
                KERNEL_ERROR("Could not enqueue memory range node\n");
                kernel_panic(error);
            }
            available_memory += (uintptr_t)mmap->len;
        }

        error = queue_push_prio(node, hw_memory_map, mem_range->base);
        if(error != OS_NO_ERR)
        {
            KERNEL_ERROR("Could not enqueue memory range node\n");
            kernel_panic(error);
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
            if(mem_range->base > free_mem_head || mem_range->limit < free_mem_head)
            {
                KERNEL_ERROR("Kernel was not loaded in the first available"
                             " memory region");
                kernel_panic(OS_ERR_OUT_OF_BOUND);
            }   
            break;
        }
        cursor = cursor->prev;
    }
    if(cursor == NULL)
    {
        KERNEL_ERROR("Kernel was not loaded in the first available"
                     " memory region");
        kernel_panic(OS_ERR_OUT_OF_BOUND);
    }

    /* Remove static kernel size from first region */
    mem_range = (mem_range_t*)free_memory_map->head->data;
    mem_range->base = free_mem_head;
    if(mem_range->base > mem_range->limit)
    {
        KERNEL_ERROR("Kernel was loaded on different regions\n");
        kernel_panic(OS_ERR_UNAUTHORIZED_ACTION);
    } 

    /* Initialize kernel pages */
    free_kernel_pages = queue_create_queue(QUEUE_ALLOCATOR(kmalloc, kfree), 
                                           &error);
    if(error != OS_NO_ERR)
    {
        KERNEL_ERROR("Could not initialize free kernel pages queue\n");
        kernel_panic(error);
    }
    mem_range = kmalloc(sizeof(mem_range_t));
    if(mem_range == NULL)
    {
        KERNEL_ERROR("Could not allocate kernel page range structure\n");
        kernel_panic(OS_ERR_MALLOC);
    }
    node = queue_create_node(mem_range, 
                                QUEUE_ALLOCATOR(kmalloc, kfree), 
                                &error);
    if(error != OS_NO_ERR)
    {
        KERNEL_ERROR("Could not initialize free kernel pages node\n");
        kernel_panic(error);
    }
    mem_range->base  = free_mem_head + KERNEL_MEM_OFFSET;
    mem_range->limit = (uintptr_t)KERNEL_VIRTUAL_ADDR_MAX;
    mem_range->type  = MULTIBOOT_MEMORY_AVAILABLE;

    error = queue_push_prio(node, free_kernel_pages, free_mem_head);
    if(error != OS_NO_ERR)
    {
        KERNEL_ERROR("Could not enqueue free kernel pages node\n");
        kernel_panic(error);
    }

    /* Update free memory */
    KERNEL_DEBUG("Kernel physical memory end: 0x%p\n", free_mem_head);
    KERNEL_DEBUG("Kernel virtual memory end: 0x%p\n", 
                 free_mem_head + KERNEL_MEM_OFFSET);

    available_memory -= free_mem_head - KERNEL_MEM_START;
}

static void* get_block(queue_t* list, const size_t length, OS_RETURN_E* err)
{
    queue_node_t* cursor;
    queue_node_t* selected;
    mem_range_t*  range;

    uintptr_t   address;
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
    if(selected == NULL)
    {
        if(err != NULL)
        {
            *err = OS_ERR_NO_MORE_FREE_MEM;
        }
        return NULL;
    }

    /* Save the block address */
    address = range->base;

    /* Modify the block */
    range->base += length * KERNEL_FRAME_SIZE;

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

static OS_RETURN_E add_block(queue_t* list, 
                             uintptr_t first_frame, 
                             const size_t length)
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
        return OS_ERR_NULL_POINTER;
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
            /* Here we free memory that is already free */
            return OS_ERR_UNAUTHORIZED_ACTION;
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
            KERNEL_ERROR("Could not crate node data in memory manager\n");
            return err;
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
            return err;
        }

        queue_push_prio(new_node, list, first_frame);        
    }
    
    return OS_NO_ERR;
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
    return OS_NO_ERR;
}

void* alloc_kframes(const size_t frame_count, OS_RETURN_E* err)
{
    uint32_t int_state;
    void*    address;

    ENTER_CRITICAL(int_state);

    address = get_block(free_memory_map, frame_count, err);

    KERNEL_DEBUG("Allocated %u frames, at 0x%p\n", frame_count, address);

    available_memory -= KERNEL_FRAME_SIZE * frame_count;

    EXIT_CRITICAL(int_state);
    return (void*)address;
}

OS_RETURN_E free_kframes(void* frame_addr, const size_t frame_count)
{
    OS_RETURN_E   err;
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
        /* Trying to free a frame that does not exist of is hardware */
        EXIT_CRITICAL(int_state);
        return OS_ERR_UNAUTHORIZED_ACTION;
    }

    err = add_block(free_memory_map, (uintptr_t)frame_addr, frame_count);

    KERNEL_DEBUG("Deallocated %u frames, at 0x%p\n", frame_count, frame_addr);

    EXIT_CRITICAL(int_state);
    return err;
}

void* alloc_kpages(const size_t page_count, OS_RETURN_E* err)
{
    void*    address;
    uint32_t int_state;

    ENTER_CRITICAL(int_state);

    address = get_block(free_kernel_pages, page_count, err);

    KERNEL_DEBUG("Allocated %u pages, at 0x%p \n", page_count, address);

    EXIT_CRITICAL(int_state);
    return (void*)address;
}

OS_RETURN_E free_kpages(void* page_addr, const size_t page_count)
{
    OS_RETURN_E err;
    uint32_t    int_state;

    if((uintptr_t)page_addr < KERNEL_MEM_OFFSET)
    {
        return OS_ERR_UNAUTHORIZED_ACTION;
    }
#ifdef ARCH_64_BITS
    if((uintptr_t)page_addr & KERNEL_VIRTUAL_ADDR_MAX_MASK)  > 
        KERNEL_VIRTUAL_ADDR_MAX)
    {
        return OS_ERR_UNAUTHORIZED_ACTION;
    }
#endif 

    ENTER_CRITICAL(int_state);

    err = add_block(free_kernel_pages, (uintptr_t)page_addr, page_count);

    KERNEL_DEBUG("Deallocated %u pages, at 0x%p \n", page_count, page_addr);

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
            kernel_panic(error);
        }
    }
    add_block(test_page, start, size);
}

queue_t* testmode_paging_get_area(void)
{
    return test_page;
}
#endif