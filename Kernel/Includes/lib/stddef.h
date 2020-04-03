/*******************************************************************************
 * @file stddef.h
 *
 * @author Alexy Torres Aurora Dugo
 *
 * @date 04/10/2017
 *
 * @version 1.0
 *
 * @brief Standard definitions for the kernel.
 *
 * @details Standard definitions for the kernel. Contains the UTK error codes
 * definition, and some types definitions.
 *
 * @copyright Alexy Torres Aurora Dugo
 ******************************************************************************/

#ifndef __STDDEF_H_
#define __STDDEF_H_

#include <config.h>           /* Kernel configuration */
#include <lib/stdint.h>       /* Generic int types */

/*******************************************************************************
 * CONSTANTS
 ******************************************************************************/

/** @brief UTK's NULL definition. */
#define NULL ((void *)0)

/** @brief Defines the MIN function, return the minimal value between two
 * variables.
 *
 * @param[in] x The first value to compare.
 * @param[in] x The second value to compare.
 *
 * @return The smallest value.
 */
#define MIN(x, y) ((x) < (y) ? (x) : (y))

/** @brief Defines the MAX function, return the maximal value between two
 * variables.
 *
 * @param[in] x The first value to compare.
 * @param[in] x The second value to compare.
 *
 * @return The biggest value.
 */
#define MAX(x, y) ((x) < (y) ? (y) : (x))

/*******************************************************************************
 * STRUCTURES
 ******************************************************************************/

/** @brief System return states enumeration. */
enum OS_RETURN
{
    /** @brief No error occured. */
    OS_NO_ERR                              = 0,
    /** @brief UTK Error value. */
    OS_ERR_NULL_POINTER                    = 1,
    /** @brief UTK Error value. */
    OS_ERR_OUT_OF_BOUND                    = 2,
    /** @brief UTK Error value. */
    OR_ERR_UNAUTHORIZED_INTERRUPT_LINE     = 3,
    /** @brief UTK Error value. */
    OS_ERR_INTERRUPT_ALREADY_REGISTERED    = 4,
    /** @brief UTK Error value. */
    OS_ERR_INTERRUPT_NOT_REGISTERED        = 5,
    /** @brief UTK Error value. */
    OS_ERR_NO_SUCH_IRQ_LINE                = 6,
    /** @brief UTK Error value. */
    OS_ERR_NO_MORE_FREE_EVENT              = 7,
    /** @brief UTK Error value. */
    OS_ERR_NO_SUCH_ID                      = 8,
    /** @brief UTK Error value. */
    OS_ERR_MALLOC                          = 9,
    /** @brief UTK Error value. */
    OS_ERR_UNAUTHORIZED_ACTION             = 10,
    /** @brief UTK Error value. */
    OS_ERR_FORBIDEN_PRIORITY               = 11,
    /** @brief UTK Error value. */
    OS_ERR_MUTEX_UNINITIALIZED             = 12,
    /** @brief UTK Error value. */
    OS_ERR_SEM_UNINITIALIZED               = 13,
    /** @brief UTK Error value. */
    OS_ERR_MAILBOX_NON_INITIALIZED         = 14,
    /** @brief UTK Error value. */
    OS_ERR_QUEUE_NON_INITIALIZED           = 15,
    /** @brief UTK Error value. */
    OS_ERR_NO_SEM_BLOCKED                  = 16,
    /** @brief UTK Error value. */
    OS_ERR_NO_MUTEX_BLOCKED                = 17,
    /** @brief UTK Error value. */
    OS_ERR_GRAPHIC_MODE_NOT_SUPPORTED      = 19,
    /** @brief UTK Error value. */
    OS_MUTEX_LOCKED                        = 20,
    /** @brief UTK Error value. */
    OS_SEM_LOCKED                          = 21,
    /** @brief UTK Error value. */
    OS_ERR_CHECKSUM_FAILED                 = 22,
    /** @brief UTK Error value. */
    OS_ERR_ACPI_UNSUPPORTED                = 23,
    /** @brief UTK Error value. */
    OS_ACPI_NOT_INITIALIZED                = 24,
    /** @brief UTK Error value. */
    OS_ERR_NO_SUCH_LAPIC_ID                = 25,
    /** @brief UTK Error value. */
    OS_ERR_NO_SUCH_SERIAL_BAUDRATE         = 26,
    /** @brief UTK Error value. */
    OS_ERR_NO_SUCH_SERIAL_PARITY           = 27,
    /** @brief UTK Error value. */
    OS_ERR_ATA_DEVICE_NOT_PRESENT          = 28,
    /** @brief UTK Error value. */
    OS_ERR_ATA_DEVICE_ERROR                = 29,
    /** @brief UTK Error value. */
    OS_ERR_ATA_BAD_SECTOR_NUMBER           = 30,
    /** @brief UTK Error value. */
    OS_ERR_ATA_SIZE_TO_HUGE                = 31,
    /** @brief UTK Error value. */
    OS_ERR_VESA_NOT_SUPPORTED              = 32,
    /** @brief UTK Error value. */
    OS_ERR_VESA_MODE_NOT_SUPPORTED         = 33,
    /** @brief UTK Error value. */
    OS_ERR_VESA_NOT_INIT                   = 34,
    /** @brief UTK Error value. */
    OS_ERR_NO_MORE_FREE_MEM                = 35,
    /** @brief UTK Error value. */
    OS_ERR_PAGING_NOT_INIT                 = 36,
    /** @brief UTK Error value. */
    OS_ERR_MAPPING_ALREADY_EXISTS          = 37,
    /** @brief UTK Error value. */
    OS_ERR_MEMORY_NOT_MAPPED               = 38,
    /** @brief UTK Error value. */
    OS_ERR_SMBIOS_NOT_FOUND                = 39,
    /** @brief UTK Error value. */
    OS_ERR_BAD_HANDLER                     = 40,
    /** @brief UTK Error value. */
    OS_ERR_MBR_PARTITION_INDEX_TOO_LARGE   = 41,
    /** @brief UTK Error value. */
    OS_ERR_BAD_PARTITION_FORMAT            = 42,
    /** @brief UTK Error value. */
    OS_ERR_PART_ALREADY_MOUNTED            = 43,
    /** @brief UTK Error value. */
    OS_ERR_PART_NOT_MOUNTED                = 44,
    /** @brief UTK Error value. */
    OS_ERR_MOUNT_POINT_USED                = 45,
    /** @brief UTK Error value. */
    OS_ERR_WRONG_MOUNT_POINT               = 46,
    /** @brief UTK Error value. */
    OS_ERR_UNSUPPORTED_DEVICE_TYPE         = 47,
    /** @brief UTK Error value. */
    OS_ERR_WRONG_FAT32_BPB                 = 48,
    /** @brief UTK Error value. */
    OS_ERR_WRONG_FILESYSTEM                = 49,
    /** @brief UTK Error value. */
    OS_ERR_FAT32_BPS_NOT_SUPPORTED         = 50,
    /** @brief UTK Error value. */
    OS_ERR_FAT32_REQ_TOO_BIG               = 51,
    /** @brief UTK Error value. */
    OS_ERR_NOT_A_FOLDER                    = 52,
    /** @brief UTK Error value. */
    OS_ERR_FILE_NOT_FOUND                  = 53,
    /** @brief UTK Error value. */
    OS_ERR_NOT_A_FILE                      = 54,
    /** @brief UTK Error value. */
    OS_ERR_FILE_ALREADY_EXISTS             = 55,
    /** @brief UTK Error value. */
    OS_ERR_BAD_CLUSTER                     = 56,
    /** @brief UTK Error value. */
    OS_ERR_BAD_FILE_NAME                   = 57,
    /** @brief UTK Error value. */
    OS_ERR_PERMISSION_DENIED               = 58,
    /** @brief UTK Error value. */
    OS_ERR_NOT_SUPPORTED                   = 59,
    /** @brief UTK Error value. */
    OS_ERR_KERNEL_MEM_OFFSET_UNALIGNED     = 60,
    /** @brief UTK Error value. */
    OS_ERR_HANDLER_ALREADY_EXISTS          = 61,
};

/**
 * @brief Defines OS_RETURN_E type as a shorcut for enum OS_RETURN.
 */
typedef enum OS_RETURN OS_RETURN_E;

/**
 * @brief Defines OS_EVENT_ID type as a renaming for int32_t.
 */
typedef int32_t OS_EVENT_ID;

#ifndef __SIZE_TYPE__
#error __SIZE_TYPE__ not defined
#endif

/**
 * @brief Defines size_t type as a renaming for __SIZE_TYPE__.
 */
typedef __SIZE_TYPE__ size_t;

#ifndef __PTRDIFF_TYPE__
#error __PTRDIFF_TYPE__ not defined
#endif

/**
 * @brief Defines ptrdiff_t type as a renaming for __PTRDIFF_TYPE__.
 */
typedef __PTRDIFF_TYPE__ ptrdiff_t;

/**
 * @brief Defines uintptr_t type as address type.
 */
#if ARCH_X86_64
typedef uint64_t uintptr_t;
#elif ARCH_I386
typedef uint32_t uintptr_t;
#endif

#endif /* __STDDEF_H_ */
