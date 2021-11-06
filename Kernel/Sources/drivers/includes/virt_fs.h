/*******************************************************************************
 * @file virt_fs.h
 * 
 * @see virt_fs.c
 *
 * @author Alexy Torres Aurora Dugo
 *
 * @date 27/03/2021
 *
 * @version 1.0
 *
 * @brief Kernel's virtual filesystem driver.
 * 
 * @details Kernel's virtual filesystem driver. Defines the functions and 
 * structures used by the kernel to manage the virtual filesystem.
 * 
 * @copyright Alexy Torres Aurora Dugo
 ******************************************************************************/

#ifndef __DRIVERS_VIRT_FS_H_
#define __DRIVERS_VIRT_FS_H_

#include <kernel_error.h> /* Kernel error codes */
#include <stdint.h>       /* Generic int types */
#include <stddef.h>       /* Standard definitions */

/*******************************************************************************
 * CONSTANTS
 ******************************************************************************/

#define VFS_PART_NAME_LENGTH       32
#define VFS_MOUNT_POINT_LENGTH     32
#define VFS_FILE_SHORT_NAME_LENGTH 256
#define VFS_FILE_NAME_LENGTH       256
#define VFS_PATH_LENGTH            1024
#define VFS_OWNER_NAME_LENGTH      32
#define VFS_GROUP_NAME_LENGTH      32

#define VFS_RIGHTS_UREAD  0x100
#define VFS_RIGHTS_UWRITE 0x800
#define VFS_RIGHTS_UEXEC  0x400

#define VFS_RIGHTS_GREAD  0x020
#define VFS_RIGHTS_GWRITE 0x010
#define VFS_RIGHTS_GEXEC  0x008

#define VFS_RIGHTS_OREAD  0x004
#define VFS_RIGHTS_OWRITE 0x002
#define VFS_RIGHTS_OEXEC  0x001

/*******************************************************************************
 * STRUCTURES
 ******************************************************************************/

enum VFS_FILE_TYPE
{
    VFS_FILE_TYPE_FILE,
    VFS_FILE_TYPE_FOLDER,
    VFS_FILE_TYPE_HARD_LINK,
    VFS_FILE_TYPE_SYM_LINK,
    VFS_FILE_TYPE_CHAR_DEV,
    VFS_FILE_TYPE_BLOCK_DEV,
    VFS_FILE_TYPE_DIR,
    VFS_FILE_TYPE_NAMED_PIPE,
    VFS_FILE_TYPE_OTHER
};

typedef enum VFS_FILE_TYPE VFS_FILE_TYPE_E;

struct vfs_fs_driver
{
    uint32_t empty;
};

typedef struct vfs_fs_driver vfs_fs_driver_t;

struct vfs_device
{
    OS_RETURN_E (*read_blocks)(const void* device, 
                               const uint32_t block_id,
	                           void* buffer, 
                               const size_t size,
                               const size_t offset);
    OS_RETURN_E (*write_blocks)(const void* device, 
                                const uint32_t block_id,
	                            const void* buffer, 
                                const size_t size,
                                const size_t offset);
    OS_RETURN_E (*flush_blocks)(const void* device, 
                                const uint32_t block_id,
                                const size_t size,
                                const size_t offset);
    
    size_t block_size;
};

typedef struct vfs_device vfs_device_t;

struct vfs_partition
{
    char                  name[VFS_PART_NAME_LENGTH];

    vfs_device_t*         device;
    vfs_fs_driver_t*      fs_driver;

    uint64_t              first_block;
    size_t                size;
};

typedef struct vfs_partition vfs_partition_t;

struct vfs_mount_point
{
    char name[VFS_MOUNT_POINT_LENGTH];

    struct vfs_partition* mounted_partition;
};

typedef struct vfs_mount_point vfs_mount_point_t;


typedef uint16_t vfs_access_right_t;

typedef struct vfs_vnode
{
    VFS_FILE_TYPE_E type;

    char path[VFS_PATH_LENGTH];
    char name[VFS_FILE_NAME_LENGTH];
    char short_name[VFS_FILE_SHORT_NAME_LENGTH];

    vfs_access_right_t access_rights;

    uint8_t  owner_id;
    uint8_t  group_id;

    char     owner_name[VFS_OWNER_NAME_LENGTH];
    char     group_name[VFS_GROUP_NAME_LENGTH];

    size_t   size;

    uint64_t creation_datetime;
    uint64_t last_access_datetime;
    uint64_t last_modification_datetime;

    vfs_partition_t* partition;
    void* fs_inode;
} vfs_vnode_t;

typedef struct vfs_ftable_entry
{
    uint32_t reference_count;

    uint64_t cursor;

    uint16_t open_rights;

    vfs_vnode_t* vnode;
} vfs_ftable_entry_t;

/*******************************************************************************
 * FUNCTIONS
 ******************************************************************************/

OS_RETURN_E vfs_mount(const char* part_path,
                      const char* mount_pt);

OS_RETURN_E vfs_umount(const char* part_path);

OS_RETURN_E vfs_open_file(const char* path,
                          vfs_ftable_entry_t* file_descriptor);

OS_RETURN_E vfs_close_file(const vfs_ftable_entry_t* file_descriptor);

OS_RETURN_E vfs_read_file(const vfs_ftable_entry_t* file_descriptor,
                          const size_t size,
                          void* buffer,
                          size_t* actual_size);

OS_RETURN_E vfs_write_file(const vfs_ftable_entry_t* file_descriptor,
                           const size_t size,
                           const void* buffer,
                           size_t* actual_size);

OS_RETURN_E vfs_create_file(const char* path);

OS_RETURN_E vfs_remove_file(const char* path);

OS_RETURN_E vfs_rename_file(const char* old_path,
                            const char* new_name);

OS_RETURN_E vfs_truncate_file(const char* path,
                              const size_t new_size);

#endif /* #ifndef __DRIVERS_VIRT_FS_H_ */