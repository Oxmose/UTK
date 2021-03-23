/*******************************************************************************
 * @file memmgt.h
 *
 * @see memmgt.c
 *
 * @author Alexy Torres Aurora Dugo
 *
 * @date 04/03/2021
 *
 * @version 1.0
 *
 * @brief Kernel memory manager.
 *
 * @details Kernel memory paging manager. This module allows to enable or
 * disable paging in the kernel. The memory mapping functions are also located
 * here. This module is used to detect the memory mapping of the system and 
 * manage physical memory.
 *
 * @copyright Alexy Torres Aurora Dugo
 ******************************************************************************/

#ifndef __CPU_MEMMGT_H_
#define __CPU_MEMMGT_H_

#include <stddef.h>         /* Standard definitions */
#include <stdint.h>         /* Generic int types */
#include <ctrl_block.h>     /* Kernel process structure */

/*******************************************************************************
 * CONSTANTS
 ******************************************************************************/

/* None */

/*******************************************************************************
 * STRUCTURES
 ******************************************************************************/

/**
 * @brief Page fault handler structure. Gathers the pafe fault addresses
 * associated with a corresponding handler.
 */
struct mem_handler
{
    /** @brief Start address of the range that is covered by the handler. */
    uintptr_t start;

    /** @brief End address of the range that is covered by the handler. */
    uintptr_t end;

    /** @brief Pointer to the handler function. */
    void (*handler)(uintptr_t fault_address);
};

/** @brief Shortcut for the struct mem_handler type. */
typedef struct mem_handler mem_handler_t;

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
 * FUNCTIONS
 ******************************************************************************/

/** 
 * @brief Inititalizes the kernel's memory manager.
 * 
 * @brief Initializes the kernel's memory manager while detecting the system's
 * memory organization.
 * 
 * @return The success state or the error code. 
 * - OS_NO_ERR is returned if no error is encountered. 
 * - No other return value is possible.
 */
OS_RETURN_E memory_manager_init(void);

/**
 * @brief Kernel memory frame allocation.
 * 
 * @details Kernel memory frame allocation. This method gets the desired number
 * of contiguous frames from the kernel frame pool and allocate them.
 * 
 * @param[in] frame_count The number of desired frames to allocate.
 * 
 * @return The address of the first frame of the contiguous block is 
 * returned.
 */
void* memory_alloc_frames(const size_t frame_count);

/**
 * @brief Kernel memory frame release.
 * 
 * @details Kernel memory frame release. This method releases the desired number
 * of contiguous frames to the kernel frame pool.
 * 
 * @param[in] frame_addr The address of the first frame to release.
 * @param[in] frame_count The number of desired frames to release.
 */
void memory_free_frames(void* frame_addr, const size_t frame_count);

/**
 * @brief Kernel memory page allocation.
 * 
 * @details Kernel memory page allocation. This method gets the desired number
 * of contiguous pages from the current process page pool and allocate them.
 * 
 * @param[in] page_count The number of desired pages to allocate.
 * @param[in] start_pt The starting point to allocated memory in the space.
 * 
 * @return The address of the first page of the contiguous block is 
 * returned.
 */
void* memory_alloc_pages(const size_t page_count, 
                         const MEM_ALLOC_START_E start_pt);

/**
 * @brief Kernel memory page release.
 * 
 * @details Kernel memory page release. This method releases the desired number
 * of contiguous pages to the current process page pool.
 * 
 * @param[in] page_addr The address of the first page to release.
 * @param[in] page_count The number of desired pages to release.
 */
void memory_free_pages(void* page_addr, const size_t page_count);

/**
 * @brief Returns a newly create free page table.
 * 
 * @details Returns a newly create free page table.
 * 
 * @param[out] err The error buffer to store the operation's result.
 * 
 * @return The free pages table is returned. 
 */
queue_t* memory_create_free_page_table(OS_RETURN_E* err);

/**
 * @brief Allocate a new stack in the free memory.
 * 
 * @details Allocate a new stack in the free memory. The address returned is the
 * begining of the stack (low address). The stack is also mapped in the memory.
 * 
 * @param[in] stack_size The size of the stack to allocate. Must be a multiple 
 * of the system's page size.
 * 
 * @return The address of the stack (low address) is returned on success. 
 * Otherwise NULL is returned.
 */
uintptr_t memory_alloc_stack(const size_t stack_size);

/**
 * @brief Release the meomry used by a stack.
 * 
 * @details Release the meomry used by a stack. 
 * 
 * @param[in] virt_addr The address of the stack to free.
 * @param[in] stack_size The size of the stack.
 */
void memory_free_stack(uintptr_t virt_addr, const size_t stack_size);

/**
 * @brief Enables paging.
 *
 * @details Enables paging. The CR0 register is modified to enable paging.
 *
 * @warning CR3 register must be set before calling this function.
 *
 * @return The success state or the error code.
 * - OS_NO_ERR is returned if no error is encountered.
 * - OS_ERR_PAGING_NOT_INIT is returned if paging has not been initialized.
 */
OS_RETURN_E memory_paging_enable(void);

/**
 * @brief Disables paging.
 *
 * @details Disables paging. The CR0 register is modified to disable paging.
 *
 * @return The success state or the error code.
 * - OS_NO_ERR is returned if no error is encountered.
 * - OS_ERR_PAGING_NOT_INIT is returned if paging has not been initialized.
 */
OS_RETURN_E memory_paging_disable(void);

/**
 * @brief Maps a virtual memory region to a memory frame.
 *
 * @details Maps a virtual memory region to a memory frame. The function will 
 * not check any address boundaries. If the page is already mapped a panic is 
 * raised. A new frame is allocated to map the memory.
 *
 * @param[in] virt_addr The virtual address to map.
 * @param[in] mapping_size The size of the region to map.
 * @param[in] read_only Sets the read only flag.
 * @param[in] exec Sets the executable flag.
 */
void memory_mmap(const void* virt_addr, 
                  const size_t mapping_size,
                  const uint8_t read_only,
                  const uint8_t exec);

/**
 * @brief Maps a virtual memory region to a memory regions.
 *
 * @details Maps a kernel virtual memory region to a memory region. The function 
 * will not check any address boundaries. If the page is already mapped a panic 
 * is raised. Mapping disables cache for hardware mapped memory.
 *
 * @param[in] virt_addr The virtual address to map.
 * @param[in] phys_addr The physical address to map.
 * @param[in] mapping_size The size of the region to map.
 * @param[in] read_only Sets the read only flag.
 * @param[in] exec Sets the executable flag.
 * @param[in] is_hw Tells if the mapping is a memory mapped hardware.
 */
void memory_mmap_direct(const void* virt_addr,
                         const void* phys_addr,
                         const size_t mapping_size,
                         const uint8_t read_only,
                         const uint8_t exec,
                         const uint8_t is_hw);

/**
 * @brief Un-maps a kernel virtual memory region from a corresponding physical
 * region.
 *
 * @details Un-maps a kernel virtual memory region from a corresponding physical
 * region.
 *
 * @param[in] virt_addr The virtual address to map.
 * @param[in] mapping_size The size of the region to map.
 */
void memory_munmap(const void* virt_addr, const size_t mapping_size);

/** 
 * @brief Copies the current process memory image mapping.
 * 
 * @details Copies the current process memory image mapping. This function 
 * copies the page memory image of the current process, copies the free page 
 * table of the source process, assign a new page directory to the new process 
 * and updates its translation. The current process's data are marked as copy
 * on write for the current and destination process.
 * 
 * @param[out] dst_process The process that will receive the image copy.
 * 
 * @return OS_NO_ERR is returned on success. Otherwise an error code is 
 * returned.
 */
OS_RETURN_E memory_copy_self_mapping(kernel_process_t* dst_process);

/**
 * @brief Returns the physical address associated to the virtual address given
 * as parameter.
 * 
 * @details Returns the physical address associated to the virtual address given
 * as parameter. NULL is retuend if not found.
 * 
 * @param[in] virt_addr The virtual address to lookup.
 * 
 * @return The physical address associated to the virtual address given
 * as parameter.
 */
uintptr_t memory_get_phys_addr(const uintptr_t virt_addr);

/**
 * ­@brief Declares a hardware region.
 * 
 * @details Declares a hardware region. This regions is added to the frame
 * reference table.
 * 
 * @param[in] phys_addr The physical address of the hardware.
 * @param[in] size The size of the region to add.
 * 
 * @return OS_NO_ERR is returned in case of success. Otherwise an error is 
 * returned.
 */
OS_RETURN_E memory_declare_hw(const uintptr_t phys_addr, const size_t size);


#endif /* #ifndef __CPU_MEMMGT_H_ */