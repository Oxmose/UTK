/*******************************************************************************
 * @file memalloc.c
 *
 * @see memalloc.h
 *
 * @author Alexy Torres Aurora Dugo
 *
 * @date 20/10/2018
 *
 * @version 1.0
 *
 * @brief Kernel memory physical frame and virtual page allocator.
 *
 * @details Kernel memory physical frame and virtual page allocator. This module
 * allows to allocate and deallocate physical memory frame and virtual pages.
 *
 * @copyright Alexy Torres Aurora Dugo
 ******************************************************************************/

#include <lib/stddef.h>         /* Standard definitions */
#include <lib/stdint.h>         /* Generic int types */
#include <memory/meminfo.h>     /* Memory information */
#include <memory/kheap.h>       /* Kernel heap */
#include <arch_paging.h>        /* Paging information */
#include <io/kernel_output.h>   /* Kernel output methods */
#include <sync/critical.h>      /* Critical sections */

/* UTK configuration file */
#include <config.h>

/* Tests header file */
#if TEST_MODE_ENABLED
#include <Tests/test_bank.h>
#endif

/* Header file */
#include <memory/memalloc.h>

/*******************************************************************************
 * GLOBAL VARIABLES
 ******************************************************************************/

/** @brief Kernel free frame pool. */
mem_area_t* kernel_free_frames;

/** @brief Kernel free page pool. */
mem_area_t* kernel_free_pages;

/** @brief Memory map structure's size. */
extern uint32_t    memory_map_size;

/** @brief Memory map storage as an array of range. */
extern mem_range_t memory_map_data[];

/** @brief Kernel start address. */
extern uint8_t     _kernel_start_phys;

/** @brief Kernel end address. */
extern uint8_t     _kernel_end;

#if MAX_CPU_COUNT > 1
/** @brief Critical section spinlock. */
static spinlock_t lock = SPINLOCK_INIT_VALUE;
#endif

/*******************************************************************************
 * FUNCTIONS
 ******************************************************************************/

static OS_RETURN_E add_free(const uintptr_t start, const size_t size,
                            mem_area_t** list)
{
    mem_area_t* cursor;
    mem_area_t* save;
    mem_area_t* new_node;

    if(list == NULL)
    {
        return OS_ERR_NULL_POINTER;
    }

    if(*list == NULL)
    {
        /* Create first node */
        *list = kmalloc(sizeof(mem_area_t));

        if(list == NULL)
        {
            return OS_ERR_MALLOC;
        }

        (*list)->start = start;
        (*list)->size  = size;

        (*list)->next = NULL;
        (*list)->prev = NULL;

        return OS_NO_ERR;
    }

    /* Search and link new node */
    cursor = *list;
    save = cursor;
    while(cursor != NULL)
    {
        if(cursor->start > start)
        {
            break;
        }
        save = cursor;
        cursor = cursor->next;
    }

    /* Check boundaries */
    if(save != cursor && save->start + save->size > start)
    {
        return OS_ERR_UNAUTHORIZED_ACTION;
    }
    if(save != cursor && cursor != NULL)
    {
        if(start + size > cursor->start)
        {
            return OS_ERR_UNAUTHORIZED_ACTION;
        }
    }

    /* End of list */
    if(cursor == NULL)
    {
        /* Try to merge */
        if(save->start + save->size != start)
        {
            new_node = kmalloc(sizeof(mem_area_t));
            if(new_node == NULL)
            {
                return OS_ERR_MALLOC;
            }
            new_node->size  = size;
            new_node->start = start;
            new_node->next = NULL;
            new_node->prev = save;
            save->next     = new_node;
        }
        else
        {
            save->size += size;
        }
    }
    else if(cursor == save)
    {
        /* Add at the begining */
        if(start + size == cursor->start)
        {
            cursor->size += size;
            cursor->start = start;
        }
        else
        {
            new_node = kmalloc(sizeof(mem_area_t));
            if(new_node == NULL)
            {
                return OS_ERR_MALLOC;
            }
            new_node->size  = size;
            new_node->start = start;
            new_node->next = cursor;
            new_node->prev = NULL;
            cursor->prev     = new_node;
            *list = new_node;
        }
    }
    else
    {
        /* Try to merge */
        if(save->start + save->size == start)
        {
            if(start + size == cursor->start)
            {
                save->size += size + cursor->size;
                save->next = cursor->next;

                kfree(cursor);
            }
            else
            {
                save->size += size;
            }
        }
        else if(start + size == cursor->start)
        {
            cursor->size += size;
            cursor->start = start;
        }
        else
        {
            new_node = kmalloc(sizeof(mem_area_t));
            if(new_node == NULL)
            {
                return OS_ERR_MALLOC;
            }
            new_node->size  = size;
            new_node->start = start;
            new_node->next = cursor;
            new_node->prev = save;
            save->next     = new_node;
            cursor->prev   = new_node;
        }
    }
    return OS_NO_ERR;
}

static void remove_free(mem_area_t* node, mem_area_t** list)
{
    if(node->next == NULL)
    {
        if(node->prev == NULL)
        {
            kfree(node);
            *list = NULL;
        }
        else
        {
            node->prev->next = NULL;
            kfree(node);
        }
    }
    else if(node->prev == NULL)
    {
        node->next->prev = NULL;
        *list = node->next;
        kfree(node);
    }
    else
    {
        node->next->prev = node->prev;
        node->prev->next = node->next;
        kfree(node);
    }
}

static void* get_block(mem_area_t** list, const size_t block_count,
                       OS_RETURN_E* err)
{
    mem_area_t* cursor;
    mem_area_t* selected;
    uintptr_t   address;
    /* Search for the next block with this size */
    cursor = *list;
    selected = NULL;
    while(cursor)
    {
        if(cursor->size >= KERNEL_PAGE_SIZE * block_count)
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
    address = selected->start;

    /* Modify the block */
    selected->size -= KERNEL_PAGE_SIZE * block_count;
    selected->start += KERNEL_PAGE_SIZE * block_count;

    if(selected->size == 0)
    {
        remove_free(selected, list);
    }

    if(err != NULL)
    {
        *err = OS_NO_ERR;
    }
    return (void*)address;
}

OS_RETURN_E memalloc_init(void)
{
    uint32_t    i;
    uintptr_t   start;
    uintptr_t   next_limit;
    uintptr_t   next_start;
    OS_RETURN_E err;

    kernel_free_frames = NULL;
    kernel_free_pages  = NULL;

    next_start = 0;

    /* Init the free memory linked list */
    for(i = 0; i < memory_map_size; ++i)
    {
        /* Check if free */
        if(memory_map_data[i].type == 1 &&
           memory_map_data[i].limit >
            (uintptr_t)&_kernel_end - KERNEL_MEM_OFFSET)
        {
            start = MAX((uintptr_t)&_kernel_end - KERNEL_MEM_OFFSET,
                        memory_map_data[i].base);
            err = add_free(start,
                           memory_map_data[i].limit - start,
                           &kernel_free_frames);
            if(err != OS_NO_ERR)
            {
                return err;
            }

#if MEMORY_KERNEL_DEBUG == 1
            kernel_serial_debug("Added free frame area 0x%p -> 0x%p (%uMB)\n",
                                start, memory_map_data[i].limit, (memory_map_data[i].limit - start) >> 20);
#endif
        }
    }

    /* Map pages, peripherals are not available */
    start = (uintptr_t)&_kernel_start_phys;
    while(start)
    {
        /* Search for next limit */
        for(i = 0; i < memory_map_size; ++i)
        {
            /* Check if free */
            if(memory_map_data[i].type != 1 &&
               memory_map_data[i].base > start)
            {
                next_limit = memory_map_data[i].base;
                break;
            }
        }

        if(i == memory_map_size)
        {
            next_limit = ARCH_MAX_ADDRESS;
        }

        /* Search for next start */
        for(i = 0; i < memory_map_size; ++i)
        {
            /* Check if free */
            if(memory_map_data[i].type == 1 &&
               memory_map_data[i].base > next_limit)
            {
                next_start = memory_map_data[i].base;
                break;
            }
        }

        err = add_free(start, next_limit - start, &kernel_free_pages);        
        if(err != OS_NO_ERR)
        {
            return err;
        }
#if MEMORY_KERNEL_DEBUG == 1
        kernel_serial_debug("Added free page area 0x%p -> 0x%p (%uMB)\n",
                            start, next_limit, (next_limit - start) >> 20);
#endif

#if MEMORY_KERNEL_DEBUG == 1
        kernel_serial_debug("Next free page range: 0x%p\n",
                            start);
#endif

        if(next_limit == ARCH_MAX_ADDRESS)
        {
            start = 0;
        }
        else 
        {
            start = next_start;
        }
    }

#if TEST_MODE_ENABLED
    memalloc_test();
#endif

    return OS_NO_ERR;
}

void* memalloc_alloc_kframes(const size_t frame_count, OS_RETURN_E* err)
{
    uint32_t int_state;
    void*    address;

#if MAX_CPU_COUNT > 1
    ENTER_CRITICAL(int_state, &lock);
#else
    ENTER_CRITICAL(int_state);
#endif

    address = get_block(&kernel_free_frames, frame_count, err);

#if MEMORY_KERNEL_DEBUG == 1
    kernel_serial_debug("Allocated %u frames, at 0x%p \n", frame_count, address);
#endif

#if MAX_CPU_COUNT > 1
    EXIT_CRITICAL(int_state, &lock);
#else
    EXIT_CRITICAL(int_state);
#endif

    return (void*)address;
}

OS_RETURN_E memalloc_free_kframes(void* frame_addr, const size_t frame_count)
{
    OS_RETURN_E err;
    uint32_t    int_state;

#if MAX_CPU_COUNT > 1
    ENTER_CRITICAL(int_state, &lock);
#else
    ENTER_CRITICAL(int_state);
#endif

    err = add_free((uintptr_t)frame_addr, frame_count * KERNEL_PAGE_SIZE,
                    &kernel_free_frames);

#if MEMORY_KERNEL_DEBUG == 1
    kernel_serial_debug("Deallocated %u frames, at 0x%p \n", frame_count, frame_addr);
#endif

#if MAX_CPU_COUNT > 1
    EXIT_CRITICAL(int_state, &lock);
#else
    EXIT_CRITICAL(int_state);
#endif

    return err;
}

void* memalloc_alloc_kpages(const size_t page_count, OS_RETURN_E* err)
{
    void*    address;
    uint32_t int_state;

#if MAX_CPU_COUNT > 1
    ENTER_CRITICAL(int_state, &lock);
#else
    ENTER_CRITICAL(int_state);
#endif

    address = get_block(&kernel_free_pages, page_count, err);

#if MEMORY_KERNEL_DEBUG == 1
    kernel_serial_debug("Allocated %u pages, at 0x%p \n", page_count, address);
#endif

#if MAX_CPU_COUNT > 1
    EXIT_CRITICAL(int_state, &lock);
#else
    EXIT_CRITICAL(int_state);
#endif

    return (void*)address;
}

OS_RETURN_E memalloc_free_kpages(void* page_addr, const size_t page_count)
{
    OS_RETURN_E err;
    uint32_t    int_state;

    if((uintptr_t)page_addr < KERNEL_MEM_OFFSET + (uintptr_t)&_kernel_end)
    {
        return OS_ERR_UNAUTHORIZED_ACTION;
    }

#if MAX_CPU_COUNT > 1
    ENTER_CRITICAL(int_state, &lock);
#else
    ENTER_CRITICAL(int_state);
#endif

    err = add_free((uintptr_t)page_addr, page_count * KERNEL_PAGE_SIZE,
                   &kernel_free_pages);

#if MEMORY_KERNEL_DEBUG == 1
    kernel_serial_debug("Deallocated %u pages, at 0x%p \n", page_count, page_addr);
#endif

#if MAX_CPU_COUNT > 1
    EXIT_CRITICAL(int_state, &lock);
#else
    EXIT_CRITICAL(int_state);
#endif

    return err;
}


/* Test Mode */
#if TEST_MODE_ENABLED == 1
const mem_area_t* paging_get_free_frames(void)
{
    return kernel_free_frames;
}

const mem_area_t* paging_get_free_pages(void)
{
    return kernel_free_pages;
}


mem_area_t* test_page;
void testmode_paging_add_page(uintptr_t start, size_t size)
{
    add_free(start, size, &test_page);
}
mem_area_t* testmode_paging_get_area(void)
{
    return test_page;
}
#endif