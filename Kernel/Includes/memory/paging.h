/*******************************************************************************
 * @file paging.h
 *
 * @see paging.c
 *
 * @author Alexy Torres Aurora Dugo
 *
 * @date 25/01/2018
 *
 * @version 1.0
 *
 * @brief Kernel memory paging manager.
 *
 * @details Kernel memory paging manager. This module allows to enable or
 * disable paging in the kernel. The memory mapping functions are also located
 * here. The module also defines the page's size.
 *
 * @copyright Alexy Torres Aurora Dugo
 ******************************************************************************/

#ifndef __MEMORY_PAGING_H_
#define __MEMORY_PAGING_H_

#include <lib/stddef.h>         /* Standard definitions */
#include <lib/stdint.h>         /* Generic int types */

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

    /** @brief Link to the next node in the linked list. */
    struct mem_handler* next;
};

/** @brief Shortcut for the struct mem_handler type. */
typedef struct mem_handler mem_handler_t;

/*******************************************************************************
 * FUNCTIONS
 ******************************************************************************/

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
 */
OS_RETURN_E paging_init(void);

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
OS_RETURN_E paging_enable(void);

/**
 * @brief Disables paging.
 *
 * @details Disables paging. The CR0 register is modified to disable paging.
 *
 * @return The success state or the error code.
 * - OS_NO_ERR is returned if no error is encountered.
 * - OS_ERR_PAGING_NOT_INIT is returned if paging has not been initialized.
 */
OS_RETURN_E paging_disable(void);

/**
 * @brief Maps a kernel virtual memory region to a free physical region.
 *
 * @details Maps a kernel virtual memory region to a free physical
 * region. The function will not check any address boundaries. If the page is
 * already mapped and the allow_remap argument is set to 0 an error will be
 * returned.
 *
 * @param[in] virt_addr The virtual address to map.
 * @param[in] mapping_size The size of the region to map.
 * @param[in] read_only Sets the read only flag.
 * @param[in] exec Sets the executable flag.
 *
 * @return The success state or the error code.
 * - OS_NO_ERR is returned if no error is encountered.
 * - OS_ERR_PAGING_NOT_INIT is returned if paging has not been initialized.
 * - OS_ERR_MAPPING_ALREADY_EXISTS is returned if the page is already mapped.
 */
OS_RETURN_E kernel_mmap(const void* virt_addr, 
                        const size_t mapping_size,
                        const uint8_t read_only,
                        const uint8_t exec);

/**
 * @brief Maps a kernel virtual memory region to a memory mapped hardware.
 *
 * @details Maps a kernel virtual memory region to a memory mapped hardware
 * region. The function will not check any address boundaries. If the page is
 * already mapped and the allow_remap argument is set to 0 an error will be
 * returned.
 *
 * @param[in] virt_addr The virtual address to map.
 * @param[in] phys_addr The physical address to map.
 * @param[in] mapping_size The size of the region to map.
 * @param[in] read_only Sets the read only flag.
 * @param[in] exec Sets the executable flag.
 *
 * @return The success state or the error code.
 * - OS_NO_ERR is returned if no error is encountered.
 * - OS_ERR_PAGING_NOT_INIT is returned if paging has not been initialized.
 * - OS_ERR_MAPPING_ALREADY_EXISTS is returned if the page is already mapped.
 */
OS_RETURN_E kernel_mmap_hw(const void* virt_addr, 
                           const void* phys_addr,
                           const size_t mapping_size,
                           const uint8_t read_only,
                           const uint8_t exec);

/**
 * @brief Un-maps a kernel virtual memory region from a corresponding physical
 * region.
 *
 * @details Un-maps a kernel virtual memory region from a corresponding physical
 * region.
 *
 * @param[in] virt_addr The virtual address to map.
 * @param[in] mapping_size The size of the region to map.
 *
 * @return The success state or the error code.
 * - OS_NO_ERR is returned if no error is encountered.
 * - OS_ERR_PAGING_NOT_INIT is returned if paging has not been initialized.
 * - OS_ERR_MEMORY_NOT_MAPPED is returned if the page is not mapped.
 */
OS_RETURN_E kernel_munmap(const void* virt_addr, const size_t mapping_size);

/**
 * @brief Registers a page fault handler for the required address range.
 * 
 * @details Registers a page fault handler for the required address range.
 * The handler will be called when a page fault occurs in this range.
 * 
 * @param[in] handler The handler to call on page fault.
 * @param[in] range_start The range base address to consider.
 * @param[in] range_end The range end address to consider.
 * 
 * @return The success state or the error code.
 * - OS_NO_ERR is returned if no error is encountered.
 * - OS_ERR_HANDLER_ALREADY_EXISTS is returned if an handler already exists
 * in the specified range.
 * - OS_ERR_NULL_POINTER is returned if the handler is NULL.
 * - OS_ERR_UNAUTHORIZED_ACTION if the start address is greater or equal to
 * the end address.
 * - OS_ERR_MALLOC if an error occurs while allocating memory.
 */
OS_RETURN_E paging_register_fault_handler(void (*handler)(const uintptr_t),
                                          const uintptr_t range_start,
                                          const uintptr_t range_end);

/**
 * @brief Returns the page fault handlers list.
 * 
 * @details Returns the page fault handlers list.
 * 
 * @return The page fault handlers list is returned.
 */
const mem_handler_t* paging_get_handler_list(void);

#endif /* #ifndef __MEMORY_PAGING_H_ */