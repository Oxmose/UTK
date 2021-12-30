/******************************************************************************
 * @file arch_memmgt.h
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

#ifndef __I386_ARCH_MEMMGT_H_
#define __I386_ARCH_MEMMGT_H_

/*******************************************************************************
 * INCLUDES
 ******************************************************************************/

/* None */

/*******************************************************************************
 * CONSTANTS
 ******************************************************************************/

/** @brief Defines the kernel frame size */
#define KERNEL_FRAME_SIZE 0x1000

/** @brief Defines the kernel page size */
#define KERNEL_PAGE_SIZE KERNEL_FRAME_SIZE

/** @brief Page alignement bitmask */
#define PAGE_ALIGN_MASK (~(KERNEL_PAGE_SIZE - 1))

/** @brief Kernel's page directory entry count. */
#define KERNEL_PGDIR_SIZE 1024

/** @brief Kernel page address mask. */
#define PG_ENTRY_ADDR_MASK 0xFFFFF000

/** @brief Kernel page directory entry offset. */
#define PG_DIR_ENTRY_OFFSET   22

/** @brief Kernel page table offset. */
#define PG_TABLE_ENTRY_OFFSET 12

/** @brief Kernel page table mask */
#define PG_TABLE_ENTRY_OFFSET_MASK 0x3FF

/** @brief Architecture maximal address. */
#define ARCH_MAX_ADDRESS 0xFFFFFFFF

/** @brief Page directory flag: 4Kb page size. */
#define PG_DIR_FLAG_PAGE_SIZE_4KB       0x00000000
/** @brief Page directory flag: 4Mb page size. */
#define PG_DIR_FLAG_PAGE_SIZE_4MB       0x00000080
/** @brief Page directory flag: page accessed. */
#define PG_DIR_FLAG_PAGE_ACCESSED       0x00000020
/** @brief Page directory flag: cache disabled. */
#define PG_DIR_FLAG_PAGE_CACHE_DISABLED 0x00000010
/** @brief Page directory flag: cache write policy set to write through. */
#define PG_DIR_FLAG_PAGE_CACHE_WT       0x00000008
/** @brief Page directory flag: cache write policy set to write back. */
#define PG_DIR_FLAG_PAGE_CACHE_WB       0x00000000
/** @brief Page directory flag: access permission set to user. */
#define PG_DIR_FLAG_PAGE_USER_ACCESS    0x00000004
/** @brief Page directory flag: access permission set to kernel. */
#define PG_DIR_FLAG_PAGE_SUPER_ACCESS   0x00000000
/** @brief Page directory flag: access permission set to read and write. */
#define PG_DIR_FLAG_PAGE_READ_WRITE     0x00000002
/** @brief Page directory flag: access permission set to read only. */
#define PG_DIR_FLAG_PAGE_READ_ONLY      0x00000000
/** @brief Page directory flag: page table present. */
#define PG_DIR_FLAG_PAGE_PRESENT        0x00000001

/** @brief Page flag: global page. */
#define PAGE_FLAG_GLOBAL         0x00000100
/** @brief Page flag: page dirty. */
#define PAGE_FLAG_DIRTY          0x00000080
/** @brief Page flag: page accessed. */
#define PAGE_FLAG_ACCESSED       0x00000020
/** @brief Page flag: cache disabled for the page. */
#define PAGE_FLAG_CACHE_DISABLED 0x00000010
/** @brief Page flag: cache write policy set to write through. */
#define PAGE_FLAG_CACHE_WT       0x00000008
/** @brief Page flag: cache write policy set to write back. */
#define PAGE_FLAG_CACHE_WB       0x00000000
/** @brief Page flag: access permission set to user. */
#define PAGE_FLAG_USER_ACCESS    0x00000004
/** @brief Page flag: access permission set to kernel. */
#define PAGE_FLAG_SUPER_ACCESS   0x00000000
/** @brief Page flag: access permission set to read and write. */
#define PAGE_FLAG_READ_WRITE     0x00000002
/** @brief Page flag: access permission set to read only. */
#define PAGE_FLAG_READ_ONLY      0x00000000
/** @brief Page flag: page present. */
#define PAGE_FLAG_PRESENT        0x00000001

/** @brief Custom define flag: regular memory. */
#define PAGE_FLAG_REGULAR        0x00000000
/** @brief Custom define flag: hardware mapped. */
#define PAGE_FLAG_HARDWARE       0x00000200
/** @brief Custom define flag: copy on write. */
#define PAGE_FLAG_COPY_ON_WRITE  0x00000400
/** @brief Custom define flag: private memory. */
#define PAGE_FLAG_PRIVATE        0x00000600
/** @brief Custome flag mask */
#define PAGE_FLAG_OS_CUSTOM_MASK 0x00000E00

/** @brief Defines the first kernel page directory entry. */
#define KERNEL_FIRST_PGDIR_ENTRY (KERNEL_MEM_OFFSET >> PG_DIR_ENTRY_OFFSET)

/** @brief Frame reference directory entry offset */
#define FRAME_REF_DIR_ENTRY_OFFSET        22
/** @brief Frame reference table entry offset */
#define FRAME_REF_TABLE_ENTRY_OFFSET      12
/** @brief Frame reference table entry offset mask */
#define FRAME_REF_TABLE_ENTRY_OFFSET_MASK 0x3FF

/** @brief Frame reference directory size */
#define FRAME_REF_DIR_SIZE   1024
/** @brief Frame reference table size */
#define FRAME_REF_TABLE_SIZE 1024

/** @brief Frame reference table present flag */
#define FRAME_REF_PRESENT    0x80000000
/** @brief Frame reference table hardware flag */
#define FRAME_REF_IS_HW      0x40000000
/** @brief Frame reference table reference count mask */
#define FRAME_REF_COUNT_MASK 0x00FFFFFF

/*******************************************************************************
 * STRUCTURES AND TYPES
 ******************************************************************************/

/* None */

/*******************************************************************************
 * MACROS
 ******************************************************************************/

/* None */

/*******************************************************************************
 * GLOBAL VARIABLES
 ******************************************************************************/

/************************* Imported global variables **************************/
/* None */

/************************* Exported global variables **************************/
/* None */

/************************** Static global variables ***************************/
/* None */

/*******************************************************************************
 * FUNCTIONS
 ******************************************************************************/

/* None */

#endif /* #ifndef __I386_ARCH_MEMMGT_H_ */

/************************************ EOF *************************************/