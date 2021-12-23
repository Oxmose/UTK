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
#include <kqueue.h>               /* Kernel queue structures */
#include <kheap.h>                /* Kernel heap allocator */
#include <scheduler.h>            /* Scheduler */
#include <arch_memmgt.h>          /* Paging information */
#include <exceptions.h>           /* Exception management */
#include <interrupt_settings.h>   /* Interrupt settings */
#include <critical.h>             /* Critical sections */
#include <cpu.h>                  /* CPU management */
#include <ctrl_block.h>           /* Kernel process structure */
#include <sys/syscall_api.h>      /* System call API */

/* UTK configuration file */
#include <config.h>

#include <test_bank.h>

/* Header file */
#include <memmgt.h>

/*******************************************************************************
 * CONSTANTS
 ******************************************************************************/

/* None */

/*******************************************************************************
 * STRUCTURES AND TYPES
 ******************************************************************************/

/** @brief Defines a memory range with its type as defined by the multiboot
 * standard.
 */
struct mem_range
{
    /** @brief Range's base address. */
    uintptr_t base;

    /** @brief Range's limit. */
    uintptr_t limit;

    /** @brief Range's memory type. */
    uint32_t type;
};

/**
 * @brief Defines mem_range_t type as a shorcut for struct mem_range.
 */
typedef struct mem_range mem_range_t;

/** @brief Defines the memory allocation starting point (begining or end of the
 * memory space).
 */
enum MEM_ALLOC_START
{
    MEM_ALLOC_BEGINING,
    MEM_ALLOC_END
};

/**
 * @brief Defines MEM_ALLOCATION_START_E type as a shorcut for
 * enum MEM_ALLOCATION_START.
 */
typedef enum MEM_ALLOC_START MEM_ALLOC_START_E;

/** @brief Defines the structure used when copying the memory image of a
 * process.
 */
struct mem_copy_self_data
{
    /** @brief The PGDir frame address */
    uintptr_t*  new_pgdir_frame;
    /** @brief The PGDir page address */
    uintptr_t*  new_pgdir_page;
    /** @brief The pagetable page address */
    uintptr_t*  new_pgtable_page;
    /** @brief The current data page */
    uintptr_t*  new_data_page;

    /** @brief Tells if the PGDir was mapped. */
    bool_t      mapped_pgdir;
    /** @brief Tells if the PGDir frame was acquired. */
    bool_t      acquired_ref_pgdir;
};

/**
 * @brief Defines mem_copy_self_data_t type as a shorcut for
 * struct mem_copy_self_data.
 */
typedef struct mem_copy_self_data mem_copy_self_data_t;

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
/** @brief Kernel symbols mapping: Symbol table address start. */
extern uint8_t _KERNEL_SYMTAB_REG_BASE;
/** @brief Kernel symbols mapping: Symbol table region size. */
extern uint8_t _KERNEL_SYMTAB_REG_SIZE;
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
/** @brief Kernel multiboot structures memory address. */
extern uint32_t _KERNEL_MULTIBOOT_MEM_BASE;
/** @brief Kernel multiboot structures memory size. */
extern uint8_t _KERNEL_MULTIBOOT_MEM_SIZE;
/** @brief Kernel init ram disk memory address. */
extern uint8_t _KERNEL_INITRD_MEM_BASE;
/** @brief Kernel init ram disk memory size. */
extern uint8_t _KERNEL_INITRD_MEM_SIZE;
/** @brief Kernel memory end address. */
extern uint8_t _KERNEL_MEMORY_END;
/** @brief Kernel recursive mapping address for page tables */
extern uint32_t _KERNEL_RECUR_PG_TABLE_BASE;
/** @brief Kernel recursive mapping address for page directory */
extern uint8_t *_KERNEL_RECUR_PG_DIR_BASE;

/** @brief Hardware memory map storage linked list. */
static kqueue_t* hw_memory_map;

/** @brief Free memory map storage linked list. */
static kqueue_t* free_memory_map;

/** @brief Free kernel pages map storage linked list. */
static kqueue_t* free_kernel_pages;

/** @brief Stores the total available memory */
static uintptr_t available_memory;

/** @brief Kernel page directory array. */
static uintptr_t kernel_pgdir[KERNEL_PGDIR_SIZE] __attribute__((aligned(4096)));

/** @brief Kernel reserved page tables. */
static uintptr_t min_pgtable[KERNEL_RESERVED_PAGING][KERNEL_PGDIR_SIZE]
                                                __attribute__((aligned(4096)));

/** @brief Stores the frame reference table directory */
static uintptr_t frame_ref_dir[FRAME_REF_DIR_SIZE];


/*******************************************************************************
 * STATIC FUNCTIONS DECLARATIONS
 ******************************************************************************/
/**
 * @brief Kernel memory frame allocation.
 *
 * @details Kernel memory frame allocation. This method gets the desired number
 * of contiguous frames from the kernel frame pool and allocate them.
 *
 * @param[in] frame_count The number of desired frames to allocate.
 * @param[out] err The error buffer to store the operation's result. If NULL,
 * the function will raise a kernel panic in case of error.
 *
 * @return The address of the first frame of the contiguous block is
 * returned.
 */
static void* memory_alloc_frames(const size_t frame_count, OS_RETURN_E* err);

/**
 * @brief Kernel memory frame release.
 *
 * @details Kernel memory frame release. This method releases the desired number
 * of contiguous frames to the kernel frame pool.
 *
 * @param[in] frame_addr The address of the first frame to release.
 * @param[in] frame_count The number of desired frames to release.
 * @param[out] err The error buffer to store the operation's result. If NULL,
 * the function will raise a kernel panic in case of error.
 */
static void memory_free_frames(void* frame_addr,
                               const size_t frame_count,
                               OS_RETURN_E* err);

/**
 * @brief Kernel memory page allocation.
 *
 * @details Kernel memory page allocation. This method gets the desired number
 * of contiguous pages from the current process page pool and allocate them.
 *
 * @param[in, out] page_table The free page table to allocate pages from.
 * @param[in] page_count The number of desired pages to allocate.
 * @param[in] start_pt The starting point to allocated memory in the space.
 * @param[out] err The error buffer to store the operation's result. If NULL,
 * the function will raise a kernel panic in case of error.
 *
 * @return The address of the first page of the contiguous block is
 * returned.
 */
static void* memory_alloc_pages_from(kqueue_t* page_table,
                                     const size_t page_count,
                                     const MEM_ALLOC_START_E start_pt,
                                     OS_RETURN_E* err);

/**
 * @brief Kernel memory page release.
 *
 * @details Kernel memory page release. This method releases the desired number
 * of contiguous pages to the current process page pool.
 *
 * @param[in, out] page_table The free page table to free pages to.
 * @param[in] page_addr The address of the first page to release.
 * @param[in] page_count The number of desired pages to release.
 * @param[out] err The error buffer to store the operation's result. If NULL,
 * the function will raise a kernel panic in case of error.
 */
static void memory_free_pages_to(kqueue_t* page_table,
                                 const uintptr_t* page_addr,
                                 const size_t page_count,
                                 OS_RETURN_E* err);

static void memory_acquire_ref(uintptr_t phys_addr);

static void memory_release_ref(uintptr_t phys_addr);

static uint32_t memory_get_ref_count(const uintptr_t phys_addr);

static void memory_set_ref_count(const uintptr_t phys_addr,
                                 const uint32_t count);

static void init_frame_ref_table(uintptr_t next_free_mem);

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
static void memory_get_khighstartup_range(uintptr_t* start, uintptr_t* end);

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
static void memory_get_ktext_range(uintptr_t* start, uintptr_t* end);

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
static void memory_get_krodata_range(uintptr_t* start, uintptr_t* end);

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
static void memory_get_kdata_range(uintptr_t* start, uintptr_t* end);

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
static void memory_get_kbss_range(uintptr_t* start, uintptr_t* end);

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
static void memory_get_kstacks_range(uintptr_t* start, uintptr_t* end);

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
static void memory_get_kheap_range(uintptr_t* start, uintptr_t* end);

/**
 * @brief Retrieves the start and end address of the kernel multiboot section.
 *
 * @details Retrieves the start and end address of the kernel multiboot section.
 * The addresses are stored in the buffer given as parameter. The function has
 * no effect if the buffer are NULL.
 *
 * @param[out] start The start address of the section.
 * @param[out] end The end address of the section.
 */
static void memory_get_multiboot_range(uintptr_t* start, uintptr_t* end);

/**
 * @brief Retrieves the start and end address of the kernel initrd section.
 *
 * @details Retrieves the start and end address of the kernel initrd section.
 * The addresses are stored in the buffer given as parameter. The function has
 * no effect if the buffer are NULL.
 *
 * @param[out] start The start address of the section.
 * @param[out] end The end address of the section.
 */
static void memory_get_initrd_range(uintptr_t* start, uintptr_t* end);

/**
 * @brief Retrieves the start and end address of the kernel symbol section.
 *
 * @details Retrieves the start and end address of the kernel symbol section.
 * The addresses are stored in the buffer given as parameter. The function has
 * no effect if the buffer are NULL.
 *
 * @param[out] start The start address of the section.
 * @param[out] end The end address of the section.
 */
static void memory_get_symtab_range(uintptr_t* start, uintptr_t* end);

static void print_kernel_map(void);

static void detect_memory(void);

static void setup_mem_table(void);

static void* get_block(kqueue_t* list,
                       const size_t length,
                       const MEM_ALLOC_START_E start_pt,
                       OS_RETURN_E* err);

static void add_block(kqueue_t* list,
                      uintptr_t first_frame,
                      const size_t length,
                      OS_RETURN_E* err);

/**
 * @brief Maps a kernel section to the memory.
 *
 * @details Maps a kernel section to the memory. No frame are allocated as the
 * memory should already be populated.
 *
 * @param[in] start_addr The start address of the section.
 * @param[in] end_addr The end address of the section.
 * @param[in] read_only Set to TRUE if the section is read only.
 */
static void map_kernel_section(uintptr_t start_addr,
                               uintptr_t end_addr,
                               const bool_t read_only);

static OS_RETURN_E memory_invocate_cow(const uintptr_t addr);

/**
 * @brief Handle a page fault exception.
 *
 * @details Handle a page fault exception raised by the cpu. The corresponding
 * registered handler will be called. If no handler is available, a panic is
 * raised.
 *
 * @param[in] cpu_state The cpu registers structure.
 * @param[in] int_id The exception number.
 * @param[in] stack_state The stack state before the exception that contain cs,
 * eip, error code and the eflags register value.
 */
static void paging_fault_general_handler(cpu_state_t* cpu_state,
                                         uintptr_t int_id,
                                         stack_state_t* stack_state);

/**
 * @brief Tells if a memory region is already mapped in the current page tables.
 *
 * @details Tells if a memory region is already mapped in the current page
 * tables. Returns 0 if the region is not mapped, 1 otherwise.
 *
 * @return Returns 0 if the region is not mapped, 1 otherwise.
 */
static bool_t is_mapped(const uintptr_t start_addr, const size_t size);

/**
 * @brief Copies the free page table of the current process and return the copy.
 *
 * @details Copies the free page table of the current process and return the
 * copy. This function performs a deep copy of the table. Meaning that the two
 * instance to the table are totally independant.
 *
 * @return A deep copy of the current process free page table is returned.
 */
static kqueue_t* paging_copy_free_page_table(void);

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
 * @param[in] flags The flags used for mapping (eg hw, cow, etc).
 * @param[out] err The error buffer to store the operation's result. If NULL,
 * the function will raise a kernel panic in case of error.
 */
static void kernel_mmap_internal(const void* virt_addr,
                                 const void* phys_addr,
                                 const size_t mapping_size,
                                 const uintptr_t flags,
                                 OS_RETURN_E* err);

/**
 * @brief Initializes paging structures for the kernel.
 *
 * @details Initializes paging structures for the kernel. This function will
 * select an available memory region to allocate the memory required for the
 * kernel. Then the kernel will be mapped to memory and paging is enabled for
 * the kernel.
 */
static void paging_init(void);

static void memory_paging_enable(void);

/* TODO Document */
static OS_RETURN_E memory_copy_self_stack(mem_copy_self_data_t* data,
                                          const void* kstack_addr,
                                          const size_t kstack_size);
static OS_RETURN_E memory_copy_self_pgtable(mem_copy_self_data_t* data);
static OS_RETURN_E memory_create_new_pagedir(mem_copy_self_data_t* data);
static OS_RETURN_E memory_copy_self_clean(mem_copy_self_data_t* data,
                                          OS_RETURN_E past_err);

static uint32_t get_free_mem(kqueue_t* mem_pool);

/*******************************************************************************
 * FUNCTIONS
 ******************************************************************************/

#define MEMMGT_ASSERT(COND, MSG, ERROR) {                   \
    if((COND) == FALSE)                                     \
    {                                                       \
        PANIC(ERROR, "MEMMGT", MSG, TRUE);                  \
    }                                                       \
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

static void* memory_alloc_frames(const size_t frame_count, OS_RETURN_E* err)
{
    uint32_t    int_state;
    void*       address;
    OS_RETURN_E internal_err;

    ENTER_CRITICAL(int_state);

    address = get_block(free_memory_map,
                        frame_count,
                        MEM_ALLOC_BEGINING,
                        &internal_err);
    if(internal_err != OS_NO_ERR)
    {
        KERNEL_ERROR("Could not allocate new frame\n");
        if(err == NULL)
        {
            MEMMGT_ASSERT(FALSE, "Could not allocate new frame", internal_err);
        }
        else
        {
            *err = internal_err;
            EXIT_CRITICAL(int_state);
            return NULL;
        }
    }

    KERNEL_DEBUG(MEMMGT_DEBUG_ENABLED,
                 "[MEMMGT] Allocated %u frames, at 0x%p",
                 frame_count, address);

    available_memory -= KERNEL_FRAME_SIZE * frame_count;

    EXIT_CRITICAL(int_state);

    if(err != NULL)
    {
        *err = OS_NO_ERR;
    }

    return address;
}

static uint32_t get_free_mem(kqueue_t* mem_pool)
{
    kqueue_node_t* head;
    mem_range_t*  range;
    uint32_t      total;

    total = 0;

    head = mem_pool->head;
    while(head != NULL)
    {
        range = (mem_range_t*)head->data;
        total += range->limit - range->base;
        head = head->next;
    }

    return total;
}


static void memory_free_frames(void* frame_addr,
                               const size_t frame_count,
                               OS_RETURN_E* err)
{
    uint32_t      int_state;
    kqueue_node_t* cursor;
    mem_range_t*  mem_range;
    OS_RETURN_E   internal_err;
    size_t        i;

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

        if(err == NULL)
        {
            MEMMGT_ASSERT(FALSE,
                          "Tried to free non existent frame",
                          OS_ERR_UNAUTHORIZED_ACTION);
        }
        else
        {
            *err = OS_ERR_UNAUTHORIZED_ACTION;
            EXIT_CRITICAL(int_state);
            return;
        }
    }

    add_block(free_memory_map,
              (uintptr_t)frame_addr,
              frame_count,
              &internal_err);

    if(internal_err != OS_NO_ERR)
    {
        KERNEL_ERROR("Could not free frame\n");
        if(err == NULL)
        {
            MEMMGT_ASSERT(FALSE,
                          "Could not free frame",
                          internal_err);
        }
        else
        {
            *err = internal_err;
            EXIT_CRITICAL(int_state);
            return;
        }
    }

    /* Set ref count to 0 */
    for(i = 0; i < frame_count; ++i)
    {
        memory_set_ref_count((uintptr_t)frame_addr, 0);
    }

    KERNEL_DEBUG(MEMMGT_DEBUG_ENABLED,
                 "[MEMMGT] Deallocated %u frames, at 0x%p",
                 frame_count, frame_addr);

    available_memory += KERNEL_FRAME_SIZE * frame_count;

    if(err != NULL)
    {
        *err = OS_NO_ERR;
    }

    EXIT_CRITICAL(int_state);
}

static void* memory_alloc_pages_from(kqueue_t* page_table,
                                     const size_t page_count,
                                     const MEM_ALLOC_START_E start_pt,
                                     OS_RETURN_E* err)
{
    void*             address;
    uint32_t          int_state;
    OS_RETURN_E       internal_err;

    ENTER_CRITICAL(int_state);

    address = get_block(page_table, page_count, start_pt, &internal_err);
    if(internal_err != OS_NO_ERR)
    {
        KERNEL_ERROR("Could not allocate new page\n");
        if(err == NULL)
        {
            MEMMGT_ASSERT(FALSE,
                          "Could not allocate new page",
                          internal_err);
        }
        else
        {
            *err = internal_err;
            EXIT_CRITICAL(int_state);
            return NULL;
        }
    }

    KERNEL_DEBUG(MEMMGT_DEBUG_ENABLED,
                 "[MEMMGT] Allocated %u pages, at 0x%p",
                 page_count,
                 address);

    EXIT_CRITICAL(int_state);

    if(err != NULL)
    {
        *err = OS_NO_ERR;
    }
    return address;
}

static void memory_free_pages_to(kqueue_t* page_table,
                                 const uintptr_t* page_addr,
                                 const size_t page_count,
                                 OS_RETURN_E* err)
{
    uint32_t    int_state;
    OS_RETURN_E internal_err;

    ENTER_CRITICAL(int_state);

    add_block(page_table, (uintptr_t)page_addr, page_count, &internal_err);

     if(internal_err != OS_NO_ERR)
    {
        KERNEL_ERROR("Could not free page\n");
        if(err == NULL)
        {
            MEMMGT_ASSERT(FALSE,
                          "Could not free page",
                          internal_err);
        }
        else
        {
            *err = internal_err;
            EXIT_CRITICAL(int_state);
            return;
        }
    }

    KERNEL_DEBUG(MEMMGT_DEBUG_ENABLED,
                 "[MEMMGT] Deallocated %u pages, at 0x%p",
                 page_count,
                 page_addr);

    EXIT_CRITICAL(int_state);

    if(err != NULL)
    {
        *err = OS_NO_ERR;
    }

}

static void memory_acquire_ref(uintptr_t phys_addr)
{
    uint16_t   dir_entry;
    uint16_t   table_entry;
    uint32_t   int_state;
    uintptr_t* current_table;


    dir_entry   = phys_addr >> FRAME_REF_DIR_ENTRY_OFFSET;
    table_entry = (phys_addr >> FRAME_REF_TABLE_ENTRY_OFFSET) &
                  FRAME_REF_TABLE_ENTRY_OFFSET_MASK;

    ENTER_CRITICAL(int_state);

    current_table = (uintptr_t*)frame_ref_dir[dir_entry];

    MEMMGT_ASSERT((current_table != NULL ||
                   current_table[table_entry] != (uintptr_t)NULL),
                  "Tried to acquire reference on non existing memory",
                  OS_ERR_UNAUTHORIZED_ACTION);

    /* Update reference count */
    ++current_table[table_entry];

    MEMMGT_ASSERT(((current_table[table_entry] & FRAME_REF_COUNT_MASK) !=
                   FRAME_REF_COUNT_MASK),
                   "Exceeded reference count reached",
                   OS_ERR_UNAUTHORIZED_ACTION);

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



    dir_entry   = phys_addr >> FRAME_REF_DIR_ENTRY_OFFSET;
    table_entry = (phys_addr >> FRAME_REF_TABLE_ENTRY_OFFSET) &
                  FRAME_REF_TABLE_ENTRY_OFFSET_MASK;

    ENTER_CRITICAL(int_state);

    current_table = (uintptr_t*)frame_ref_dir[dir_entry];

    MEMMGT_ASSERT(current_table != NULL &&
                  current_table[table_entry] != (uintptr_t)NULL,
                  "Tried to release reference on non existing memory",
                  OS_ERR_UNAUTHORIZED_ACTION);

    MEMMGT_ASSERT((current_table[table_entry] & FRAME_REF_COUNT_MASK) != 0,
                  "Tried to release reference on free memory",
                  OS_ERR_UNAUTHORIZED_ACTION);

    /* Update reference count */
    --current_table[table_entry];

    KERNEL_DEBUG(MEMMGT_DEBUG_ENABLED,
                 "[MEMMGT] Released reference 0x%p",
                 phys_addr);

    /* Check if we should release the frame */
    if((current_table[table_entry] & FRAME_REF_COUNT_MASK) == 0 &&
       (current_table[table_entry] & FRAME_REF_IS_HW) == 0)
    {
        memory_free_frames((void*)phys_addr, 1, NULL);
    }

    EXIT_CRITICAL(int_state);
}

static uint32_t memory_get_ref_count(const uintptr_t phys_addr)
{
    uint16_t   dir_entry;
    uint16_t   table_entry;
    uint32_t   int_state;
    uintptr_t* current_table;
    uint32_t   ref_count;

    dir_entry   = phys_addr >> FRAME_REF_DIR_ENTRY_OFFSET;
    table_entry = (phys_addr >> FRAME_REF_TABLE_ENTRY_OFFSET) &
                  FRAME_REF_TABLE_ENTRY_OFFSET_MASK;

    ENTER_CRITICAL(int_state);

    current_table = (uintptr_t*)frame_ref_dir[dir_entry];

    MEMMGT_ASSERT(current_table != NULL &&
                  current_table[table_entry] != (uintptr_t)NULL,
                  "Tried to release reference on non existing memory",
                  OS_ERR_UNAUTHORIZED_ACTION);

    ref_count = current_table[table_entry] & FRAME_REF_COUNT_MASK;

    EXIT_CRITICAL(int_state);

    return ref_count;
}

static void memory_set_ref_count(const uintptr_t phys_addr,
                                 const uint32_t count)
{
    uint16_t   dir_entry;
    uint16_t   table_entry;
    uint32_t   int_state;
    uintptr_t* current_table;

    dir_entry   = phys_addr >> FRAME_REF_DIR_ENTRY_OFFSET;
    table_entry = (phys_addr >> FRAME_REF_TABLE_ENTRY_OFFSET) &
                  FRAME_REF_TABLE_ENTRY_OFFSET_MASK;

    ENTER_CRITICAL(int_state);

    current_table = (uintptr_t*)frame_ref_dir[dir_entry];

    MEMMGT_ASSERT(current_table != NULL &&
                  current_table[table_entry] != (uintptr_t)NULL,
                  "Tried to release reference on non existing memory",
                  OS_ERR_UNAUTHORIZED_ACTION);

    current_table[table_entry] = (current_table[table_entry] &
                                  ~FRAME_REF_COUNT_MASK) |
                                  (count & FRAME_REF_COUNT_MASK);

    EXIT_CRITICAL(int_state);
}

static void init_frame_ref_table(uintptr_t next_free_mem)
{
    kqueue_node_t* cursor;
    mem_range_t*   mem_range;
    uintptr_t      current_addr;
    uintptr_t      current_limit;
    uintptr_t      flags;
    uint16_t       dir_entry;
    uint16_t       table_entry;
    uintptr_t*     current_table;

    /* Align next free meme to next frame */
    next_free_mem += KERNEL_FRAME_SIZE -
                     (next_free_mem & (KERNEL_FRAME_SIZE - 1));

    memset(frame_ref_dir, 0, FRAME_REF_DIR_SIZE * sizeof(uintptr_t));

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
            KERNEL_INFO("[MEMMGT] Memory manager cannot manage unaligned "
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
                frame_ref_dir[dir_entry] = (uintptr_t)kmalloc(
                                                    FRAME_REF_TABLE_SIZE *
                                                    sizeof(uintptr_t));

                MEMMGT_ASSERT(frame_ref_dir[dir_entry] != (uintptr_t)NULL,
                              "Cannot allocate reference page table",
                              OS_ERR_MALLOC);

                memset((void*)frame_ref_dir[dir_entry],
                       0,
                       FRAME_REF_TABLE_SIZE * sizeof(uintptr_t));
            }

            current_table = (uintptr_t*)frame_ref_dir[dir_entry];

            MEMMGT_ASSERT(current_table[table_entry] == (uintptr_t)NULL,
                          "Reference table cannot have multiple ref",
                          OS_ERR_UNAUTHORIZED_ACTION);

            current_table[table_entry] = flags;

            current_addr += KERNEL_FRAME_SIZE;
        }
        cursor = cursor->next;
    }
}

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

static void memory_get_multiboot_range(uintptr_t* start, uintptr_t* end)
{
    if(start != NULL)
    {
        *start = (uintptr_t)&_KERNEL_MULTIBOOT_MEM_BASE;
    }
    if(end != NULL)
    {
        *end = ((uintptr_t)&_KERNEL_MULTIBOOT_MEM_BASE) +
               ((uintptr_t)&_KERNEL_MULTIBOOT_MEM_SIZE);
    }
}

static void memory_get_symtab_range(uintptr_t* start, uintptr_t* end)
{
    if(start != NULL)
    {
        *start = (uintptr_t)&_KERNEL_SYMTAB_REG_BASE;
    }
    if(end != NULL)
    {
        *end = (uintptr_t)&_KERNEL_SYMTAB_REG_BASE +
               (uintptr_t)&_KERNEL_SYMTAB_REG_SIZE;
    }
}

static void memory_get_initrd_range(uintptr_t* start, uintptr_t* end)
{
    if(start != NULL)
    {
        *start = (uintptr_t)&_KERNEL_INITRD_MEM_BASE;
    }
    if(end != NULL)
    {
        *end = (uintptr_t)&_KERNEL_INITRD_MEM_BASE +
               (uintptr_t)&_KERNEL_INITRD_MEM_SIZE;
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
    KERNEL_INFO("Multiboot       0x%p -> 0x%p | "PRIPTR"KB\n",
                    (uintptr_t)&_KERNEL_MULTIBOOT_MEM_BASE,
                    ((uintptr_t)&_KERNEL_MULTIBOOT_MEM_BASE) +
                    (uintptr_t)&_KERNEL_MULTIBOOT_MEM_SIZE,
                    ((uintptr_t)&_KERNEL_MULTIBOOT_MEM_SIZE) >> 10);
    KERNEL_INFO("INITRD          0x%p -> 0x%p | "PRIPTR"KB\n",
                    &_KERNEL_INITRD_MEM_BASE,
                    &_KERNEL_INITRD_MEM_BASE +
                    (uintptr_t)&_KERNEL_INITRD_MEM_SIZE,
                    ((uintptr_t)&_KERNEL_INITRD_MEM_SIZE) >> 10);
}

static void detect_memory(void)
{
    struct multiboot_tag*   multiboot_tag;
    uint32_t                multiboot_info_size;
    multiboot_memory_map_t* curr_entry;
    mem_range_t*            mem_range;
    mem_range_t*            mem_range2;
    kqueue_node_t*          node;
    kqueue_node_t*          node2;
    uint32_t                entry_size;
    uint32_t                i;

    /* Create memory tables */
    hw_memory_map   = kqueue_create_queue();
    free_memory_map = kqueue_create_queue();

    /* Update address to higher half kernel */
    KERNEL_DEBUG(MEMMGT_DEBUG_ENABLED,
                 "[MEMMGT] Reading memory configuration from 0x%x",
                 (uintptr_t)&_KERNEL_MULTIBOOT_MEM_BASE);

    /* Get multiboot data */
    multiboot_info_size = *(uint32_t*)&_KERNEL_MULTIBOOT_MEM_BASE;
    multiboot_tag = (struct multiboot_tag*)
                    (((uintptr_t)&_KERNEL_MULTIBOOT_MEM_BASE) + 8);

    KERNEL_DEBUG(MEMMGT_DEBUG_ENABLED,
                 "[MEMMGT] Memory configuration size 0x%p",
                 multiboot_info_size);

    /* Search for memory information */
    available_memory = 0;
    while((uintptr_t)multiboot_tag <
          ((uintptr_t)&_KERNEL_MULTIBOOT_MEM_BASE) + multiboot_info_size)
    {
        entry_size = ((multiboot_tag->size + 7) & ~7);
        KERNEL_DEBUG(MEMMGT_DEBUG_ENABLED,
                                 "[MEMMGT] Detection entry S 0x%X, T 0x%X",
                                 entry_size, multiboot_tag->type);
        if(multiboot_tag->type == MULTIBOOT_TAG_TYPE_MMAP)
        {
            /* We found the entries, now parse them */
            for(i = 0;
                i < (entry_size - 16) / sizeof(multiboot_memory_map_t);
                ++i)
            {
                curr_entry = (multiboot_memory_map_t*)
                             (((uintptr_t)multiboot_tag + 16) + i * 24);

                /* Everything over the 4G limit is not registered on 32 bits
                 * systems
                 */
                if(curr_entry->addr > 0xFFFFFFFFULL)
                {
                    KERNEL_DEBUG(MEMMGT_DEBUG_ENABLED,
                                 "[MEMMGT] Detection, skipped region at 0x%llX",
                                 curr_entry->addr);
                    continue;
                }

                KERNEL_DEBUG(MEMMGT_DEBUG_ENABLED,
                                 "[MEMMGT] Detection, register region 0x%llX",
                                 curr_entry->addr);

                mem_range = kmalloc(sizeof(mem_range_t));
                MEMMGT_ASSERT(mem_range != NULL,
                              "Could not allocate memory range structure",
                              OS_ERR_MALLOC);

                node = kqueue_create_node(mem_range);

                mem_range->base  = (uintptr_t)curr_entry->addr;
                mem_range->limit = (uintptr_t)curr_entry->addr +
                                   (uintptr_t)curr_entry->len;
                mem_range->type  = curr_entry->type;

                /* Low memory is treated as HW */
                if(curr_entry->type == MULTIBOOT_MEMORY_AVAILABLE &&
                   curr_entry->addr >= KERNEL_MEM_START)
                {
                    mem_range2 = kmalloc(sizeof(mem_range_t));
                    MEMMGT_ASSERT(mem_range2 != NULL,
                                  "Could not allocate memory range structure",
                                  OS_ERR_MALLOC);

                    node2 = kqueue_create_node(mem_range2);

                    mem_range2->base  = (uintptr_t)curr_entry->addr;
                    mem_range2->limit = (uintptr_t)curr_entry->addr +
                                        (uintptr_t)curr_entry->len;
                    mem_range2->type  = curr_entry->type;

                    kqueue_push_prio(node2, free_memory_map, mem_range2->base);
                    available_memory += (uintptr_t)curr_entry->len;
                }

                kqueue_push_prio(node, hw_memory_map, mem_range->base);
            }

        }
        multiboot_tag = (struct multiboot_tag*)
                        ((uintptr_t)multiboot_tag + entry_size);
    }
}

static void setup_mem_table(void)
{
    uintptr_t      free_mem_head;
    mem_range_t*   mem_range;
    kqueue_node_t* cursor;
    kqueue_node_t* node;

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
            MEMMGT_ASSERT(mem_range->base <= free_mem_head &&
                          mem_range->limit >= free_mem_head,
                          "Kernel was not loaded in the first available region",
                          OS_ERR_OUT_OF_BOUND);
            break;
        }
        cursor = cursor->prev;
    }
    MEMMGT_ASSERT(cursor != NULL,
                  "Kernel was not loaded in the first available region",
                  OS_ERR_OUT_OF_BOUND);

    /* Remove static kernel size from first region */
    mem_range = (mem_range_t*)free_memory_map->head->data;
    mem_range->base = free_mem_head;
    MEMMGT_ASSERT(mem_range->base <= mem_range->limit,
                  "Kernel was loaded on different regions",
                  OS_ERR_UNAUTHORIZED_ACTION);

    /* Initialize the frame reference table */
    init_frame_ref_table(free_mem_head);

    /* Initialize kernel pages */
    free_kernel_pages = kqueue_create_queue();

    mem_range = kmalloc(sizeof(mem_range_t));
    MEMMGT_ASSERT(mem_range != NULL,
                  "Could not allocate kernel page range structure",
                  OS_ERR_MALLOC);

    node = kqueue_create_node(mem_range);

    mem_range->base  = free_mem_head + KERNEL_MEM_OFFSET;
    mem_range->limit = (uintptr_t)&_KERNEL_RECUR_PG_TABLE_BASE;
    mem_range->type  = MULTIBOOT_MEMORY_AVAILABLE;

    kqueue_push_prio(node, free_kernel_pages, free_mem_head);

    /* Update free memory */
    KERNEL_DEBUG(MEMMGT_DEBUG_ENABLED,
                 "[MEMMGT] Kernel physical memory end: 0x%p",
                 free_mem_head);
    KERNEL_DEBUG(MEMMGT_DEBUG_ENABLED,
                 "[MEMMGT] Kernel virtual memory end: 0x%p",
                 free_mem_head + KERNEL_MEM_OFFSET);

    available_memory -= free_mem_head - KERNEL_MEM_START;
}

static void* get_block(kqueue_t* list,
                       const size_t length,
                       const MEM_ALLOC_START_E start_pt,
                       OS_RETURN_E* err)
{
    kqueue_node_t* cursor;
    kqueue_node_t* selected;
    mem_range_t*   range;
    uintptr_t      address;

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
        KERNEL_ERROR("No more free memory\n");

        MEMMGT_ASSERT(err != NULL,
                      "No more free memory",
                      OS_ERR_NO_MORE_FREE_MEM);

        *err = OS_ERR_NO_MORE_FREE_MEM;
        return NULL;
    }

    if(start_pt == MEM_ALLOC_BEGINING)
    {
        /* Save the block address */
        address = range->base;

        /* Modify the block */
        range->base += length * KERNEL_FRAME_SIZE;

        /* Update priority */
        selected->priority = range->base;
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
        kqueue_remove(list, selected, TRUE);
        kqueue_delete_node(&selected);
    }

    if(err != NULL)
    {
        *err = OS_NO_ERR;
    }
    return (void*)address;
}

static void add_block(kqueue_t* list,
                      uintptr_t first_frame,
                      const size_t length,
                      OS_RETURN_E* err)
{
    kqueue_node_t* cursor;
    kqueue_node_t* new_node;
    kqueue_node_t* last_cursor;
    kqueue_node_t* save_cursor;
    mem_range_t*   range;
    uintptr_t      limit;

    MEMMGT_ASSERT(list != NULL,
                  "Tried to add a memory block to a NULL list",
                  OS_ERR_NULL_POINTER);

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
                    kqueue_remove(list, save_cursor, TRUE);
                    kqueue_delete_node(&save_cursor);
                }
            }
            cursor->priority = range->base;

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
                    kqueue_remove(list, last_cursor, TRUE);
                    kqueue_delete_node(&last_cursor);
                }
            }

            break;
        }
        else if(range->base <= first_frame && range->limit > first_frame)
        {
            MEMMGT_ASSERT(FALSE,
                          "Tried to free an already free block",
                          OS_ERR_UNAUTHORIZED_ACTION);
        }
        else if(range->limit < first_frame)
        {
            /* Blocks are ordered by decreasing address, if the limit is ower
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
        MEMMGT_ASSERT(range != NULL,
                      "Could not create node data in memory manager",
                      OS_ERR_MALLOC);

        range->base  = first_frame;
        range->limit = limit;
        range->type  = MULTIBOOT_MEMORY_AVAILABLE;

        new_node = kqueue_create_node(range);
        kqueue_push_prio(new_node, list, first_frame);
    }

    if(err != NULL)
    {
        *err = OS_NO_ERR;
    }
}

static void map_kernel_section(uintptr_t start_addr, uintptr_t end_addr,
                               const bool_t read_only)
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
        pg_dir_entry      = (uintptr_t)start_addr >> PG_DIR_ENTRY_OFFSET;
        pg_table_entry    = ((uintptr_t)start_addr >> PG_TABLE_ENTRY_OFFSET) &
                             PG_TABLE_ENTRY_OFFSET_MASK;
        min_pgtable_entry = (((uintptr_t)start_addr - KERNEL_MEM_OFFSET) >>
                            PG_DIR_ENTRY_OFFSET) & PG_TABLE_ENTRY_OFFSET_MASK;
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

static OS_RETURN_E memory_invocate_cow(const uintptr_t addr)
{
    uintptr_t         start_align;
    uintptr_t         old_frame;
    uint32_t          ref_count;
    uint16_t          pgdir_entry;
    uint16_t          pgtable_entry;
    uint32_t*         pgdir_rec_addr;
    uint32_t*         pgtable;
    uintptr_t*        new_frame;
    uintptr_t*        tmp_page;
    OS_RETURN_E       err;
    uint32_t          int_state;
    kernel_process_t* curr_process;

    err = OS_ERR_MEMORY_NOT_MAPPED;

    /* Align addresses */
    start_align = addr & PAGE_ALIGN_MASK;

    /* Get entries */
    pgdir_entry   = (start_align >> PG_DIR_ENTRY_OFFSET);
    pgtable_entry = (start_align >> PG_TABLE_ENTRY_OFFSET) &
                    PG_TABLE_ENTRY_OFFSET_MASK;

    /* Check page directory presence and allocate if not present */
    pgdir_rec_addr = (uintptr_t*)&_KERNEL_RECUR_PG_DIR_BASE;

    curr_process = sched_get_current_process();

    MEMMGT_ASSERT(curr_process != NULL,
                  "COW called when no process is running",
                  OS_ERR_UNAUTHORIZED_ACTION);

    ENTER_CRITICAL(int_state);

    if((pgdir_rec_addr[pgdir_entry] & PG_DIR_FLAG_PAGE_PRESENT) != 0)
    {
        /* Check present in page table */
        pgtable = (uintptr_t*)(((uintptr_t)&_KERNEL_RECUR_PG_TABLE_BASE) +
                                KERNEL_PAGE_SIZE *
                                pgdir_entry);
        if((pgtable[pgtable_entry] & PG_DIR_FLAG_PAGE_PRESENT) != 0 &&
           (pgtable[pgtable_entry] & PAGE_FLAG_OS_CUSTOM_MASK) ==
            PAGE_FLAG_COPY_ON_WRITE)
        {
            /* Check reference count */
            old_frame = pgtable[pgtable_entry] & PG_ENTRY_ADDR_MASK;
            ref_count = memory_get_ref_count(old_frame);

            MEMMGT_ASSERT(ref_count != 0,
                          "Error in reference count management",
                          OS_ERR_OUT_OF_BOUND);

            if(ref_count > 1)
            {
                /* Allocate new frame */
                new_frame = memory_alloc_frames(1, &err);
                if(err != OS_NO_ERR)
                {
                    return err;
                }
                memory_acquire_ref((uintptr_t)new_frame);

                /* Temporary map new frame */
                tmp_page =
                    memory_alloc_pages_from(curr_process->free_page_table,
                                            1,
                                            MEM_ALLOC_BEGINING,
                                            &err);
                if(err != OS_NO_ERR)
                {
                    memory_free_frames(new_frame, KERNEL_PAGE_SIZE, NULL);
                    return err;
                }
                memory_mmap_direct(tmp_page,
                                   new_frame,
                                   KERNEL_PAGE_SIZE,
                                   0,
                                   0,
                                   1,
                                   0,
                                   &err);
                if(err != OS_NO_ERR)
                {
                    memory_free_frames(new_frame, KERNEL_PAGE_SIZE, NULL);
                    return err;
                }

                /* Copy to new frame */
                memcpy(tmp_page, (void*)start_align, KERNEL_PAGE_SIZE);

                /* Unmap temporary mapping */
                memory_munmap(tmp_page, KERNEL_PAGE_SIZE, &err);
                MEMMGT_ASSERT(err == OS_NO_ERR,
                              "COW could not unmap temporary page",
                              err);

                memory_free_pages_to(curr_process->free_page_table,
                                     tmp_page,
                                     1,
                                     &err);
                MEMMGT_ASSERT(err == OS_NO_ERR,
                              "COW could not free temporary page",
                              err);

                /* Update pg dir */
                pgtable[pgtable_entry] =
                    (pgtable[pgtable_entry] & ~PG_ENTRY_ADDR_MASK) |
                    (uintptr_t)new_frame;

                /* Decrement reference count */
                memory_release_ref(old_frame);

                KERNEL_DEBUG(MEMMGT_DEBUG_ENABLED,
                             "[MEMMGT] Copy on write copied 0x%p",
                             start_align);
            }
            pgtable[pgtable_entry] =
                ((pgtable[pgtable_entry] & ~PAGE_FLAG_OS_CUSTOM_MASK) &
                    ~PAGE_FLAG_READ_ONLY) |
                PAGE_FLAG_REGULAR      |
                PAGE_FLAG_READ_WRITE;

            KERNEL_DEBUG(MEMMGT_DEBUG_ENABLED,
                             "[MEMMGT] Copy on write set attributes 0x%p",
                             start_align);

            err = OS_NO_ERR;
        }
    }

    EXIT_CRITICAL(int_state);

    return err;
}

static void paging_fault_general_handler(cpu_state_t* cpu_state,
                                         uintptr_t int_id,
                                         stack_state_t* stack_state)
{
    uintptr_t fault_address;

    (void)cpu_state;
    (void)int_id;
    (void)stack_state;

    /* If the exception line is not right */
    if(int_id != PAGE_FAULT_LINE)
    {
        PANIC(OS_ERR_INCORRECT_VALUE, "MEMMGT",
              "Page fault handler invocated with wrong exception line.", TRUE);
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

    /* Check the copy on write mechanism */
    if(memory_invocate_cow(fault_address) == OS_NO_ERR)
    {
        return;
    }

    /* Kernel cannot handle page fault at the moment */
    PANIC(OS_ERR_UNAUTHORIZED_ACTION, "MEMMGT",
          "Page fault not resolved.", TRUE);
}

static bool_t is_mapped(const uintptr_t start_addr, const size_t size)
{
    uintptr_t start_align;
    uintptr_t to_check;
    uint32_t  found;
    uint16_t  pgdir_entry;
    uint16_t  pgtable_entry;
    uint32_t* pgdir_rec_addr;
    uint32_t* pgtable;
    uint32_t  int_state;

     /* Align addresses */
    start_align = start_addr & PAGE_ALIGN_MASK;

    /* Get mapping size */
    to_check = size + (start_addr - start_align);

    found = FALSE;

    ENTER_CRITICAL(int_state);
    while(to_check)
    {
        /* Get entries */
        pgdir_entry   = (start_align >> PG_DIR_ENTRY_OFFSET);
        pgtable_entry = (start_align >> PG_TABLE_ENTRY_OFFSET) & PG_TABLE_ENTRY_OFFSET_MASK;

        /* Check page directory presence and allocate if not present */
        pgdir_rec_addr = (uintptr_t*)&_KERNEL_RECUR_PG_DIR_BASE;
        if((pgdir_rec_addr[pgdir_entry] & PG_DIR_FLAG_PAGE_PRESENT) != 0)
        {
             /* Check present in page table */
            pgtable = (uintptr_t*)(((uintptr_t)&_KERNEL_RECUR_PG_TABLE_BASE) +
                                   KERNEL_PAGE_SIZE *
                                   pgdir_entry);
            if((pgtable[pgtable_entry] & PG_DIR_FLAG_PAGE_PRESENT) != 0)
            {
                found = TRUE;
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

    EXIT_CRITICAL(int_state);

    return found;
}

static kqueue_t* paging_copy_free_page_table(void)
{
    kqueue_t*      new_table;
    kqueue_t*      current_table;
    kqueue_node_t* cursor;
    kqueue_node_t* new_node;
    mem_range_t*  range;
    uint32_t      int_state;

    /* Create the new table */
    new_table = kqueue_create_queue();

    ENTER_CRITICAL(int_state);

    current_table = sched_get_current_process()->free_page_table;
    cursor = current_table->head;
    while(cursor)
    {
        /* Create range and node */
        range = kmalloc(sizeof(mem_range_t));
        MEMMGT_ASSERT(range != NULL,
                      "Could not allocate new free page table range",
                      OS_ERR_MALLOC);

        memcpy(range, cursor->data, sizeof(mem_range_t));
        new_node = kqueue_create_node(range);

        /* Add range to list */
        kqueue_push(new_node, new_table);

        /* Next entry */
        cursor = cursor->next;
    }

    EXIT_CRITICAL(int_state);

    return new_table;
}

static void kernel_mmap_internal(const void* virt_addr,
                                 const void* phys_addr,
                                 const size_t mapping_size,
                                 const uintptr_t flags,
                                 OS_RETURN_E* err)
{

    uintptr_t   virt_align;
    uintptr_t   phys_align;
    uint16_t    pgdir_entry;
    uint16_t    pgtable_entry;
    size_t      to_map;
    size_t      reverse_to_map;
    uintptr_t*  pgdir_rec_addr;
    uintptr_t*  pgtable;
    uint32_t    i;

    /* Align addresses */
    virt_align = (uintptr_t)virt_addr & PAGE_ALIGN_MASK;
    phys_align = (uintptr_t)phys_addr & PAGE_ALIGN_MASK;

    /* Get mapping size */
    to_map = mapping_size + ((uintptr_t)virt_addr - virt_align);
    reverse_to_map = to_map;

    if(err != NULL)
    {
        *err = OS_NO_ERR;
    }

    /* Check for existing mapping */
    if(is_mapped(virt_align, to_map) == TRUE)
    {
        MEMMGT_ASSERT(err != NULL,
                      "Trying to remap memory",
                      OS_ERR_MAPPING_ALREADY_EXISTS);

        *err = OS_ERR_MAPPING_ALREADY_EXISTS;
        return;
    }

    while(to_map != 0)
    {
        KERNEL_DEBUG(MEMMGT_DEBUG_ENABLED,
                     "[MEMMGT] Mapping page at 0x%p -> 0x%p",
                     virt_align,
                     phys_align);
        /* Get entries */
        pgdir_entry   = (virt_align >> PG_DIR_ENTRY_OFFSET);
        pgtable_entry = (virt_align >> PG_TABLE_ENTRY_OFFSET) &
                        PG_TABLE_ENTRY_OFFSET_MASK;

        /* Check page directory presence and allocate if not present */
        pgdir_rec_addr = (uint32_t*)&_KERNEL_RECUR_PG_DIR_BASE;
        if((pgdir_rec_addr[pgdir_entry] & PG_DIR_FLAG_PAGE_PRESENT) == 0)
        {
            pgtable = memory_alloc_frames(1, err);
            if(err != NULL && *err != OS_NO_ERR)
            {
                break;
            }

            /* Map page */
            pgdir_rec_addr[pgdir_entry] =
                (uintptr_t)pgtable |
                PG_DIR_FLAG_PAGE_SIZE_4KB |
                PG_DIR_FLAG_PAGE_SUPER_ACCESS |
                PG_DIR_FLAG_PAGE_READ_WRITE |
                PG_DIR_FLAG_PAGE_PRESENT;

            /* Get recursive virtual address */
            pgtable = (uint32_t*)(((uintptr_t)&_KERNEL_RECUR_PG_TABLE_BASE) +
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
            pgtable = (uint32_t*)(((uintptr_t)&_KERNEL_RECUR_PG_TABLE_BASE) +
                                  KERNEL_PAGE_SIZE *
                                  pgdir_entry);
        }

        /* Map the entry */
        pgtable[pgtable_entry] =
            phys_align |
            PAGE_FLAG_SUPER_ACCESS |
            flags |
            PAGE_FLAG_PRESENT;

        memory_acquire_ref((uintptr_t)phys_align);

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

    /* Clean in case of error */
    if(err != NULL && *err != OS_NO_ERR)
    {
        virt_align = (uintptr_t)virt_addr & PAGE_ALIGN_MASK;

        while(reverse_to_map > to_map)
        {
            /* Get entries */
            pgdir_entry   = (virt_align >> PG_DIR_ENTRY_OFFSET);
            pgtable_entry = (virt_align >> PG_TABLE_ENTRY_OFFSET) &
                            PG_TABLE_ENTRY_OFFSET_MASK;

            /* Check page directory presence and allocate if not present */
            pgdir_rec_addr = (uint32_t*)&_KERNEL_RECUR_PG_DIR_BASE;
            pgtable = (uint32_t*)(((uintptr_t)&_KERNEL_RECUR_PG_TABLE_BASE) +
                                    KERNEL_PAGE_SIZE *
                                    pgdir_entry);
            phys_align = pgtable[pgtable_entry] &  PG_ENTRY_ADDR_MASK;
            /* Unmap the entry */
            pgtable[pgtable_entry] = 0;
            for(i = 0; i < KERNEL_PGDIR_SIZE; ++i)
            {
                if((pgtable[i] & PAGE_FLAG_PRESENT) != 0)
                {
                    break;
                }
            }
            /* If the page table is empty */
            if(i == KERNEL_PAGE_SIZE)
            {
                memory_free_frames((void*)(pgdir_rec_addr[pgdir_entry] &
                                    PG_ENTRY_ADDR_MASK),
                                   1,
                                   NULL);
                pgdir_rec_addr[pgdir_entry] = 0;
            }

            memory_release_ref((uintptr_t)phys_align);

            KERNEL_DEBUG(MEMMGT_DEBUG_ENABLED,
                        "[MEMMGT] Unmapped page at 0x%p -> 0x%p",
                        virt_align,
                        phys_align);

            /* Update addresses and size */
            virt_align += KERNEL_PAGE_SIZE;
            if(reverse_to_map >= KERNEL_PAGE_SIZE)
            {
                reverse_to_map -= KERNEL_PAGE_SIZE;
            }
            else
            {
                reverse_to_map = 0;
            }
        }
    }

    if(err != NULL)
    {
        *err = OS_NO_ERR;
    }
}

static void paging_init(void)
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
    memory_get_symtab_range(&start_addr, &end_addr);
    map_kernel_section(start_addr, end_addr, 1);
    memory_get_kdata_range(&start_addr, &end_addr);
    map_kernel_section(start_addr, end_addr, 0);
    memory_get_kbss_range(&start_addr, &end_addr);
    map_kernel_section(start_addr, end_addr, 0);
    memory_get_kstacks_range(&start_addr, &end_addr);
    map_kernel_section(start_addr, end_addr, 0);
    memory_get_kheap_range(&start_addr, &end_addr);
    map_kernel_section(start_addr, end_addr, 0);
    memory_get_multiboot_range(&start_addr, &end_addr);
    map_kernel_section(start_addr, end_addr, 0);
    memory_get_initrd_range(&start_addr, &end_addr);
    map_kernel_section(start_addr, end_addr, 0);

    KERNEL_DEBUG(MEMMGT_DEBUG_ENABLED, "[MEMMGT] Mapped all kernel sections");

    /* Add page fault exception */
    err = kernel_exception_register_handler(PAGE_FAULT_LINE,
                                            paging_fault_general_handler);
    MEMMGT_ASSERT(err == OS_NO_ERR,
                  "Could not set page fault handler",
                  err);

    /* Set CR3 register */
    __asm__ __volatile__("mov %%eax, %%cr3": :"a"((uintptr_t)kernel_pgdir -
                                                  KERNEL_MEM_OFFSET));

    KERNEL_DEBUG(MEMMGT_DEBUG_ENABLED, "[MEMMGT] Set new page directory and"
                                       " page fault handler");

    memory_paging_enable();

    KERNEL_TEST_POINT(paging_test);
}

static void memory_paging_enable(void)
{
    uint32_t int_state;

    ENTER_CRITICAL(int_state);

    /* Enable paging, write protect */
    __asm__ __volatile__("mov %%cr0, %%eax\n\t"
                         "or $0x80010000, %%eax\n\t"
                         "mov %%eax, %%cr0\n\t"
                         : : : "eax");

    KERNEL_DEBUG(MEMMGT_DEBUG_ENABLED, "[MEMMGT] Paging enabled");

    EXIT_CRITICAL(int_state);
}

static OS_RETURN_E memory_copy_self_clean(mem_copy_self_data_t* data,
                                          OS_RETURN_E past_err)
{
    OS_RETURN_E err;

    if(data->new_data_page != NULL)
    {
        memory_free_pages_to(free_kernel_pages, data->new_data_page, 1, &err);
        data->new_data_page = NULL;
        MEMMGT_ASSERT(err == OS_NO_ERR,
                      "Error while recovering from faulted process copy.",
                      err);
    }

    if(data->new_pgtable_page != NULL)
    {
        memory_free_pages_to(free_kernel_pages, data->new_pgtable_page, 1, &err);
        data->new_pgtable_page = NULL;
        MEMMGT_ASSERT(err == OS_NO_ERR,
                      "Error while recovering from faulted process copy.",
                      err);
    }

    if(data->mapped_pgdir == TRUE)
    {
        memory_munmap(data->new_pgdir_page, KERNEL_PAGE_SIZE, &err);
        data->mapped_pgdir = FALSE;
        MEMMGT_ASSERT(err == OS_NO_ERR,
                      "Error while recovering from faulted process copy.",
                      err);
    }

    if(data->new_pgdir_page != NULL)
    {
        memory_free_pages_to(free_kernel_pages, data->new_pgdir_page, 1, &err);
        data->new_pgdir_page = NULL;
        MEMMGT_ASSERT(err == OS_NO_ERR,
                      "Error while recovering from faulted process copy.",
                      err);
    }

    if(data->new_pgdir_frame != NULL)
    {
        memory_free_frames(data->new_pgdir_frame, 1, &err);
        data->new_pgdir_frame = NULL;
        MEMMGT_ASSERT(err == OS_NO_ERR,
                      "Error while recovering from faulted process copy.",
                      err);
    }

    if(data->acquired_ref_pgdir == TRUE)
    {
        data->acquired_ref_pgdir = FALSE;
        memory_release_ref((uintptr_t)data->new_pgdir_frame);
    }

    kernel_error("Could not copy process mapping\n");

    return past_err;
}

static OS_RETURN_E memory_create_new_pagedir(mem_copy_self_data_t* data)
{
    OS_RETURN_E err;

    data->new_pgdir_frame = (uintptr_t*)memory_alloc_frames(1, &err);
    if(err != OS_NO_ERR)
    {
        KERNEL_ERROR("Could not copy process mapping\n");
        return err;
    }
    memory_acquire_ref((uintptr_t)data->new_pgdir_frame);
    data->acquired_ref_pgdir = TRUE;

    data->new_pgdir_page = (uintptr_t*)memory_alloc_pages_from(free_kernel_pages,
                                                         1,
                                                         MEM_ALLOC_BEGINING,
                                                         &err);
    if(err != OS_NO_ERR)
    {
        return memory_copy_self_clean(data, err);
    }

    memory_mmap_direct(data->new_pgdir_page, data->new_pgdir_frame,
                       KERNEL_PAGE_SIZE, 0, 0, 1,0, &err);
    if(err != OS_NO_ERR)
    {
        return memory_copy_self_clean(data, err);
    }
    data->mapped_pgdir = TRUE;

    /* Init the page directory to be empty */
    memset(data->new_pgdir_page, 0, KERNEL_PAGE_SIZE);

    return OS_NO_ERR;
}

static OS_RETURN_E memory_copy_self_pgtable(mem_copy_self_data_t* data)
{
    uintptr_t*  current_pgdir;
    uintptr_t*  current_pgtable;
    uintptr_t*  new_pgtable_frame;
    uint32_t    i;
    uint32_t    j;
    OS_RETURN_E err;
    OS_RETURN_E saved_err;

    /* The current page directory is always recursively mapped */
    current_pgdir = (uintptr_t*)&_KERNEL_RECUR_PG_DIR_BASE;

    /* Copy the page directory kernel entries, minus the recursive entry */
    for(i = KERNEL_FIRST_PGDIR_ENTRY; i < KERNEL_PGDIR_SIZE - 1; ++i)
    {
        data->new_pgdir_page[i] = current_pgdir[i];
    }

    /* Set the recursive entry on the new page directory */
    data->new_pgdir_page[KERNEL_PGDIR_SIZE - 1] =
                                            (uintptr_t)data->new_pgdir_frame |
                                            PG_DIR_FLAG_PAGE_SIZE_4KB        |
                                            PG_DIR_FLAG_PAGE_SUPER_ACCESS    |
                                            PG_DIR_FLAG_PAGE_READ_WRITE      |
                                            PG_DIR_FLAG_PAGE_PRESENT;

    /* Copy the rest of the page table and set copy on write */
    for(i = 0; i < KERNEL_FIRST_PGDIR_ENTRY; ++i)
    {
        if((current_pgdir[i] & PG_DIR_FLAG_PAGE_PRESENT) != 0)
        {
            /* Get recursive virtual address */
            current_pgtable = (uintptr_t*)(((uintptr_t)&_KERNEL_RECUR_PG_TABLE_BASE) +
                                           KERNEL_PAGE_SIZE *
                                           i);

            /* Create new page table */
            new_pgtable_frame = memory_alloc_frames(1, &err);
            if(err != OS_NO_ERR)
            {
                break;
            }

            memory_mmap_direct(data->new_pgtable_page,
                               new_pgtable_frame,
                               KERNEL_PAGE_SIZE,
                               0,
                               0,
                               1,
                               0,
                               &err);
            if(err != OS_NO_ERR)
            {
                memory_free_frames(new_pgtable_frame, 1, &err);
                MEMMGT_ASSERT(err == OS_NO_ERR,
                              "Error copying process image.",
                              err);
                break;
            }

            data->new_pgdir_page[i] = (current_pgdir[i] & ~PG_ENTRY_ADDR_MASK) |
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
                       (current_pgtable[j] & PAGE_FLAG_OS_CUSTOM_MASK) ==
                        PAGE_FLAG_REGULAR)
                    {

                        current_pgtable[j] = (current_pgtable[j] &
                                            ~PAGE_FLAG_READ_WRITE) |
                                            PAGE_FLAG_READ_ONLY    |
                                            PAGE_FLAG_COPY_ON_WRITE;
                        data->new_pgtable_page[j] = current_pgtable[j];
                        /* Increment the reference count */
                        memory_acquire_ref(data->new_pgtable_page[j] &
                                           PG_ENTRY_ADDR_MASK);
                    }
                    else if((current_pgtable[j] & PAGE_FLAG_OS_CUSTOM_MASK) !=
                            PAGE_FLAG_PRIVATE)
                    {
                        data->new_pgtable_page[j] = current_pgtable[j];
                        /* Increment the reference count */
                        memory_acquire_ref(data->new_pgtable_page[j] &
                                           PG_ENTRY_ADDR_MASK);
                    }
                    else
                    {
                        data->new_pgtable_page[j] = 0;
                    }
                }
                else
                {
                    data->new_pgtable_page[j] = 0;
                }
            }

            memory_munmap(data->new_pgtable_page, KERNEL_PAGE_SIZE, &err);
            MEMMGT_ASSERT(err == OS_NO_ERR,
                          "Error copying process image.",
                          err);
        }
    }

    saved_err = err;

    /* If we stopped because of an error */
    if(i != KERNEL_FIRST_PGDIR_ENTRY)
    {
        /* Skip the last that is already cleaned */
        --i;

        /* Free what we created */
        for(; (int32_t)i >= 0; --i)
        {
            if((current_pgdir[i] & PG_DIR_FLAG_PAGE_PRESENT) != 0)
            {
                /* Get recursive virtual address */
                current_pgtable = (uintptr_t*)(((uintptr_t)&_KERNEL_RECUR_PG_TABLE_BASE) +
                                            KERNEL_PAGE_SIZE *
                                            i);

                new_pgtable_frame = (uintptr_t*)
                                    (data->new_pgdir_page[i] & PG_ENTRY_ADDR_MASK);

                memory_mmap_direct(data->new_pgtable_page,
                                   new_pgtable_frame,
                                   KERNEL_PAGE_SIZE,
                                   0,
                                   0,
                                   1,
                                   0,
                                   &err);
                MEMMGT_ASSERT(err == OS_NO_ERR,
                              "Error copying process image.",
                              err);

                for(j = 0; j < KERNEL_PGDIR_SIZE; ++j)
                {
                    if((current_pgtable[j] & PG_DIR_FLAG_PAGE_PRESENT) != 0)
                    {
                        if((current_pgtable[j] & PAGE_FLAG_READ_WRITE) != 0 &&
                           (current_pgtable[j] & PAGE_FLAG_OS_CUSTOM_MASK) ==
                            PAGE_FLAG_REGULAR)
                        {
                            /* Reset current page values */
                            current_pgtable[j] &= ~PAGE_FLAG_COPY_ON_WRITE;
                            current_pgtable[j] |= PAGE_FLAG_READ_WRITE;

                            /* Decrement the reference count */
                            memory_release_ref(data->new_pgtable_page[j] &
                                               PG_ENTRY_ADDR_MASK);
                        }
                        else if((current_pgtable[j] &
                                 PAGE_FLAG_OS_CUSTOM_MASK) !=
                                PAGE_FLAG_PRIVATE)
                        {
                            /* Decrement the reference count */
                            memory_release_ref(data->new_pgtable_page[j] &
                                               PG_ENTRY_ADDR_MASK);
                        }
                    }
                }
                memory_munmap(data->new_pgtable_page, KERNEL_PAGE_SIZE, &err);
                MEMMGT_ASSERT(err == OS_NO_ERR,
                              "Error copying process image.",
                              err);
                memory_free_frames(new_pgtable_frame, 1, &err);
                MEMMGT_ASSERT(err == OS_NO_ERR,
                              "Error copying process image.",
                              err);
            }
        }
    }

    return saved_err;
}

static OS_RETURN_E memory_copy_self_stack(mem_copy_self_data_t* data,
                                          const void* kstack_addr,
                                          const size_t kstack_size)
{
    uintptr_t   curr_addr;
    uint16_t    pgtable_entry;
    uintptr_t*  new_pgtable_frame;
    uintptr_t*  new_data_frame;
    OS_RETURN_E err;

    curr_addr = (uintptr_t)kstack_addr;
    while(curr_addr < (uintptr_t)kstack_addr + kstack_size)
    {
        pgtable_entry = (curr_addr >> PG_TABLE_ENTRY_OFFSET) &
                        PG_TABLE_ENTRY_OFFSET_MASK;

        new_pgtable_frame = (uintptr_t*)
                            data->new_pgdir_page[curr_addr >> PG_DIR_ENTRY_OFFSET];
        if(((uintptr_t)new_pgtable_frame & PG_DIR_FLAG_PAGE_PRESENT) == 0)
        {
            new_pgtable_frame = memory_alloc_frames(1, &err);
            if(err != OS_NO_ERR)
            {
                KERNEL_ERROR("Could not create new kstack page frame\n");
                break;
            }
            memory_acquire_ref((uintptr_t)new_pgtable_frame);
            data->new_pgdir_page[curr_addr >> PG_DIR_ENTRY_OFFSET] =
                (uintptr_t)new_pgtable_frame  |
                PG_DIR_FLAG_PAGE_SIZE_4KB     |
                PG_DIR_FLAG_PAGE_SUPER_ACCESS |
                PG_DIR_FLAG_PAGE_READ_WRITE   |
                PG_DIR_FLAG_PAGE_PRESENT;
        }

        memory_mmap_direct(data->new_pgtable_page,
                           new_pgtable_frame,
                           KERNEL_PAGE_SIZE,
                           0,
                           0,
                           1,
                           0,
                           &err);
        if(err != OS_NO_ERR)
        {
            KERNEL_ERROR("Could not create new kstack page frame\n");
            memory_free_frames(new_pgtable_frame, 1, NULL);
            break;
        }

        /* Allocate the new frame */
        new_data_frame = memory_alloc_frames(1, &err);
        if(err != OS_NO_ERR)
        {
            KERNEL_ERROR("Could not create new kstack page frame\n");
            memory_munmap(data->new_pgtable_page, KERNEL_PAGE_SIZE, &err);
            MEMMGT_ASSERT(err == OS_NO_ERR,
                          "Error copying process stack.",
                          err);
            memory_free_frames(new_pgtable_frame, 1, &err);
            MEMMGT_ASSERT(err == OS_NO_ERR,
                          "Error copying process stack.",
                          err);
            break;
        }
        memory_acquire_ref((uintptr_t)new_data_frame);

        /* Map the new frame */
        memory_mmap_direct(data->new_data_page,
                           new_data_frame,
                           KERNEL_PAGE_SIZE,
                           0,
                           0,
                           1,
                           0,
                           &err);
        if(err != OS_NO_ERR)
        {
            KERNEL_ERROR("Could not create new kstack page frame\n");
            memory_munmap(data->new_pgtable_page, KERNEL_PAGE_SIZE, &err);
            MEMMGT_ASSERT(err == OS_NO_ERR,
                          "Error copying process stack.",
                          err);
            memory_free_frames(new_pgtable_frame, 1, &err);
            MEMMGT_ASSERT(err == OS_NO_ERR,
                          "Error copying process stack.",
                          err);
            memory_free_frames(new_data_frame, 1, &err);
            MEMMGT_ASSERT(err == OS_NO_ERR,
                          "Error copying process stack.",
                          err);
            break;
        }

        /* Copy data */
        memcpy(data->new_data_page, (void*)curr_addr, KERNEL_PAGE_SIZE);

        /* Unmap the new frame */
        memory_munmap(data->new_data_page, KERNEL_PAGE_SIZE, &err);
        MEMMGT_ASSERT(err == OS_NO_ERR,
                      "Error copying process stack.",
                      err);

        /* Update the mapping */
        data->new_pgtable_page[pgtable_entry] =
            (uintptr_t)new_data_frame |
            PAGE_FLAG_SUPER_ACCESS    |
            PAGE_FLAG_READ_WRITE      |
            PAGE_FLAG_CACHE_WB        |
            PAGE_FLAG_PRIVATE         |
            PAGE_FLAG_PRESENT;

        memory_munmap(data->new_pgtable_page, KERNEL_PAGE_SIZE, &err);
        MEMMGT_ASSERT(err == OS_NO_ERR,
                      "Error copying process stack.",
                      err);

        curr_addr += KERNEL_PAGE_SIZE;
    }

    /* If an error occured, free the resources */
    if(curr_addr < (uintptr_t)kstack_addr + kstack_size)
    {
        /* Skip frst that is already cleaned */
        curr_addr -= KERNEL_PAGE_SIZE;
        while(curr_addr > (uintptr_t)kstack_addr)
        {
            pgtable_entry = (curr_addr >> PG_TABLE_ENTRY_OFFSET) &
                            PG_TABLE_ENTRY_OFFSET_MASK;

            new_pgtable_frame = (uintptr_t*)
                            data->new_pgdir_page[curr_addr >> PG_DIR_ENTRY_OFFSET];

            memory_mmap_direct(data->new_pgtable_page,
                               new_pgtable_frame,
                               KERNEL_PAGE_SIZE,
                               0,
                               0,
                               1,
                               0,
                               &err);
            MEMMGT_ASSERT(err == OS_NO_ERR,
                          "Error copying process stack.",
                          err);

            /* Deallocate the new frame */
            new_data_frame = (uintptr_t*)(data->new_pgtable_page[pgtable_entry] &
                                         PG_ENTRY_ADDR_MASK);
            memory_free_frames(new_data_frame, 1, &err);
            MEMMGT_ASSERT(err == OS_NO_ERR,
                          "Error copying process stack.",
                          err);

            /* Update the mapping */
            data->new_pgtable_page[pgtable_entry] = 0;
            data->new_pgdir_page[curr_addr >> PG_DIR_ENTRY_OFFSET] = 0;

            memory_munmap(data->new_pgtable_page, KERNEL_PAGE_SIZE, &err);
            MEMMGT_ASSERT(err == OS_NO_ERR,
                          "Error copying process stack.",
                          err);

            curr_addr -= KERNEL_PAGE_SIZE;
        }

        memory_free_frames(new_pgtable_frame, 1, &err);
        MEMMGT_ASSERT(err == OS_NO_ERR,
                      "Error copying process stack.",
                      err);

        return OS_ERR_MEMORY_NOT_MAPPED;
    }

    return OS_NO_ERR;
}

void memory_manager_init(void)
{
    kqueue_node_t* cursor;
    mem_range_t*   mem_range;

    /* Print inital memory mapping */
    print_kernel_map();

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

    paging_init();
}

kqueue_t* memory_create_free_page_table(void)
{
    kqueue_t*      new_queue;
    kqueue_node_t* node;
    mem_range_t*   mem_range;

    /* Initialize kernel pages */
    new_queue = kqueue_create_queue();
    mem_range = kmalloc(sizeof(mem_range_t));
    MEMMGT_ASSERT(mem_range != NULL,
                  "Could not allocated memory range while creating page table",
                  OS_ERR_MALLOC);

    node = kqueue_create_node(mem_range);

    mem_range->base  = PROCESS_START_VIRT_SPACE;
    mem_range->limit = KERNEL_MEM_OFFSET;
    mem_range->type  = MULTIBOOT_MEMORY_AVAILABLE;

    kqueue_push_prio(node, new_queue, PROCESS_START_VIRT_SPACE);

    return new_queue;
}

void* memory_alloc_stack(const size_t stack_size,
                         const bool_t is_kernel,
                         OS_RETURN_E* err)
{
    uint32_t  int_state;
    uint32_t  frame_count;
    void*     stack_frames;
    void*     stack_pages;

    kernel_process_t* curr_proc;

    curr_proc = sched_get_current_process();

    if(curr_proc == NULL)
    {
        MEMMGT_ASSERT(err != NULL,
                      "Cannot allocate stack when no process is running",
                      OS_ERR_UNAUTHORIZED_ACTION);

        *err = OS_ERR_UNAUTHORIZED_ACTION;
        return NULL;
    }

    frame_count = stack_size / KERNEL_FRAME_SIZE;
    if(stack_size % KERNEL_FRAME_SIZE != 0)
    {
        ++frame_count;
    }

    ENTER_CRITICAL(int_state);

    /* Allocate frames and page */
    stack_frames = memory_alloc_frames(frame_count, err);
    if(err != NULL && *err != OS_NO_ERR)
    {
        KERNEL_ERROR("Error while allocating stack\n");
        return NULL;
    }
    stack_pages  = memory_alloc_pages_from(curr_proc->free_page_table,
                                           frame_count,
                                           MEM_ALLOC_END,
                                           err);
    if(err != NULL && *err != OS_NO_ERR)
    {
        memory_free_frames(stack_frames, frame_count, NULL);

        KERNEL_ERROR("Error while allocating stack\n");
        return NULL;
    }

    /* Add mapping */
    kernel_mmap_internal(stack_pages,
                         stack_frames,
                         stack_size,
                         PAGE_FLAG_READ_WRITE |
                         PAGE_FLAG_CACHE_WB   |
                         (is_kernel ? PAGE_FLAG_PRIVATE : PAGE_FLAG_REGULAR),
                         err);

    if(err != NULL && *err != OS_NO_ERR)
    {
        memory_free_frames(stack_frames, frame_count, NULL);
        memory_free_pages_to(curr_proc->free_page_table,
                             stack_pages,
                             frame_count,
                             NULL);

        KERNEL_ERROR("Error while allocating stack\n");
        return NULL;
    }

    EXIT_CRITICAL(int_state);

    return stack_pages;
}

OS_RETURN_E memory_free_stack(void* stack, const size_t stack_size)
{
    uint32_t    int_state;
    size_t      page_count;
    size_t      aligned_size;
    OS_RETURN_E err;


    aligned_size = stack_size + ((uintptr_t)stack & (KERNEL_PAGE_SIZE - 1));
    page_count = aligned_size / KERNEL_PAGE_SIZE;
    if(aligned_size % KERNEL_PAGE_SIZE != 0)
    {
        ++page_count;
    }

    ENTER_CRITICAL(int_state);

    memory_munmap(stack, stack_size, &err);
    MEMMGT_ASSERT(err == OS_NO_ERR,
                  "Cannot unmap free stack",
                  err);

    memory_free_pages_to(sched_get_current_process()->free_page_table,
                         stack, page_count, &err);

    EXIT_CRITICAL(int_state);

    return err;
}

void memory_mmap(const void* virt_addr,
                  const size_t mapping_size,
                  const bool_t read_only,
                  const bool_t exec,
                  OS_RETURN_E* err)
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
    phys_addr = memory_alloc_frames(to_map / KERNEL_FRAME_SIZE, err);

    kernel_mmap_internal(virt_addr,
                         phys_addr,
                         to_map,
            (read_only ? PAGE_FLAG_READ_ONLY : PAGE_FLAG_READ_WRITE) |
            PAGE_FLAG_CACHE_WB |
            PAGE_FLAG_REGULAR  |
            (exec ? 0 : 0),
            err);

    if(err != NULL && *err != OS_NO_ERR)
    {
        memory_free_frames(phys_addr, to_map / KERNEL_FRAME_SIZE, NULL);
    }

    EXIT_CRITICAL(int_state);
}

void memory_mmap_direct(const void* virt_addr,
                        const void* phys_addr,
                        const size_t mapping_size,
                        const bool_t read_only,
                        const bool_t exec,
                        const bool_t cache_enabled,
                        const bool_t is_hw,
                        OS_RETURN_E* err)
{
    uint32_t int_state;

    ENTER_CRITICAL(int_state);

    kernel_mmap_internal(virt_addr,
                         phys_addr,
                         mapping_size,
            (read_only ? PAGE_FLAG_READ_ONLY : PAGE_FLAG_READ_WRITE) |
            (cache_enabled ? PAGE_FLAG_CACHE_WB : PAGE_FLAG_CACHE_DISABLED) |
            (is_hw ? PAGE_FLAG_HARDWARE : PAGE_FLAG_REGULAR) |
            (exec ? 0 : 0),
            err);

    EXIT_CRITICAL(int_state);
}

void memory_munmap(void* virt_addr,
                   const size_t mapping_size,
                   OS_RETURN_E* err)
{
    uintptr_t   end_map;
    uintptr_t   start_map;
    uint16_t    pgdir_entry;
    uint16_t    pgtable_entry;
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
        pgdir_entry   = (start_map >> PG_DIR_ENTRY_OFFSET);
        pgtable_entry = (start_map >> PG_TABLE_ENTRY_OFFSET) &
                         PG_TABLE_ENTRY_OFFSET_MASK;

        /* Check page directory presence and allocate if not present */
        pgdir_rec_addr = (uint32_t*)&_KERNEL_RECUR_PG_DIR_BASE;
        if((pgdir_rec_addr[pgdir_entry] & PG_DIR_FLAG_PAGE_PRESENT) != 0)
        {
            /* Get recursive virtual address */
            pgtable = (uint32_t*)(((uintptr_t)&_KERNEL_RECUR_PG_TABLE_BASE) +
                                  KERNEL_PAGE_SIZE *
                                  pgdir_entry);

            if((pgtable[pgtable_entry] & PAGE_FLAG_PRESENT) != 0)
            {
                /* Unmap */
                KERNEL_DEBUG(MEMMGT_DEBUG_ENABLED,
                             "[MEMMGT] Unmapped page at 0x%p",
                             start_map);

                /* Decrement the ref count and potentialy free frame */
                memory_release_ref(pgtable[pgtable_entry] & PG_ENTRY_ADDR_MASK);
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
                    (void*)(pgdir_rec_addr[pgdir_entry] & PG_ENTRY_ADDR_MASK),
                    1,
                    NULL);
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

    if(err != NULL)
    {
        *err = OS_NO_ERR;
    }
}

OS_RETURN_E memory_copy_self_mapping(kernel_process_t* dst_process,
                                     const void* kstack_addr,
                                     const size_t kstack_size)
{
    OS_RETURN_E          err;
    mem_copy_self_data_t data;

    if(dst_process == NULL)
    {
        return OS_ERR_NULL_POINTER;
    }

    memset(&data, 0, sizeof(mem_copy_self_data_t));

    KERNEL_DEBUG(MEMMGT_DEBUG_ENABLED, "[MEMMGT] Copying process image");

    /* Create a new page directory and map for kernel */
    err = memory_create_new_pagedir(&data);
    if(err != OS_NO_ERR)
    {
        return memory_copy_self_clean(&data, err);
    }

    /* Create temporary pages */
    data.new_pgtable_page = memory_alloc_pages_from(free_kernel_pages,
                                               1,
                                               MEM_ALLOC_BEGINING,
                                               &err);
    if(err != OS_NO_ERR)
    {
        return memory_copy_self_clean(&data, err);
    }

    data.new_data_page = memory_alloc_pages_from(free_kernel_pages,
                                                 1,
                                                 MEM_ALLOC_BEGINING,
                                                 &err);
    if(err != OS_NO_ERR)
    {
        return memory_copy_self_clean(&data, err);
    }

    /* Copy the page table and set COW for both processes */
    err = memory_copy_self_pgtable(&data);
    if(err != OS_NO_ERR)
    {
        return memory_copy_self_clean(&data, err);
    }

    /* Map and duplicate the kernel stack */
    err = memory_copy_self_stack(&data, kstack_addr, kstack_size);
    if(err != OS_NO_ERR)
    {
        return memory_copy_self_clean(&data, err);
    }

    /* Unmap the new page directory for the kernel */
    memory_munmap(data.new_pgdir_page, KERNEL_PAGE_SIZE, &err);
    data.mapped_pgdir = FALSE;
    MEMMGT_ASSERT(err == OS_NO_ERR,
                  "Cannot clean temporary data for process copy",
                  err);
    memory_free_pages_to(free_kernel_pages, data.new_pgdir_page, 1, &err);
    data.new_pgdir_page = NULL;
    MEMMGT_ASSERT(err == OS_NO_ERR,
                  "Cannot clean temporary data for process copy",
                  err);
    memory_free_pages_to(free_kernel_pages, data.new_pgtable_page, 1, &err);
    data.new_pgtable_page = NULL;
    MEMMGT_ASSERT(err == OS_NO_ERR,
                  "Cannot clean temporary data for process copy",
                  err);
    memory_free_pages_to(free_kernel_pages, data.new_data_page, 1, &err);
    MEMMGT_ASSERT(err == OS_NO_ERR,
                  "Cannot clean temporary data for process copy",
                  err);

    /* Set the destination process data */
    dst_process->page_dir = (uintptr_t)data.new_pgdir_frame;
    dst_process->free_page_table = paging_copy_free_page_table();
    if(err != OS_NO_ERR)
    {
        return memory_copy_self_clean(&data, err);
    }

    return OS_NO_ERR;
}

uintptr_t memory_get_phys_addr(const uintptr_t virt_addr)
{
    uint16_t  pgdir_entry;
    uint16_t  pgtable_entry;
    uint32_t* pgdir_rec_addr;
    uint32_t* pgtable;

    /* Get entries */
    pgdir_entry   = (virt_addr >> PG_DIR_ENTRY_OFFSET);
    pgtable_entry = (virt_addr >> PG_TABLE_ENTRY_OFFSET) & PG_TABLE_ENTRY_OFFSET_MASK;

    /* Check page directory presence and allocate if not present */
    pgdir_rec_addr = (uint32_t*)&_KERNEL_RECUR_PG_DIR_BASE;
    if((pgdir_rec_addr[pgdir_entry] & PG_DIR_FLAG_PAGE_PRESENT) != 0)
    {
            /* Check present in page table */
        pgtable = (uint32_t*)(((uintptr_t)&_KERNEL_RECUR_PG_TABLE_BASE) +
                                KERNEL_PAGE_SIZE *
                                pgdir_entry);
        if((pgtable[pgtable_entry] & PG_DIR_FLAG_PAGE_PRESENT) != 0)
        {
            return (pgtable[pgtable_entry] & PG_ENTRY_ADDR_MASK) |
                   (~PG_ENTRY_ADDR_MASK & virt_addr);
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
                 "[MEMMGT] Adding HW region 0x%p -> 0x%p to reference table",
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
            frame_ref_dir[dir_entry] = (uintptr_t)kmalloc(FRAME_REF_TABLE_SIZE *
                                                          sizeof(uintptr_t));
            memset((void*)frame_ref_dir[dir_entry],
                   0,
                   FRAME_REF_TABLE_SIZE * sizeof(uintptr_t));
            current_table = (uintptr_t*)frame_ref_dir[dir_entry];
        }
        else
        {
            current_table = (uintptr_t*)frame_ref_dir[dir_entry];
            if(current_table[table_entry] != (uintptr_t)NULL)
            {

                KERNEL_ERROR("Reference table cannot have multiple ref 0x%p\n",
                                current_addr);
                err = OS_ERR_UNAUTHORIZED_ACTION;
                current_addr -= KERNEL_FRAME_SIZE;
                break;
            }
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
                if(current_table[i] != 0)
                {
                    break;
                }
            }
            if(i == FRAME_REF_TABLE_SIZE)
            {
                /* It should release the frame too */
                kfree((void*)frame_ref_dir[dir_entry]);
                frame_ref_dir[dir_entry] = (uintptr_t)NULL;
            }
            current_addr -= KERNEL_FRAME_SIZE;
        }
    }
    EXIT_CRITICAL(int_state);

    KERNEL_DEBUG(MEMMGT_DEBUG_ENABLED,
                 "[MEMMGT] Added HW region 0x%p -> 0x%p to reference table",
                phys_addr, phys_addr + size);

    return err;
}

void memory_delete_free_page_table(kqueue_t* page_table)
{
    kqueue_node_t* mem_range;

    if(page_table == NULL)
    {
        return;
    }

    mem_range = kqueue_pop(page_table);
    while(mem_range != NULL)
    {
        kfree(mem_range->data);
        kqueue_delete_node(&mem_range);

        mem_range = kqueue_pop(page_table);
    }

    kqueue_delete_queue(&page_table);
}

void memory_clean_process_memory(uintptr_t pg_dir)
{
    uint16_t pgdir_entry;
    uint16_t pgtable_entry;

    uintptr_t* pgdir_page;
    uintptr_t* pgtable_page;

    /* Allocate temporary tables */
    pgdir_page   = memory_alloc_pages_from(free_kernel_pages,
                                           1,
                                           MEM_ALLOC_BEGINING,
                                           NULL);
    pgtable_page = memory_alloc_pages_from(free_kernel_pages,
                                           1,
                                           MEM_ALLOC_BEGINING,
                                           NULL);

    memory_mmap_direct(pgdir_page,
                       (void*)pg_dir,
                       KERNEL_PAGE_SIZE,
                       0,
                       0,
                       1,
                       0,
                       NULL);

    for(pgdir_entry = 0; pgdir_entry < KERNEL_FIRST_PGDIR_ENTRY; ++pgdir_entry)
    {
        if((pgdir_page[pgdir_entry] & PG_DIR_FLAG_PAGE_PRESENT) == 0)
        {
            continue;
        }

        memory_mmap_direct(pgtable_page,
                           (void*)(pgdir_page[pgdir_entry] & PG_ENTRY_ADDR_MASK),
                           KERNEL_PAGE_SIZE,
                           0,
                           0,
                           1,
                           0,
                           NULL);

        for(pgtable_entry = 0;
            pgtable_entry < KERNEL_PGDIR_SIZE;
            ++pgtable_entry)
        {
            if((pgtable_page[pgtable_entry] & PG_DIR_FLAG_PAGE_PRESENT) != 0)
            {
                /* This will free the frame is the reference count is 0 */
                memory_release_ref(pgtable_page[pgtable_entry] &
                                   PG_ENTRY_ADDR_MASK);
            }
        }

        memory_munmap(pgtable_page, KERNEL_PAGE_SIZE, NULL);
        memory_release_ref((uintptr_t)(pgdir_page[pgdir_entry] & PG_ENTRY_ADDR_MASK) &
                                   PG_ENTRY_ADDR_MASK);
    }

    memory_munmap(pgdir_page, KERNEL_PAGE_SIZE, NULL);
    memory_release_ref((uintptr_t)pg_dir &
                                   PG_ENTRY_ADDR_MASK);

    memory_free_pages_to(free_kernel_pages, pgdir_page, 1, NULL);
    memory_free_pages_to(free_kernel_pages, pgtable_page, 1, NULL);
}

void memory_free_process_data(const void* virt_addr,
                              const size_t size,
                              kernel_process_t* process)
{
    uintptr_t   current_addr;
    size_t      to_unmap;
    uint16_t    pgdir_entry;
    uint16_t    pgtable_entry;
    uintptr_t*  pgdir_page;
    uintptr_t*  pgtable_page;
    OS_RETURN_E err;

    MEMMGT_ASSERT(process != NULL,
                  "Cannot free process data of NULL process",
                  OS_ERR_NULL_POINTER);

    MEMMGT_ASSERT(process != sched_get_current_process(),
                  "Cannot free process data of an active process",
                  OS_ERR_UNAUTHORIZED_ACTION);

    /* Allocate temporary tables */
    pgdir_page   = memory_alloc_pages_from(free_kernel_pages,
                                           1,
                                           MEM_ALLOC_BEGINING,
                                           &err);
    MEMMGT_ASSERT(err == OS_NO_ERR,
                  "Cannot allocate temporary page when freeing process memory",
                  err);

    pgtable_page = memory_alloc_pages_from(free_kernel_pages,
                                           1,
                                           MEM_ALLOC_BEGINING,
                                           &err);
    MEMMGT_ASSERT(err == OS_NO_ERR,
                  "Cannot allocate temporary page when freeing process memory",
                  err);

    memory_mmap_direct(pgdir_page,
                       (void*)process->page_dir,
                       KERNEL_PAGE_SIZE,
                       0,
                       0,
                       1,
                       0,
                       &err);
    MEMMGT_ASSERT(err == OS_NO_ERR,
                  "Cannot map temporary page when freeing process memory",
                  err);

    current_addr = (uintptr_t)virt_addr & ~(KERNEL_PAGE_SIZE - 1);
    to_unmap = size + (uintptr_t)virt_addr - current_addr;

    while(current_addr < (uintptr_t)virt_addr + to_unmap)
    {
        pgdir_entry   = current_addr >> PG_DIR_ENTRY_OFFSET;
        pgtable_entry = (current_addr >> PG_TABLE_ENTRY_OFFSET) &
                        PG_TABLE_ENTRY_OFFSET_MASK;

        MEMMGT_ASSERT((pgdir_page[pgdir_entry] & PG_DIR_FLAG_PAGE_PRESENT) != 0,
                      "Trying to free already unmapped data for process",
                      OS_ERR_UNAUTHORIZED_ACTION);

        memory_mmap_direct(pgtable_page,
                           (void*)(pgdir_page[pgdir_entry] & PG_ENTRY_ADDR_MASK),
                           KERNEL_PAGE_SIZE,
                           0,
                           0,
                           1,
                           0,
                           NULL);

        MEMMGT_ASSERT((pgtable_page[pgtable_entry] &
                       PG_DIR_FLAG_PAGE_PRESENT) != 0,
                      "Trying to free already unmapped data for process",
                      OS_ERR_UNAUTHORIZED_ACTION);

        /* This will free the frame is the reference count is 0 */
        memory_release_ref(pgtable_page[pgtable_entry] & PG_ENTRY_ADDR_MASK);
        pgtable_page[pgtable_entry] = 0;

        /* Free in page table */
        memory_free_pages_to(process->free_page_table,
                             (uintptr_t*)current_addr,
                             1,
                             &err);
        MEMMGT_ASSERT(err == OS_NO_ERR,
                      "Cannot free temporary data when free process memory",
                      err);

        memory_munmap(pgtable_page, KERNEL_PAGE_SIZE, &err);
        MEMMGT_ASSERT(err == OS_NO_ERR,
                      "Cannot free temporary data when free process memory",
                      err);

        current_addr += KERNEL_PAGE_SIZE;
    }

    memory_munmap(pgdir_page, KERNEL_PAGE_SIZE, &err);
    MEMMGT_ASSERT(err == OS_NO_ERR,
                  "Cannot unmap temporary data when free process memory",
                  err);

    memory_free_pages_to(free_kernel_pages, pgdir_page, 1, &err);
    MEMMGT_ASSERT(err == OS_NO_ERR,
                  "Cannot free temporary data when free process memory",
                  err);
    memory_free_pages_to(free_kernel_pages, pgtable_page, 1, &err);
    MEMMGT_ASSERT(err == OS_NO_ERR,
                  "Cannot free temporary data when free process memory",
                  err);
}

void* memory_alloc_kernel_pages(const size_t page_count, OS_RETURN_E* err)
{
    return memory_alloc_pages_from(free_kernel_pages,
                                   page_count,
                                   MEM_ALLOC_BEGINING,
                                   err);
}

void memory_free_kernel_pages(const void* page_addr,
                              const size_t page_count,
                              OS_RETURN_E* err)
{
    memory_free_pages_to(free_kernel_pages, page_addr, page_count, err);
}

void memory_alloc_page(const SYSCALL_FUNCTION_E func, void* params)
{
    uint32_t                   int_state;
    uint32_t                   frame_count;
    void*                      frames;
    void*                      pages;
    memmgt_page_alloc_param_t* func_params;
    kernel_process_t*          curr_proc;
    OS_RETURN_E                err;

    func_params = (memmgt_page_alloc_param_t*)params;

    if(func != SYSCALL_PAGE_ALLOC)
    {
        if(func_params != NULL)
        {
            func_params->error = OS_ERR_UNAUTHORIZED_ACTION;
        }
        return;
    }
    if(func_params == NULL)
    {
        return;
    }

    curr_proc = sched_get_current_process();

    if(curr_proc == NULL)
    {
        KERNEL_ERROR("Cannot allocate pages when no process is running\n");
        func_params->error = OS_ERR_UNAUTHORIZED_ACTION;
        return;
    }

    frame_count = func_params->page_count;

    ENTER_CRITICAL(int_state);

    /* Allocate frames and page */
    frames = memory_alloc_frames(frame_count, &err);
    if(err != OS_NO_ERR)
    {
        func_params->error = err;
        KERNEL_ERROR("Error while allocating pages\n");
        EXIT_CRITICAL(int_state);
        return;
    }
    pages  = memory_alloc_pages_from(curr_proc->free_page_table,
                                     frame_count,
                                     MEM_ALLOC_BEGINING,
                                    &err);
    if(err != OS_NO_ERR)
    {
        func_params->error = err;
        memory_free_frames(frames, frame_count, &err);
        MEMMGT_ASSERT(err == OS_NO_ERR,
                      "Cannot free frame for allocated page",
                      err);
        EXIT_CRITICAL(int_state);
        return;
    }

    /* Add mapping */
    kernel_mmap_internal(pages,
                         frames,
                         frame_count * KERNEL_FRAME_SIZE,
                         PAGE_FLAG_READ_WRITE |
                         PAGE_FLAG_CACHE_WB   |
                         PAGE_FLAG_REGULAR,
                         &err);

    if(err != OS_NO_ERR)
    {
        func_params->error = err;
        memory_free_frames(frames, frame_count, &err);
        MEMMGT_ASSERT(err == OS_NO_ERR,
                      "Cannot free frame for allocated page",
                      err);

        memory_free_pages_to(curr_proc->free_page_table,
                             pages,
                             frame_count,
                             &err);
        MEMMGT_ASSERT(err == OS_NO_ERR,
                      "Cannot free frame for allocated page",
                      err);

        EXIT_CRITICAL(int_state);
        KERNEL_ERROR("Error while allocating frames\n");
        return;
    }

    EXIT_CRITICAL(int_state);

    func_params->start_addr = pages;
    func_params->error      = OS_NO_ERR;
}

uint32_t memory_get_free_kpages(void)
{
    return get_free_mem(free_kernel_pages);
}

uint32_t memory_get_free_pages(void)
{
    kernel_process_t* curr_proc;

    curr_proc = sched_get_current_process();

    return get_free_mem(curr_proc->free_page_table);
}

uint32_t memory_get_free_frames(void)
{
    return get_free_mem(free_memory_map);
}