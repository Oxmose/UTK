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
 * CONSTANTS
 ******************************************************************************/

/* None */

/*******************************************************************************
 * STRUCTURES
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
/** @brief Kernel multiboot structures memory address. */
extern uint32_t _KERNEL_MULTIBOOT_MEM_ADDR;
/** @brief Kernel multiboot structures memory size. */
extern uint8_t _KERNEL_MULTIBOOT_MEM_SIZE;
/** @brief Kernel init ram disk memory address. */
extern uint8_t _KERNEL_INITRD_MEM_ADDR;
/** @brief Kernel init ram disk memory size. */
extern uint8_t _KERNEL_INITRD_MEM_SIZE;
/** @brief Kernel memory end address. */
extern uint8_t _KERNEL_MEMORY_END;
/** @brief Kernel recursive mapping address for page tables */
extern uint32_t _KERNEL_RECUR_PG_TABLE_BASE;
/** @brief Kernel recursive mapping address for page directory */
extern uint8_t *_KERNEL_RECUR_PG_DIR_BASE;

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
static void* memory_alloc_pages_from(queue_t* page_table, 
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
static void memory_free_pages_to(queue_t* page_table, 
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

static void print_kernel_map(void);

static void detect_memory(void);

static void setup_mem_table(void);

static void* get_block(queue_t* list, 
                       const size_t length, 
                       const MEM_ALLOC_START_E start_pt,
                       OS_RETURN_E* err);

static void add_block(queue_t* list, 
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
 * @param[out] err The error buffer to store the operation's result. If NULL, 
 * the function will raise a kernel panic in case of error.
 * 
 * @return A deep copy of the currentp process free page table is returned.
 */
static queue_t* paging_copy_free_page_table(OS_RETURN_E* err);

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

/*******************************************************************************
 * FUNCTIONS
 ******************************************************************************/

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
            KERNEL_PANIC(internal_err);
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

static void memory_free_frames(void* frame_addr, 
                               const size_t frame_count,
                               OS_RETURN_E* err)
{
    uint32_t      int_state;
    queue_node_t* cursor;
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
            KERNEL_PANIC(OS_ERR_UNAUTHORIZED_ACTION);
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
            KERNEL_PANIC(internal_err);
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

static void* memory_alloc_pages_from(queue_t* page_table, 
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
            KERNEL_PANIC(internal_err);
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

static void memory_free_pages_to(queue_t* page_table, 
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
            KERNEL_PANIC(internal_err);
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

    if(err != NULL)
    {
        *err = OS_NO_ERR;
    }

    EXIT_CRITICAL(int_state);
}

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

    

    dir_entry   = phys_addr >> FRAME_REF_DIR_ENTRY_OFFSET;
    table_entry = (phys_addr >> FRAME_REF_TABLE_ENTRY_OFFSET) &
                  FRAME_REF_TABLE_ENTRY_OFFSET_MASK;

    ENTER_CRITICAL(int_state);

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

    if(current_table == NULL)
    {
        KERNEL_ERROR("Tried to get reference on non existing memory 0x%p\n", 
                     phys_addr);
        KERNEL_PANIC(OS_ERR_UNAUTHORIZED_ACTION);
    }

    if(current_table[table_entry] == (uintptr_t)NULL)
    {
        KERNEL_ERROR("Tried to get reference on non existing memory 0x%p\n", 
                     phys_addr);
        KERNEL_PANIC(OS_ERR_UNAUTHORIZED_ACTION);
    }

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

    if(current_table == NULL)
    {
        KERNEL_ERROR("Tried to get reference on non existing memory 0x%p\n", 
                     phys_addr);
        KERNEL_PANIC(OS_ERR_UNAUTHORIZED_ACTION);
    }

    if(current_table[table_entry] == (uintptr_t)NULL)
    {
        KERNEL_ERROR("Tried to get reference on non existing memory 0x%p\n", 
                     phys_addr);
        KERNEL_PANIC(OS_ERR_UNAUTHORIZED_ACTION);
    }

    current_table[table_entry] = (current_table[table_entry] & 
                                  ~FRAME_REF_COUNT_MASK) | 
                                  (count & FRAME_REF_COUNT_MASK);

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
                if(frame_ref_dir[dir_entry] == (uintptr_t)NULL)
                {
                    KERNEL_ERROR("Cannot allocated reference page table\n",
                                 current_addr);
                    KERNEL_PANIC(OS_ERR_MALLOC);
                }
                
                memset((void*)frame_ref_dir[dir_entry], 
                       0, 
                       FRAME_REF_TABLE_SIZE * sizeof(uintptr_t));
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
        *start = (uintptr_t)&_KERNEL_MULTIBOOT_MEM_ADDR;
    }
    if(end != NULL)
    {
        *end = ((uintptr_t)&_KERNEL_MULTIBOOT_MEM_ADDR) + 
               ((uintptr_t)&_KERNEL_MULTIBOOT_MEM_SIZE);
    }
}

static void memory_get_initrd_range(uintptr_t* start, uintptr_t* end)
{
    if(start != NULL)
    {
        *start = (uintptr_t)&_KERNEL_INITRD_MEM_ADDR;
    }
    if(end != NULL)
    {
        *end = (uintptr_t)&_KERNEL_INITRD_MEM_ADDR + 
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
                    (uintptr_t)&_KERNEL_MULTIBOOT_MEM_ADDR,
                    ((uintptr_t)&_KERNEL_MULTIBOOT_MEM_ADDR) + 
                    (uintptr_t)&_KERNEL_MULTIBOOT_MEM_SIZE,
                    ((uintptr_t)&_KERNEL_MULTIBOOT_MEM_SIZE) >> 10);
    KERNEL_INFO("INITRD          0x%p -> 0x%p | "PRIPTR"KB\n",
                    &_KERNEL_INITRD_MEM_ADDR,
                    &_KERNEL_INITRD_MEM_ADDR + 
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
    OS_RETURN_E             error;
    queue_node_t*           node;
    queue_node_t*           node2;
    uint32_t                entry_size;
    uint32_t                i;

    /* Create memory tables */
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

    /* Update address to higher half kernel */
    KERNEL_DEBUG(MEMMGT_DEBUG_ENABLED, 
                 "[MEMMGT] Reading memory configuration from 0x%x", 
                 (uintptr_t)&_KERNEL_MULTIBOOT_MEM_ADDR);

    /* Get multiboot data */
    multiboot_info_size = *(uint32_t*)&_KERNEL_MULTIBOOT_MEM_ADDR;
    multiboot_tag = (struct multiboot_tag*)
                    (((uintptr_t)&_KERNEL_MULTIBOOT_MEM_ADDR) + 8);

    KERNEL_DEBUG(MEMMGT_DEBUG_ENABLED, 
                 "[MEMMGT] Memory configuration size 0x%p", 
                 multiboot_info_size);

    /* Search for memory information */
    available_memory = 0;
    while((uintptr_t)multiboot_tag < 
          ((uintptr_t)&_KERNEL_MULTIBOOT_MEM_ADDR) + multiboot_info_size)
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
                mem_range->base  = (uintptr_t)curr_entry->addr;
                mem_range->limit = (uintptr_t)curr_entry->addr + 
                                   (uintptr_t)curr_entry->len;
                mem_range->type  = curr_entry->type;

                /* Low memory is treated as HW */
                if(curr_entry->type == MULTIBOOT_MEMORY_AVAILABLE && 
                   curr_entry->addr >= KERNEL_MEM_START)
                {
                    mem_range2 = kmalloc(sizeof(mem_range_t));
                    if(mem_range2 == NULL)
                    {
                        KERNEL_ERROR("Could not allocate memory range"
                                     " structure\n");
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
                    mem_range2->base  = (uintptr_t)curr_entry->addr;
                    mem_range2->limit = (uintptr_t)curr_entry->addr + 
                                        (uintptr_t)curr_entry->len;
                    mem_range2->type  = curr_entry->type;

                    error = queue_push_prio(node2, 
                                            free_memory_map, 
                                            mem_range2->base);
                    if(error != OS_NO_ERR)
                    {
                        KERNEL_ERROR("Could not enqueue memory range node\n");
                        KERNEL_PANIC(error);
                    }
                    available_memory += (uintptr_t)curr_entry->len;
                }

                error = queue_push_prio(node, hw_memory_map, mem_range->base);
                if(error != OS_NO_ERR)
                {
                    KERNEL_ERROR("Could not enqueue memory range node\n");
                    KERNEL_PANIC(error);
                }
            }

        }
        multiboot_tag = (struct multiboot_tag*)
                        ((uintptr_t)multiboot_tag + entry_size);
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
    mem_range->limit = (uintptr_t)&_KERNEL_RECUR_PG_TABLE_BASE;
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
    uintptr_t     address;
    OS_RETURN_E   internal_err;

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
        KERNEL_ERROR("No mode free memory\n");
        if(err != NULL)
        {
            *err = OS_ERR_NO_MORE_FREE_MEM;
            return NULL;
        }
        else
        {
            KERNEL_PANIC(OS_ERR_NO_MORE_FREE_MEM);
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
        internal_err = queue_remove(list, selected);
        internal_err |= queue_delete_node(&selected);

        if(internal_err != OS_NO_ERR)
        {
            KERNEL_ERROR("Could not get memory block\n");
            KERNEL_PANIC(internal_err);
        }
    }

    if(err != NULL)
    {
        *err = OS_NO_ERR;
    }
    return (void*)address;
}

static void add_block(queue_t* list, 
                      uintptr_t first_frame, 
                      const size_t length,
                      OS_RETURN_E* err)
{
    queue_node_t* cursor;
    queue_node_t* new_node;
    queue_node_t* last_cursor;
    queue_node_t* save_cursor;
    mem_range_t*  range;
    uintptr_t     limit;
    OS_RETURN_E   internal_err;

    if(list == NULL)
    {
        KERNEL_ERROR("Tried to add a memory block to a NULL list\n");
        if(err != NULL)
        {
            *err = OS_ERR_NULL_POINTER;
            return;
        }
        else
        {
            KERNEL_PANIC(OS_ERR_NULL_POINTER);
        }
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
                    internal_err = queue_remove(list, save_cursor);
                    internal_err |= queue_delete_node(&save_cursor);

                    if(internal_err != OS_NO_ERR)
                    {
                        KERNEL_ERROR("Could not add memory block\n");
                        KERNEL_PANIC(internal_err);
                    }
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
                    internal_err = queue_remove(list, last_cursor);
                    internal_err |= queue_delete_node(&last_cursor);

                    if(internal_err != OS_NO_ERR)
                    {
                        KERNEL_ERROR("Could not add memory block\n");
                        KERNEL_PANIC(internal_err);
                    }
                }
            }

            break;
        }
        else if(range->base <= first_frame && range->limit > first_frame)
        {
            KERNEL_ERROR("Tried to free an already free block 0x%p\n",
                          first_frame);
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
                                     &internal_err);
        if(internal_err != OS_NO_ERR)
        {
            KERNEL_ERROR("Could not create queue node in memory manager\n");
            KERNEL_PANIC(internal_err);
        }

        internal_err = queue_push_prio(new_node, list, first_frame); 
        if(internal_err != OS_NO_ERR)
        {
            KERNEL_ERROR("Could not add memory block\n");
            KERNEL_PANIC(internal_err);
        }       
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

    if(curr_process == NULL)
    {
        KERNEL_ERROR("COW called when no process is running\n");
        KERNEL_PANIC(OS_ERR_UNAUTHORIZED_ACTION);
    }

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
            if(memory_get_ref_count(old_frame) == 0)
            {
                KERNEL_ERROR("Error in reference count management");
                KERNEL_PANIC(OS_ERR_OUT_OF_BOUND);
            }
            if(memory_get_ref_count(old_frame) > 1)
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
                if(err != OS_NO_ERR)
                {
                    KERNEL_ERROR("COW could not unmap temporary page\n");
                    KERNEL_PANIC(err);
                }
                memory_free_pages_to(curr_process->free_page_table, 
                                     tmp_page, 
                                     1, 
                                     NULL);

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

    /* Check the copy on write mechanism */
    if(memory_invocate_cow(fault_address) == OS_NO_ERR)
    {
        return;
    }

    /* Kernel cannot handle page fault at the moment */
    panic(cpu_state, int_id, stack_state);
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

static queue_t* paging_copy_free_page_table(OS_RETURN_E* err)
{
    OS_RETURN_E   internal_err;
    OS_RETURN_E   internal_err2;
    queue_t*      new_table;
    queue_t*      current_table;
    queue_node_t* cursor;
    queue_node_t* new_node;
    mem_range_t*  range;

    /* Create the new table */
    new_table = queue_create_queue(QUEUE_ALLOCATOR(kmalloc, kfree), 
                                   &internal_err);
    if(internal_err != OS_NO_ERR)
    {
        KERNEL_ERROR("Could not create new free page table list[%d]\n", 
                     internal_err);
        if(err != NULL)
        {
            *err = internal_err;
            return NULL;
        }
        else 
        {
            KERNEL_PANIC(internal_err);
        }
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
            internal_err = OS_ERR_MALLOC;
            if(err != NULL)
            {
                *err = internal_err;
                break;
            }
            else 
            {
                KERNEL_PANIC(internal_err);
            }
        }
        memcpy(range, cursor->data, sizeof(mem_range_t));
        new_node = queue_create_node(range, 
                                     QUEUE_ALLOCATOR(kmalloc, kfree), 
                                     &internal_err);
        if(internal_err != OS_NO_ERR)
        {
            KERNEL_ERROR("Could not create free page table node[%d]\n", 
                         internal_err);
            kfree(range);
            if(err != NULL)
            {
                *err = internal_err;
                break;
            }
            else 
            {
                KERNEL_PANIC(internal_err);
            }
        }

        /* Add range to list */
        internal_err = queue_push(new_node, new_table);
        if(internal_err != OS_NO_ERR)
        {
            KERNEL_ERROR("Could not push free page table node[%d]\n", 
                         internal_err);

            internal_err2 = queue_delete_node(&new_node);
            if(internal_err2 != OS_NO_ERR)
            {
                KERNEL_ERROR("Free page internal error\n");
                KERNEL_PANIC(internal_err2);
            }
            kfree(range);
            if(err != NULL)
            {
                *err = internal_err;
                break;
            }
            else 
            {
                KERNEL_PANIC(internal_err);
            }
        }

        /* Next entry */
        cursor = cursor->next;
    }

    /* Revert changes */
    if(internal_err != OS_NO_ERR)
    {
        cursor = queue_pop(new_table, &internal_err2);
        if(internal_err2 != OS_NO_ERR)
        {
            KERNEL_ERROR("Free page internal error\n");
            KERNEL_PANIC(internal_err2);
        }
        while (cursor != NULL)
        {
            kfree(cursor->data);
            internal_err2 = queue_delete_node(&cursor);
            if(internal_err2 != OS_NO_ERR)
            {
                KERNEL_ERROR("Free page internal error\n");
                KERNEL_PANIC(internal_err2);
            }
        }
        internal_err2 = queue_delete_queue(&new_table);
        if(internal_err2 != OS_NO_ERR)
        {
            KERNEL_ERROR("Free page internal error\n");
            KERNEL_PANIC(internal_err2);
        }

        return NULL;
    }

    if(err != NULL)
    {
        *err = OS_NO_ERR;
    }

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

    /* Check for existing mapping */
    if(is_mapped(virt_align, to_map))
    {
        KERNEL_ERROR("Trying to remap memory\n");
        if(err != NULL)
        {
            *err = OS_ERR_MAPPING_ALREADY_EXISTS;
            return;
        }
        else 
        {
            KERNEL_PANIC(OS_ERR_MAPPING_ALREADY_EXISTS);
        }
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
    if(err != OS_NO_ERR)
    {
        KERNEL_ERROR("Could not set page fault handler\n");
        KERNEL_PANIC(err);
    }
                                            
    /* Set CR3 register */
    __asm__ __volatile__("mov %%eax, %%cr3": :"a"((uintptr_t)kernel_pgdir -
                                                  KERNEL_MEM_OFFSET));

    KERNEL_DEBUG(MEMMGT_DEBUG_ENABLED, "[MEMMGT] Set new page directory and"
                                       " page fault handler");

    memory_paging_enable();

#ifdef TEST_MODE_ENABLED
    paging_test();
#endif
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

void memory_manager_init(void)
{
    queue_node_t* cursor;
    mem_range_t*  mem_range;

    (void)mem_range;

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

queue_t* memory_create_free_page_table(OS_RETURN_E* err)
{
    queue_t*      new_queue;
    queue_node_t* node;
    mem_range_t*  mem_range;
    OS_RETURN_E   internal_error;

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
        internal_error = queue_delete_queue(&new_queue);
        if(internal_error != OS_NO_ERR)
        {
            KERNEL_ERROR("Internal error while creating new page table\n");
            KERNEL_PANIC(internal_error);
        }
        KERNEL_ERROR("Could not allocate page range structure\n");
        *err = OS_ERR_MALLOC;
        return NULL;
    }
    node = queue_create_node(mem_range, 
                             QUEUE_ALLOCATOR(kmalloc, kfree), 
                             err);
    if(*err != OS_NO_ERR)
    {
        internal_error = queue_delete_queue(&new_queue);
        if(internal_error != OS_NO_ERR)
        {
            KERNEL_ERROR("Internal error while creating new page table\n");
            KERNEL_PANIC(internal_error);
        }
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
        internal_error = queue_delete_node(&node);
        internal_error |= queue_delete_queue(&new_queue);
        if(internal_error != OS_NO_ERR)
        {
            KERNEL_ERROR("Internal error while creating new page table\n");
            KERNEL_PANIC(internal_error);
        }
        kfree(mem_range);
        KERNEL_ERROR("Could not enqueue free pages node\n");
        return NULL;
    }

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
        KERNEL_ERROR("Cannot allocate stack when no process is running\n");
        if(err != NULL)
        {
            *err = OS_ERR_UNAUTHORIZED_ACTION;
            return NULL;
        }
        else 
        {
            KERNEL_PANIC(OS_ERR_UNAUTHORIZED_ACTION);
        }
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
    OS_RETURN_E err;

    ENTER_CRITICAL(int_state);

    memory_munmap(stack, stack_size, &err);

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
    uintptr_t*  new_pgdir_frame;
    uintptr_t*  new_pgdir_page;  
    uintptr_t*  new_pgtable_frame; 
    uintptr_t*  new_pgtable_page;
    uintptr_t*  current_pgdir;
    uintptr_t*  current_pgtable;
    uintptr_t*  new_data_page;
    uintptr_t*  new_data_frame;
    uintptr_t   curr_addr;
    uint16_t    pgtable_entry;
    uint32_t    i;
    uint32_t    j;
    OS_RETURN_E err;

    if(dst_process == NULL)
    {
        return OS_ERR_NULL_POINTER;
    }

    KERNEL_DEBUG(MEMMGT_DEBUG_ENABLED, "[MEMMGT] Copying process image");

    /* Create a new page directory and map for kernel */
    new_pgdir_frame = (uintptr_t*)memory_alloc_frames(1, &err);
    if(err != OS_NO_ERR)
    {
        KERNEL_ERROR("Could not copy process mapping\n");
        return err;
    }

    new_pgdir_page = (uintptr_t*)memory_alloc_pages_from(free_kernel_pages,
                                                         1, 
                                                         MEM_ALLOC_BEGINING,
                                                         &err);
    if(err != OS_NO_ERR)
    {
        memory_free_frames(new_pgdir_frame, 1, NULL);
        KERNEL_ERROR("Could not copy process mapping\n");
        return err;
    }

    memory_mmap_direct(new_pgdir_page, 
                       new_pgdir_frame, 
                       KERNEL_PAGE_SIZE, 
                       0, 
                       0, 
                       1,
                       0,
                       &err);
    if(err != OS_NO_ERR)
    {
        memory_free_pages_to(free_kernel_pages, new_pgdir_frame, 1, NULL);
        memory_free_frames(new_pgdir_frame, 1, NULL);
        KERNEL_ERROR("Could not copy process mapping\n");
        return err;
    }

    memory_acquire_ref((uintptr_t)new_pgdir_frame);

    /* Create temporary pages */
    new_pgtable_page = memory_alloc_pages_from(free_kernel_pages,
                                               1, 
                                               MEM_ALLOC_BEGINING,
                                               &err);
    if(err != OS_NO_ERR)
    {
        memory_munmap(new_pgdir_page, KERNEL_PAGE_SIZE, NULL);
        memory_free_frames(new_pgdir_frame, 1, NULL);
        memory_free_pages_to(free_kernel_pages, new_pgdir_frame, 1, NULL);
        KERNEL_ERROR("Could not copy process mapping\n");
        return err;
    }

    new_data_page    = memory_alloc_pages_from(free_kernel_pages, 
                                               1, 
                                               MEM_ALLOC_BEGINING,
                                               &err);

    if(err != OS_NO_ERR)
    {
        memory_free_pages_to(free_kernel_pages, new_pgtable_page, 1, NULL);
        memory_munmap(new_pgdir_page, KERNEL_PAGE_SIZE, NULL);
        memory_free_frames(new_pgdir_frame, 1, NULL);
        memory_free_pages_to(free_kernel_pages, new_pgdir_frame, 1, NULL);
        KERNEL_ERROR("Could not copy process mapping\n");
        return err;
    }

    /* The current page directory is always recursively mapped */
    current_pgdir = (uintptr_t*)&_KERNEL_RECUR_PG_DIR_BASE;

    /* Copy the page directory kernel entries, minus the recursive entry */    
    for(i = KERNEL_FIRST_PGDIR_ENTRY; i < KERNEL_PGDIR_SIZE - 1; ++i)
    {
        new_pgdir_page[i] = current_pgdir[i];
    }

    /* Set the recursive entry on the new page directory */
    new_pgdir_page[KERNEL_PGDIR_SIZE - 1] = (uintptr_t)new_pgdir_frame    |
                                            PG_DIR_FLAG_PAGE_SIZE_4KB     |
                                            PG_DIR_FLAG_PAGE_SUPER_ACCESS |
                                            PG_DIR_FLAG_PAGE_READ_WRITE   |
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
            
            memory_mmap_direct(new_pgtable_page, 
                               new_pgtable_frame, 
                               KERNEL_PAGE_SIZE, 
                               0, 
                               0,
                               1,
                               0,
                               &err);
            if(err != OS_NO_ERR)
            {
                memory_free_frames(new_pgtable_frame, 1, NULL);
                break;
            }

            new_pgdir_page[i] = (current_pgdir[i] & ~PG_ENTRY_ADDR_MASK) | 
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
                        new_pgtable_page[j] = current_pgtable[j];
                        /* Increment the reference count */
                        memory_acquire_ref(new_pgtable_page[j] & 
                                           PG_ENTRY_ADDR_MASK);
                    }
                    else if((current_pgtable[j] & PAGE_FLAG_OS_CUSTOM_MASK) != 
                            PAGE_FLAG_PRIVATE)
                    {
                        new_pgtable_page[j] = current_pgtable[j];
                        /* Increment the reference count */
                        memory_acquire_ref(new_pgtable_page[j] & 
                                           PG_ENTRY_ADDR_MASK);
                    }
                    else 
                    {
                        new_pgtable_page[j] = 0;
                    }
                }
                else 
                {
                    new_pgtable_page[j] = 0;
                }
            }

            memory_munmap(new_pgtable_page, KERNEL_PAGE_SIZE, NULL);
        }
        
    }

    /* TODO: put this in function: clean copy */
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
                                    (new_pgdir_page[i] & PG_ENTRY_ADDR_MASK);

                memory_mmap_direct(new_pgtable_page, 
                                   new_pgtable_frame, 
                                   KERNEL_PAGE_SIZE, 
                                   0, 
                                   0,
                                   1,
                                   0,
                                   NULL);

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
                            memory_release_ref(new_pgtable_page[j] & 
                                               PG_ENTRY_ADDR_MASK);
                        }
                        else if((current_pgtable[j] & 
                                 PAGE_FLAG_OS_CUSTOM_MASK) != 
                                PAGE_FLAG_PRIVATE)
                        {
                            /* Decrement the reference count */
                            memory_release_ref(new_pgtable_page[j] & 
                                               PG_ENTRY_ADDR_MASK);
                        }
                    }
                }
                memory_munmap(new_pgtable_page, KERNEL_PAGE_SIZE, NULL);
                memory_free_frames(new_pgtable_frame, 1, NULL);    
            }
        }

        memory_free_pages_to(free_kernel_pages, new_pgtable_page, 1, NULL);
        memory_munmap(new_pgdir_page, KERNEL_PAGE_SIZE, NULL);
        memory_free_frames(new_pgdir_frame, 1, NULL);
        memory_free_pages_to(free_kernel_pages, new_pgdir_frame, 1, NULL);
        KERNEL_ERROR("Could not copy process mapping\n");
        return err;
    }

    /* TODO: put this in function */
    /* Map and duplicate the kernel stack */
    curr_addr = (uintptr_t)kstack_addr;
    while(curr_addr < (uintptr_t)kstack_addr + kstack_size)
    {
        pgtable_entry = (curr_addr >> PG_TABLE_ENTRY_OFFSET) & 
                        PG_TABLE_ENTRY_OFFSET_MASK;

        new_pgtable_frame = (uintptr_t*)
                            new_pgdir_page[curr_addr >> PG_DIR_ENTRY_OFFSET];
        if(((uintptr_t)new_pgtable_frame & PG_DIR_FLAG_PAGE_PRESENT) == 0)
        {
            new_pgtable_frame = memory_alloc_frames(1, &err);
            if(err != OS_NO_ERR)
            {
                KERNEL_ERROR("Could not create new kstack page frame\n");
                break;
            }
            memory_acquire_ref((uintptr_t)new_pgtable_frame);
            new_pgdir_page[curr_addr >> PG_DIR_ENTRY_OFFSET] = 
                (uintptr_t)new_pgtable_frame  |
                PG_DIR_FLAG_PAGE_SIZE_4KB     |
                PG_DIR_FLAG_PAGE_SUPER_ACCESS |
                PG_DIR_FLAG_PAGE_READ_WRITE   |
                PG_DIR_FLAG_PAGE_PRESENT;
        }

        memory_mmap_direct(new_pgtable_page, 
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
            memory_munmap(new_pgtable_page, KERNEL_PAGE_SIZE, NULL);
            memory_free_frames(new_pgtable_frame, 1, NULL);
            break;
        }
        memory_acquire_ref((uintptr_t)new_data_frame);

        /* Map the new frame */
        memory_mmap_direct(new_data_page, 
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
            memory_munmap(new_pgtable_page, KERNEL_PAGE_SIZE, NULL);
            memory_free_frames(new_pgtable_frame, 1, NULL);
            memory_free_frames(new_data_frame, 1, NULL);
            break;
        }

        /* Copy data */
        memcpy(new_data_page, (void*)curr_addr, KERNEL_PAGE_SIZE);

        /* Unmap the new frame */
        memory_munmap(new_data_page, KERNEL_PAGE_SIZE, NULL);

        /* Update the mapping */
        new_pgtable_page[pgtable_entry] =
            (uintptr_t)new_data_frame |
            PAGE_FLAG_SUPER_ACCESS    | 
            PAGE_FLAG_READ_WRITE      |
            PAGE_FLAG_CACHE_WB        |
            PAGE_FLAG_PRIVATE         |
            PAGE_FLAG_PRESENT;

        memory_munmap(new_pgtable_page, KERNEL_PAGE_SIZE, NULL);

        curr_addr += KERNEL_PAGE_SIZE;
    }

    /* TODO: put this in function: clean stack */
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
                            new_pgdir_page[curr_addr >> PG_DIR_ENTRY_OFFSET];

            memory_mmap_direct(new_pgtable_page, 
                               new_pgtable_frame, 
                               KERNEL_PAGE_SIZE, 
                               0, 
                               0,
                               1,
                               0,
                               NULL);

            /* Allocate the new frame */
            new_data_frame = (uintptr_t*)(new_pgtable_page[pgtable_entry] & 
                                         PG_ENTRY_ADDR_MASK);
            memory_free_frames(new_data_frame, 1, NULL);

            /* Update the mapping */
            new_pgtable_page[pgtable_entry] = 0;
            new_pgdir_page[curr_addr >> PG_DIR_ENTRY_OFFSET] = 0;

            memory_munmap(new_pgtable_page, KERNEL_PAGE_SIZE, NULL);
            /* We don't free the pagtable frame, the clean copy will do it for 
             * us 
             */

            curr_addr -= KERNEL_PAGE_SIZE;
        }

        /* TODO: add call to clean copy */
    }


    /* Unmap the new page directory for the kernel */
    memory_munmap(new_pgdir_page, KERNEL_PAGE_SIZE, NULL);
    memory_free_pages_to(free_kernel_pages, new_pgdir_page, 1, NULL);     
    memory_free_pages_to(free_kernel_pages, new_pgtable_page, 1, NULL);
    memory_free_pages_to(free_kernel_pages, new_data_page, 1, NULL);   

    /* Set the destination process data */
    dst_process->page_dir = (uintptr_t)new_pgdir_frame;
    dst_process->free_page_table = paging_copy_free_page_table(&err);
    if(err != OS_NO_ERR)
    {
        /* TODO call clean stack */
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
            return (pgtable[pgtable_entry] & PG_ENTRY_ADDR_MASK);
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
                if(current_table[table_entry] != 0)
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

void memory_delete_free_page_table(queue_t* page_table)
{
    queue_node_t* mem_range;
    OS_RETURN_E   err;

    if(page_table == NULL)
    {
        return;
    }
    mem_range = queue_pop(page_table, &err);
    if(err != OS_NO_ERR)
    {
        KERNEL_ERROR("Error while cleaning page table\n");
        KERNEL_PANIC(err);
    }
    while(mem_range != NULL)
    {
        kfree(mem_range->data);
        err = queue_delete_node(&mem_range);
        if(err != OS_NO_ERR)
        {
            KERNEL_ERROR("Error while cleaning page table\n");
            KERNEL_PANIC(err);
        }
        
        mem_range = queue_pop(page_table, &err);
        if(err != OS_NO_ERR)
        {
            KERNEL_ERROR("Error while cleaning page table\n");
            KERNEL_PANIC(err);
        }
    }

    err = queue_delete_queue(&page_table);
    if(err != OS_NO_ERR)
    {
        KERNEL_ERROR("Error while cleaning page table\n");
        KERNEL_PANIC(err);
    }
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
    }

    memory_munmap(pgdir_page, KERNEL_PAGE_SIZE, NULL);

    memory_free_pages_to(free_kernel_pages, pgdir_page, 1, NULL);
    memory_free_pages_to(free_kernel_pages, pgtable_page, 1, NULL);
}

void memory_free_process_data(const void* virt_addr, 
                              const size_t size,
                              kernel_process_t* process)
{
    uintptr_t  current_addr;
    size_t     to_unmap;
    uint16_t   pgdir_entry;
    uint16_t   pgtable_entry;
    uintptr_t* pgdir_page;
    uintptr_t* pgtable_page;

    if(process == NULL)
    {
        KERNEL_ERROR("Cannot free process data of NULL process\n");
        KERNEL_PANIC(OS_ERR_NULL_POINTER);
    }
    if(process == sched_get_current_process())
    {
        KERNEL_ERROR("Cannot free process data of active process\n");
        KERNEL_PANIC(OS_ERR_NULL_POINTER);
    }

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
                       (void*)process->page_dir, 
                       KERNEL_PAGE_SIZE, 
                       0, 
                       0, 
                       1, 
                       0,
                       NULL);

    current_addr = (uintptr_t)virt_addr & ~(KERNEL_PAGE_SIZE - 1);
    to_unmap = size + (uintptr_t)virt_addr - current_addr;

    while(current_addr < (uintptr_t)virt_addr + to_unmap)
    {
        pgdir_entry   = current_addr >> PG_DIR_ENTRY_OFFSET;
        pgtable_entry = (current_addr >> PG_TABLE_ENTRY_OFFSET) & 
                        PG_TABLE_ENTRY_OFFSET_MASK;

        if((pgdir_page[pgdir_entry] & PG_DIR_FLAG_PAGE_PRESENT) == 0)
        {
            KERNEL_ERROR("Trying to free already unmapped data for process\n");
            KERNEL_PANIC(OS_ERR_UNAUTHORIZED_ACTION);
        }

        memory_mmap_direct(pgtable_page, 
                           (void*)(pgdir_page[pgdir_entry] & PG_ENTRY_ADDR_MASK), 
                           KERNEL_PAGE_SIZE, 
                           0, 
                           0, 
                           1, 
                           0,
                           NULL);

        if((pgtable_page[pgtable_entry] & PG_DIR_FLAG_PAGE_PRESENT) == 0)
        {
            KERNEL_ERROR("Trying to free already unmapped data for process\n");
            KERNEL_PANIC(OS_ERR_UNAUTHORIZED_ACTION);
        }

        /* This will free the frame is the reference count is 0 */
        memory_release_ref(pgtable_page[pgtable_entry] & PG_ENTRY_ADDR_MASK);
        pgtable_page[pgtable_entry] = 0;

        /* Free in page table */
        memory_free_pages_to(process->free_page_table, 
                             (uintptr_t*)current_addr, 
                             1,
                             NULL);

        memory_munmap(pgtable_page, KERNEL_PAGE_SIZE, NULL);

        current_addr += KERNEL_PAGE_SIZE;
    }

    memory_munmap(pgdir_page, KERNEL_PAGE_SIZE, NULL);

    memory_free_pages_to(free_kernel_pages, pgdir_page, 1, NULL);
    memory_free_pages_to(free_kernel_pages, pgtable_page, 1, NULL);
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