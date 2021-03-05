/*******************************************************************************
 * @file x86memmgt.h
 * 
 * @see x86memmgt.c
 *
 * @author Alexy Torres Aurora Dugo
 *
 * @date 05/03/2021
 *
 * @version 1.0
 *
 * @brief x86 specific kernel physical memory manager.
 *
 * @details This module is used to detect the memory mapping of the system and 
 * manage physical memory for x86 architecture
 * 
 * @copyright Alexy Torres Aurora Dugo
 ******************************************************************************/

#ifndef __BSP_X86_MEMMGT_H_
#define __BSP_X86_MEMMGT_H_

#include <stddef.h> /* Standard definition */
#include <stdint.h> /* Generic int types */

/*******************************************************************************
 * CONSTANTS
 ******************************************************************************/

/*******************************************************************************
* STRUCTURES
******************************************************************************/

/*******************************************************************************
* FUNCTIONS
******************************************************************************/

/**
 * @brief Retrieves the start and end address of the kernel low startup section.
 * 
 * @details Retrieves the start and end address of the kernel low startup 
 * section. The addresses are stored in the buffer given as parameter. The 
 * function has no effect if the buffer are NULL.
 * 
 * @param[out] start The start address of the section.
 * @param[out] end The end address of the section.
 */
void memory_get_klowstartup_range(uintptr_t* start, uintptr_t* end);

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
void memory_get_khighstartup_range(uintptr_t* start, uintptr_t* end);

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
void memory_get_ktext_range(uintptr_t* start, uintptr_t* end);

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
void memory_get_krodata_range(uintptr_t* start, uintptr_t* end);

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
void memory_get_kdata_range(uintptr_t* start, uintptr_t* end);

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
void memory_get_kbss_range(uintptr_t* start, uintptr_t* end);

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
void memory_get_kstacks_range(uintptr_t* start, uintptr_t* end);

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
void memory_get_kheap_range(uintptr_t* start, uintptr_t* end);

#endif /* #ifndef __BSP_X86_MEMMGT_H_ */