/******************************************************************************
 * @file arch_paging.h
 *
 * @author Alexy Torres Aurora Dugo
 *
 * @date 25/01/2018
 *
 * @version 1.0
 *
 * @brief i386 kernel memory paging informations.
 *
 * @details i386 kernel memory paging informations and structures.
 * 
 * @copyright Alexy Torres Aurora Dugo
 ******************************************************************************/

#ifndef __I386_ARCH_PAGING_H_
#define __I386_ARCH_PAGING_H_

/*******************************************************************************
 * CONSTANTS
 ******************************************************************************/

/** @brief Kernel's page size in Bytes. */
#define KERNEL_PAGE_SIZE 4096

/** @brief i386 maximum usable address. */
#define ARCH_MAX_ADDRESS 0xFFFFFFFF

/*******************************************************************************
 * STRUCTURES
 ******************************************************************************/

/* None */

/*******************************************************************************
 * FUNCTIONS
 ******************************************************************************/

/* None */

#endif /* #ifndef __I386_ARCH_PAGING_H_ */