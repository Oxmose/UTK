/*******************************************************************************
 * @file ustar_fs.h
 * 
 * @see ustar_fs.c
 *
 * @author Alexy Torres Aurora Dugo
 *
 * @date 27/03/2021
 *
 * @version 1.0
 *
 * @brief Kernel's USTAR filesystem driver.
 * 
 * @details Kernel's USTAR filesystem driver. Defines the functions and 
 * structures used by the kernel to manage USTAR partitions.
 * 
 * @copyright Alexy Torres Aurora Dugo
 ******************************************************************************/

#ifndef __DRIVERS_USTAR_FS_H_
#define __DRIVERS_USTAR_FS_H_

#include <virt_fs.h>      /* Kernel virtual file system */
#include <kernel_error.h> /* Kernel error codes */

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

OS_RETURN_E ustart_mount(vfs_partition_t* partition,
                         vfs_mount_point_t* mount_pt);

OS_RETURN_E ustart_umount(vfs_partition_t* partition,
                          vfs_mount_point_t* mount_pt);

OS_RETURN_E ustart_open_file(const vfs_partition_t* partition,
                             vfs_file_t* file_descriptor,
                             const char* path,
                             const char* filename);

OS_RETURN_E ustart_close_file(const vfs_partition_t* partition,
                              const vfs_file_t* file_descriptor);

OS_RETURN_E ustart_read_file(const vfs_partition_t* partition,
                             const vfs_file_t* file_descriptor,
                             const uint32_t byte_offset,
                             const uint32_t size,
                             uint8_t* buffer);

OS_RETURN_E ustart_write_file(const vfs_partition_t* partition,
                              vfs_file_t* file_descriptor,
                              const uint32_t offset,
                              const uint32_t size,
                              const uint8_t* write_buffer);

OS_RETURN_E ustart_create_file(const vfs_partition_t* partition,
                               vfs_file_t* file_descriptor);

OS_RETURN_E ustart_remove_file(const vfs_partition_t* partition,
                               vfs_file_t* file_descriptor);

OS_RETURN_E ustart_rename_file(const vfs_partition_t* partition,
                               vfs_file_t* file_descriptor,
                               const char* new_name);

OS_RETURN_E ustart_truncate_file(const vfs_partition_t* partition,
                                 vfs_file_t* file_descriptor,
                                 const uint32_t new_size);

OS_RETURN_E ustart_move_file(const vfs_partition_t* partition,
                             vfs_file_t* file_descriptor,
                             const char* new_path);

#endif /* #ifndef __DRIVERS_USTAR_FS_H_ */