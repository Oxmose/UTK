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

#ifndef __FS_USTAR_FS_H_
#define __FS_USTAR_FS_H_


/*******************************************************************************
 * INCLUDES
 ******************************************************************************/

#include <virt_fs.h>      /* Kernel virtual file system */
#include <kernel_error.h> /* Kernel error codes */
#include <stddef.h>       /* Standard definitions */

/*******************************************************************************
 * CONSTANTS
 ******************************************************************************/

/* None */

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

/* Imported global variables */
/* None */

/* Exported global variables */
/* None */

/* Static global variables */
/* None */

/*******************************************************************************
 * FUNCTIONS
 ******************************************************************************/

OS_RETURN_E ustar_mount(vfs_partition_t* partition,
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

OS_RETURN_E ustar_create_file(const char* path,
                              const VFS_FILE_TYPE_E type,
                              const vfs_access_right_t access_rights,
                              const char* owner_name,
                              const char* group_name);

OS_RETURN_E ustar_remove_file(const vfs_partition_t* part, const char* path);

OS_RETURN_E ustar_rename_file(const vfs_partition_t* vnode,
                              const char* old_path,
                              const char* new_path);

OS_RETURN_E ustar_truncate_file(const vfs_partition_t* partition,
                                const char* path,
                                const size_t new_size);

OS_RETURN_E ustar_list_directory(const vfs_vnode_t* vnode,
                                 size_t* item_count,
                                 char* buffer,
                                 const size_t buff_size);

#endif /* #ifndef __FS_USTAR_FS_H_ */

/* EOF */