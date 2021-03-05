/*******************************************************************************
 * @file paging.h
 *
 * @see paging.c
 *
 * @author Alexy Torres Aurora Dugo
 *
 * @date 04/03/2021
 *
 * @version 1.0
 *
 * @brief Kernel memory paging manager.
 *
 * @details Kernel memory paging manager. This module allows to enable or
 * disable paging in the kernel. The memory mapping functions are also located
 * here. 
 *
 * @copyright Alexy Torres Aurora Dugo
 ******************************************************************************/

#include <stddef.h>               /* Standard definitions */
#include <stdint.h>               /* Generic int types */
#include <memmgt.h>               /* Memory manager */
#include <arch_paging.h>          /* Paging information */
#include <exceptions.h>           /* Exception management */
#include <interrupt_settings.h>   /* Interrupt settings */
#include <panic.h>                /* Kernel panic */
#include <kernel_output.h>        /* Kernel output methods */
#include <critical.h>             /* Critical sections */
#include <x86memmgt.h>            /* X86 memory management */

#ifdef TEST_MODE_ENABLED
#include <test_bank.h>
#endif

/* UTK configuration file */
#include <config.h>

/* Header file */
#include <paging.h>

/*******************************************************************************
 * GLOBAL VARIABLES
 ******************************************************************************/

/** @brief Kernel page directory array. */
static uintptr_t kernel_pgdir[KERNEL_PGDIR_SIZE] __attribute__((aligned(4096)));

/** @brief Kernel reserved page tables. */
static uintptr_t min_pgtable[KERNEL_RESERVED_PAGING][KERNEL_PGDIR_SIZE]
                                                __attribute__((aligned(4096)));

/** @brief Tells if paging is initialized. */
static uint8_t init = 0;

/** @brief Tells if paging is enabled. */
static uint8_t enabled = 0;

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

    KERNEL_DEBUG("Mapping kernel section at 0x%p -> 0x%p\n", 
                 start_addr, end_addr);
    while(start_addr < end_addr)
    {
        /* Get entry indexes */
        pg_dir_entry      = (uintptr_t)start_addr >> PG_DIR_OFFSET;
        pg_table_entry    = ((uintptr_t)start_addr >> PG_TABLE_OFFSET) & 0x3FF;
        min_pgtable_entry = (((uintptr_t)start_addr - KERNEL_MEM_OFFSET) >> 
                            PG_DIR_OFFSET) & 0x3FF;
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
        kernel_error("Page fault handler in wrong exception line.\n");
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
    KERNEL_DEBUG("Page fault at 0x%p\n", fault_address);

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
static uint32_t is_mapped(const uintptr_t start_addr, const size_t size)
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
        pgtable_entry = (start_align >> PG_TABLE_OFFSET) & 0x3FF;

        /* Check page directory presence and allocate if not present */
        pgdir_rec_addr = (uint32_t*)PAGING_RECUR_PG_DIR;
        if((pgdir_rec_addr[pgdir_entry] & PG_DIR_FLAG_PAGE_PRESENT) != 0)
        {
             /* Check present in page table */
            pgtable = (uint32_t*)(pgdir_rec_addr[pgdir_entry] & PG_ENTRY_MASK);
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
static OS_RETURN_E kernel_mmap_internal(const void* virt_addr,
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
    OS_RETURN_E err;
    uint32_t    i;

    /* Align addresses */
    virt_align = (uintptr_t)virt_addr & PAGE_ALIGN_MASK;
    phys_align = (uintptr_t)phys_addr & PAGE_ALIGN_MASK;

    /* Get mapping size */
    to_map = mapping_size + ((uintptr_t)virt_addr - virt_align);

    /* Check for existing mapping */
    if(is_mapped(virt_align, to_map))
    {
        return OS_ERR_MAPPING_ALREADY_EXISTS;
    }

    err = OS_NO_ERR;

    while(to_map)
    {
        /* Get entries */
        pgdir_entry   = (virt_align >> PG_DIR_OFFSET);
        pgtable_entry = (virt_align >> PG_TABLE_OFFSET) & 0x3FF;

        /* Check page directory presence and allocate if not present */
        pgdir_rec_addr = (uint32_t*)PAGING_RECUR_PG_DIR;
        if((pgdir_rec_addr[pgdir_entry] & PG_DIR_FLAG_PAGE_PRESENT) == 0)
        {
            pgtable = alloc_kframes(1, &err);
            if(err != OS_NO_ERR)
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
            pgtable = (uint32_t*)(pgdir_rec_addr[pgdir_entry] & PG_ENTRY_MASK);

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

        KERNEL_DEBUG("Mapped page at 0x%p -> 0x%p\n", virt_align, phys_align);

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

    return err;
}

OS_RETURN_E paging_init(void)
{
    uint32_t    i;
    uintptr_t   start_addr;
    uintptr_t   end_addr;
    OS_RETURN_E err;

    err = OS_NO_ERR;

    KERNEL_DEBUG("Initializing paging\n");

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
    paging_enable();

#ifdef TEST_MODE_ENABLED
    paging_test();
#endif

    return err;
}

OS_RETURN_E paging_enable(void)
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

    /* Enable paging and write protect */
    __asm__ __volatile__("mov %%cr0, %%eax\n\t"
                         "or $0x80010000, %%eax\n\t"
                         "mov %%eax, %%cr0" : : : "eax");

    KERNEL_DEBUG("Paging enabled\n");

    enabled = 1;

    EXIT_CRITICAL(int_state);

    return OS_NO_ERR;
}

OS_RETURN_E paging_disable(void)
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

    KERNEL_DEBUG("Paging disabled\n");

    enabled = 0;

    EXIT_CRITICAL(int_state);

    return OS_NO_ERR;
}

OS_RETURN_E paging_kmmap_hw(const void* virt_addr,
                            const void* phys_addr,
                            const size_t mapping_size,
                            const uint8_t read_only,
                            const uint8_t exec)
{
    uint32_t    int_state;
    OS_RETURN_E err;

    ENTER_CRITICAL(int_state);

    KERNEL_DEBUG("Request HW mappping at 0x%p -> 0x%p (%uB)\n", 
                 virt_addr, 
                 phys_addr,
                 mapping_size);
    err = kernel_mmap_internal(virt_addr, phys_addr, 
                               mapping_size, read_only, exec, 0, 1);

    EXIT_CRITICAL(int_state);
    return err;
}

OS_RETURN_E paging_kmmap(const void* virt_addr, 
                         const size_t mapping_size,
                         const uint8_t read_only,
                         const uint8_t exec)
{
    void*       frames;
    uintptr_t   end_map;
    uintptr_t   start_map;
    size_t      page_count;
    OS_RETURN_E err;
    uint32_t    int_state;

    /* Compute physical memory size */
    end_map   = (uintptr_t)virt_addr + mapping_size;
    start_map = (uintptr_t)virt_addr & PAGE_ALIGN_MASK;

    if(end_map % KERNEL_PAGE_SIZE)
    {
        end_map &= PAGE_ALIGN_MASK;
        end_map += KERNEL_PAGE_SIZE;
    }
    page_count = (end_map - start_map) / KERNEL_PAGE_SIZE;

    ENTER_CRITICAL(int_state);

    /* Get a physical frame block */
    frames = alloc_kframes(page_count, &err);
    if(err != OS_NO_ERR)
    {
        EXIT_CRITICAL(int_state);
        return err;
    }

    KERNEL_DEBUG("Request regular mappping at 0x%p -> 0x%p (%uB)\n",
                 virt_addr, 
                 frames,
                 mapping_size);

    err = kernel_mmap_internal(virt_addr, frames, 
                               mapping_size, read_only, exec, 1, 0);
    if(err != OS_NO_ERR)
    {
        /* Free allocated frames */
        free_kframes(frames, page_count);
    }
    
    EXIT_CRITICAL(int_state);
    return err;
}

OS_RETURN_E paging_kmunmap(const void* virt_addr, const size_t mapping_size)
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

    KERNEL_DEBUG("Request unmappping at 0x%p (%uB)\n",
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
        pgtable_entry = (start_map >> PG_TABLE_OFFSET) & 0x3FF;

        /* Check page directory presence and allocate if not present */
        pgdir_rec_addr = (uint32_t*)PAGING_RECUR_PG_DIR;
        if((pgdir_rec_addr[pgdir_entry] & PG_DIR_FLAG_PAGE_PRESENT) != 0)
        {
            pgtable = (uint32_t*)(pgdir_rec_addr[pgdir_entry] & PG_ENTRY_MASK);

            /* Get recursive virtual address */
            pgtable = (uint32_t*)(PAGING_RECUR_PG_TABLE + 
                                  KERNEL_PAGE_SIZE * 
                                  pgdir_entry);

            if((pgtable[pgtable_entry] & PAGE_FLAG_PRESENT) != 0)
            {
                /* Check if it was hardware mapping, release otherwise */
                if((pgtable[pgtable_entry] & PAGE_FLAG_HARDWARE) == 0)
                {
                    free_kframes(
                        (void*)(pgtable[pgtable_entry] & PG_ENTRY_MASK), 1);
                }
                /* Unmap */
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
                free_kframes(
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
    return OS_NO_ERR;
}