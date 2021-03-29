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

/*******************************************************************************
 * STRUCTURES
 ******************************************************************************/

enum VFS_FILE_TYPE
{
    VFS_FILE_TYPE_FILE,
    VFS_FILE_TYPE_FOLDER,
    VFS_FILE_TYPE_DELETED
};

typedef enum VFS_FILE_TYPE VFS_FILE_TYPE_E;

struct vfs_fs_driver
{
    uint32_t empty;
};

typedef struct vfs_fs_driver vfs_fs_driver_t;

struct vfs_device
{
    uint32_t empty;
};

typedef struct vfs_device vfs_device_t;

struct vfs_partition
{
    char                  name[VFS_PART_NAME_LENGTH];

    vfs_device_t*         device;
    vfs_fs_driver_t*      fs_type;

    uint64_t              block_start;
    size_t                size;
};

typedef struct vfs_partition vfs_partition_t;

struct vfs_mount_point
{
    char name[VFS_MOUNT_POINT_LENGTH];

    struct vfs_partition* mounted_partition;
};

typedef struct vfs_mount_point vfs_mount_point_t;

struct vfs_timedate
{
    uint8_t  day;
    uint8_t  month;
    uint16_t year;

    uint8_t hours;
    uint8_t minutes;
    uint8_t seconds;

    uint8_t timezone_shift;
};

typedef struct vfs_timedate vfs_timedate_t;

typedef struct vfs_file
{
    VFS_FILE_TYPE_E type;

    char path[VFS_PATH_LENGTH];
    char name[VFS_FILE_NAME_LENGTH];
    char short_name[VFS_FILE_SHORT_NAME_LENGTH];

    uint32_t access_rights;

    uint64_t block_start;
    size_t   size;

    vfs_timedate_t creation_datetime;
    vfs_timedate_t last_access_datetime;
    vfs_timedate_t last_modification_datetime;

    uint64_t children_count;

    struct vfs_file* parent;
    struct vfs_file* first_child;
    struct vfs_file* next;
} vfs_file_t;

/*******************************************************************************
 * FUNCTIONS
 ******************************************************************************/

/* None */

#endif /* #ifndef __DRIVERS_VIRT_FS_H_ */