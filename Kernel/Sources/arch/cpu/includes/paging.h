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

#ifndef __CPU_PAGING_H_
#define __CPU_PAGING_H_

#include <stddef.h>         /* Standard definitions */
#include <stdint.h>         /* Generic int types */

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
 * 
 * @warning This function assumes the kernel is setup with basic paging.
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
OS_RETURN_E paging_kmmap(const void* virt_addr, 
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
OS_RETURN_E paging_kmmap_hw(const void* virt_addr, 
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
OS_RETURN_E paging_kmunmap(const void* virt_addr, const size_t mapping_size);

#endif /* #ifndef __CPU_PAGING_H_ */