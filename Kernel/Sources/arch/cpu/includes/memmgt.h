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

#include <kernel_error.h>    /* Kernel error codes */
#include <stdint.h>          /* Generic int types */
#include <ctrl_block.h>      /* Kernel process structure */
#include <sys/syscall_api.h> /* System call API */

/*******************************************************************************
 * CONSTANTS
 ******************************************************************************/

/* None */

/*******************************************************************************
 * STRUCTURES
 ******************************************************************************/

/* None */

/*******************************************************************************
 * FUNCTIONS
 ******************************************************************************/

/** 
 * @brief Inititalizes the kernel's memory manager.
 * 
 * @brief Initializes the kernel's memory manager while detecting the system's
 * memory organization.
 */
void memory_manager_init(void);

/**
 * @brief Returns a newly created free page table.
 * 
 * @details Returns a newly created free page table.
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
 * @param[in] is_kernel Tells if the stack is a kernel stack or not.
 * @param[out] err The error buffer to store the operation's result.
 * 
 * @return The address of the stack (low address) is returned on success. 
 * Otherwise NULL is returned.
 */
void* memory_alloc_stack(const size_t stack_size, 
                         const bool_t is_kernel,
                         OS_RETURN_E* err);

/**
 * @brief Release the memory used by a stack.
 * 
 * @details Release the memory used by a stack. 
 * 
 * @param[in] virt_addr The address of the stack to free.
 * @param[in] stack_size The size of the stack.
 * 
 * @return Returns the success or error state.
 */
OS_RETURN_E memory_free_stack(void* virt_addr, const size_t stack_size);

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
 * @param[out] err The error buffer to store the operation's result. If NULL, 
 * the function will raise a kernel panic in case of error.
 */
void memory_mmap(const void* virt_addr, 
                  const size_t mapping_size,
                  const bool_t read_only,
                  const bool_t exec,
                  OS_RETURN_E* err);

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
 * @param[in] cache_enable Tells if cache is enabled for this page.
 * @param[in] is_hw Tells if the mapping is a memory mapped hardware.
 * @param[out] err The error buffer to store the operation's result. If NULL, 
 * the function will raise a kernel panic in case of error.
 */
void memory_mmap_direct(const void* virt_addr,
                         const void* phys_addr,
                         const size_t mapping_size,
                         const bool_t read_only,
                         const bool_t exec,
                         const bool_t cache_enable,
                         const bool_t is_hw,
                         OS_RETURN_E* err);

/**
 * @brief Un-maps a kernel virtual memory region from a corresponding physical
 * region.
 *
 * @details Un-maps a kernel virtual memory region from a corresponding physical
 * region.
 *
 * @param[in, out] virt_addr The virtual address to map.
 * @param[in] mapping_size The size of the region to map.
 * @param[out] err The error buffer to store the operation's result. If NULL, 
 * the function will raise a kernel panic in case of error.
 */
void memory_munmap(void* virt_addr, 
                   const size_t mapping_size, 
                   OS_RETURN_E* err);

/** 
 * @brief Copies the current process memory image mapping.
 * 
 * @details Copies the current process memory image mapping. This function 
 * copies the page memory image of the current process, copies the free page 
 * table of the source process, assign a new page directory to the new process 
 * and updates its translation. The current process's data are marked as copy
 * on write for the current and destination process.
 * The function duplicates the main kernel stack used by the process to allow
 * the kernel stack to be usable directly after the copy. This region will not
 * be marked as copy on write.
 * 
 * @param[out] dst_process The process that will receive the image copy.
 * @param[in] kstack The kernel stack address of the main process.
 * @param[in] kstack_size The kernel stack size.
 * 
 * @return OS_NO_ERR is returned on success. Otherwise an error code is 
 * returned.
 */
OS_RETURN_E memory_copy_self_mapping(kernel_process_t* dst_process,
                                     const void* kstack,
                                     const size_t kstack_size);

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

/**
 * @brief Release the free page table memory.
 * 
 * @details Release the page table memory. Clear all internal structure of the 
 * free page table queue and delete the queue.
 * 
 * @param[out] page_table The free page table to release.
 */
void memory_delete_free_page_table(queue_t* page_table);

/**
 * @brief Release all the physical memory owned by a page directory. 
 * 
 * @details Release all the physical memory used by a process. This function 
 * walks the page directory and release memory frames and hardware. The page
 *  directory itself is removed from memory.
 * 
 * @param[in] pd_dir The page directory to clean.
 */
void memory_clean_process_memory(uintptr_t pg_dir);

/**
 * @brief Releases a process memory region.
 * 
 * @details Releases a process memory region. The function directly modifies
 * the process page tables and the process free page table.
 * 
 * @param[in] virt_addr The start address of the memory to release.
 * @param[in] size The size of the region to release.
 * @param[in] process The process for which we want to release the memory.
 * 
 * @warning This function should not be used to release memory of the current
 * process.
 */
void memory_free_process_data(const void* virt_addr, 
                              const size_t size,
                              kernel_process_t* process);

/**
 * @brief Kernel memory page allocation.
 * 
 * @details Kernel memory page allocation. This method allocates the desired 
 * number of contiguous pages to the kernel pages pool.
 * 
 * @param[in] page_count The number of desired pages to allocate.
 * @param[out] err The error buffer to store the operation's result. If NULL, 
 * the function will raise a kernel panic in case of error.
 * 
 * @return The function return the address of the first allocated page.
 */
void* memory_alloc_kernel_pages(const size_t page_count, OS_RETURN_E* err);

/**
 * @brief Kernel memory page release.
 * 
 * @details Kernel memory page release. This method releases the desired number
 * of contiguous pages to the kernel pages pool.
 * 
 * @param[in] page_addr The address of the first page to release.
 * @param[in] page_count The number of desired pages to release.
 * @param[out] err The error buffer to store the operation's result. If NULL, 
 * the function will raise a kernel panic in case of error.
 */
void memory_free_kernel_pages(const void* page_addr, 
                              const size_t page_count, 
                              OS_RETURN_E* err);

/**
 * @brief System call handler to allocate memory pages.
 * 
 * @details System call handler to  allocate memory pages. This 
 * system call uses as memmgt_page_alloc_param_t sctructure given as parameter.
 * 
 * @param[in] func The syscall function ID, must correspond to the alloc_pages
 * call.
 * @param[in, out] params The parameters used by the function, must be of type 
 * memmgt_page_alloc_param_t.
 */
void memory_alloc_page(const SYSCALL_FUNCTION_E func, void* params);

/**
 * @brief @brief Returns the amount of free pages memory (in bytes) for the 
 * kernel.
 * 
 * @return The amount of free pages memory (in bytes) for the 
 * kernel. 
 */
uint32_t memory_get_free_kpages(void);

/**
 * @brief @brief Returns the amount of free pages memory (in bytes) for the 
 * current process.
 * 
 * @return The amount of free pages memory (in bytes) for the 
 * current process. 
 */
uint32_t memory_get_free_pages(void);

/**
 * @brief @brief Returns the amount of free frames memory (in bytes).
 * 
 * @return the amount of free frames memory (in bytes).
 */
uint32_t memory_get_free_frames(void);

#endif /* #ifndef __CPU_MEMMGT_H_ */