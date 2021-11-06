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
                         const char* mount_pt);

OS_RETURN_E ustar_umount(vfs_partition_t* partition);

OS_RETURN_E ustar_open_file(const char* path, 
                            vfs_vnode_t* vnode);

OS_RETURN_E ustar_close_file(const vfs_vnode_t* vnode);

OS_RETURN_E ustar_read_file(const vfs_vnode_t* vnode,
                            void* buffer,
                            size_t size,                            
                            size_t* actual_size);

OS_RETURN_E ustar_write_file(const vfs_vnode_t* vnode,
                             const void* buffer,
                             size_t size,                             
                             size_t* actual_size);

OS_RETURN_E ustar_remove_file(const vfs_vnode_t* vnode, const char* path);

OS_RETURN_E ustar_rename_file(const vfs_vnode_t* vnode,
                              const char* old_path,
                              const char* new_path);

OS_RETURN_E ustar_truncate_file(const vfs_vnode_t* vnode,
                                const char* path,
                                const size_t new_size);

#endif /* #ifndef __DRIVERS_USTAR_FS_H_ */