/*******************************************************************************
 * @file init_rd.h
 *
 * @see init_rd.c
 *
 * @author Alexy Torres Aurora Dugo
 *
 * @date 27/03/2021
 *
 * @version 1.0
 *
 * @brief Kernel's init ram disk driver.
 *
 * @details Kernel's init ram disk driver. Defines the functions and
 * structures used by the kernel to manage manage the init ram disk.
 *
 * @copyright Alexy Torres Aurora Dugo
 ******************************************************************************/

#ifndef __FS_INIT_RD_H_
#define __FS_INIT_RD_H_

/*******************************************************************************
 * INCLUDES
 ******************************************************************************/

#include <kernel_error.h> /* Kernel error codes */
#include <stdint.h>       /* Generic int types */
#include <stddef.h>       /* Standard definitions */
#include <virt_fs.h>      /* Virtual filesystem */

/*******************************************************************************
 * CONSTANTS
 ******************************************************************************/

/* None */

/*******************************************************************************
 * STRUCTURES AND TYPES
 ******************************************************************************/

/** @brief Defines the structure that represents the INIT RAM Disk. */
typedef struct
{
    /** @brief The start address of the INIT RAM Disk region. */
    uintptr_t start_addr;

    /** @brief The end address of the INIT RAM Disk region. */
    uintptr_t end_addr;

    /** @brief The size of the INIT RAM Disk region. */
    size_t    size;
} initrd_device_t;

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

/**
 * @brief Initializes the init ram disk device.
 *
 * @details Initializes the init ram disk device. This function fill the device
 * given as parameter with the information of the init ram disk.
 *
 * @param[out] device The device to initialize.
 *
 * @returns OS_NO_ERR on success, an error otherwise.
 */
OS_RETURN_E initrd_init_device(initrd_device_t* device);

/**
 * @brief Returns the init ram disk device.
 *
 * @details Returns the init ram disk device. This function fill the device
 * given as parameter with the information of the init ram disk after
 * initialization.
 *
 * @param[out] device The device buffer to fill.
 *
 * @returns OS_NO_ERR on success, an error otherwise.
 */
OS_RETURN_E initrd_get_device(initrd_device_t* device);

/**
 * @brief Reads bytes on the device pointed by the device given as parameter.
 *
 * @details Reads bytes on the device pointed by the device given as parameter.
 * The read is performed contiguously on blocks starting from the begining of
 * the block ID given as parameter until the given size is reached.
 *
 * @param device The device to read the data from.
 * @param sector The block identifier where the data are located.
 * @param buffer The buffer that is used to store the read data.
 * @param size The number of bytes to read from the device.
 * @param offset The offset to start to read from.
 *
 * @returns OS_NO_ERR if no error were detected. An error code is returned
 * otherwise.
 */
OS_RETURN_E initrd_read_blocks(const vfs_device_t* device,
                               const uint32_t block_id,
	                           void* buffer,
                               const size_t size,
                               const size_t offset);
/**
 * @brief Writes bytes on the device pointed by the device given as parameter.
 *
 * @details Writes bytes on the device pointed by the device given as parameter.
 * The write is performed contiguously on blocks starting from the begining of
 * the block ID given as parameter until the given size is reached. If a block
 * is paritally writen, the rest of the block is zeroized.
 *
 * @param device The device to write the data to.
 * @param sector The block identifier where the data are located.
 * @param buffer The buffer that is used to store the write data.
 * @param size The number of bytes to write to the device.
 * @param offset The offset to start to write to.
 *
 * @returns OS_NO_ERR if no error were detected. An error code is returned
 * otherwise.
 */
OS_RETURN_E initrd_write_blocks(const vfs_device_t* device,
                                const uint32_t block_id,
	                            const void* buffer,
                                const size_t size,
                                const size_t offset);

/**
 * @brief Flushed data to the init ram disk.
 *
 * @details Flushed data to the init ram disk. This function has no effect as
 * all modification done by other function are directly flushed to the ram disk.
 *
 * @param device The device to be flushed.
 * @param block_id Unused, set for compatibility.
 * @param size Unused, set for compatibility.
 * @param offset Unused, set for compatibility.
 *
 * @returns OS_NO_ERR if no error were detected. An error is returned otherwise.
 */
OS_RETURN_E initrd_flush(const vfs_device_t* device,
                         const uint32_t block_id,
                         const size_t size,
                         const size_t offset);

#endif /* #ifndef __FS_INIT_RD_H_ */

/************************************ EOF *************************************/
