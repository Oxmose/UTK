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
 * @brief Mounts the USTAR partition.
 *
 * @details Mounts the USTAR partition. This function validates the mount point
 * and the devices blocks used to identifiy the partition.
 *
 * @param[in,out] partition The partition structure to mount.
 * @param[in] mount_pt The mount point used to mount the partition.
 *
 * @return The error or success status is returned.
 */
OS_RETURN_E ustar_mount(vfs_partition_t* partition,
                        const char* mount_pt);

/**
 * @brief Unmounts the USTAR partition.
 *
 * @details Unmounts the USTAR partition. This function validates the devices
 * blocks used to identifiy the partition. The partition is flushed before the
 * unmount is performed.
 *
 * @param[in,out] partition The partition structure to unmount.
 *
 * @return The error or success status is returned.
 */
OS_RETURN_E ustar_umount(vfs_partition_t* partition);

/**
 * @brief Opens a file in the USTAR partition.
 *
 * @details Opens a file in the USTAR partition. The vnode must be initialized
 * to point to the partition used. The partition's status and validity is
 * checked and the vnode structure filled with requested data.
 *
 * @param[in] path The file to open path.
 * @param[in,out] vnode The virtual node to be populated when opening the file.
 *
 * @return The error or success status is returned.
 */
OS_RETURN_E ustar_open_file(const char* path,
                            vfs_vnode_t* vnode);

/**
 * @brief Closes a file in the USTAR partition.
 *
 * @details Closes a file in the USTAR partition. The vnode must be previously
 * opened. The partition's status and validity is checked when closing the file.
 * The partition is also flushed on closing files.
 *
 * @param[in,out] vnode The virtual node that was precedently opened and needs
 * to be closed.
 *
 * @return The error or success status is returned.
 */
OS_RETURN_E ustar_close_file(const vfs_vnode_t* vnode);

/**
 * @brief Reads a file in the USTAR partition.
 *
 * @details Reads a file in the USTAR partition. The vnode must be previously
 * opened. The partition's status and validity is checked prior to reading the
 * file. If the file cannot be fully read, the actual read size of set in the
 * parameter's buffer.
 *
 * @param[in] vnode The virtual node that represents the file to manipulate.
 * @param[out] buffer The buffer used to receive the data.
 * @param[in] size The size of the data to read from the file in bytes.
 * @param[out] actual_size The size read from the file in bytes.
 *
 * @return The error or success status is returned.
 */
OS_RETURN_E ustar_read_file(const vfs_vnode_t* vnode,
                            void* buffer,
                            size_t size,
                            size_t* actual_size);

/**
 * @brief Writes a file in the USTAR partition.
 *
 * @details Writes a file in the USTAR partition. The vnode must be previously
 * opened. The partition's status and validity is checked prior to writing the
 * file. If the file cannot be fully written, the actual write size of set in
 * the parameter's buffer.
 *
 * @param[in] vnode The virtual node that represents the file to manipulate.
 * @param[in] buffer The buffer that contains the data to write.
 * @param[in] size The size of the data to write to the file in bytes.
 * @param[out] actual_size The size written to the file in bytes.
 *
 * @return The error or success status is returned.
 */
OS_RETURN_E ustar_write_file(const vfs_vnode_t* vnode,
                             const void* buffer,
                             size_t size,
                             size_t* actual_size);

/**
 * @brief Not implemented. Creates a file in the USTAR partition.
 *
 * @details Not implemented. Creates a file in the USTAR partition.
 *
 * @warning Not implemented.
 *
 * @param[in] path The path to the file to create.
 * @param[in] type The type od file to create.
 * @param[in] access_rights The file's octal access rights.
 * @param[in] owner_name The file's owner name.
 * @param[in] group_name The group's owner name.
 *
 * @return The error or success status is returned.
 */
OS_RETURN_E ustar_create_file(const char* path,
                              const VFS_FILE_TYPE_E type,
                              const vfs_access_right_t access_rights,
                              const char* owner_name,
                              const char* group_name);

/**
 * @brief Removes a file in the USTAR partition.
 *
 * @details Removes a file in the USTAR partition. The file is searched in the
 * partition to be removed. The partition's integrity is validated prior to the
 * file's removal.
 *
 * @param[in] part The partition to remove the file from.
 * @param[in] path The path to the file to remove.
 *
 * @return The error or success status is returned.
 */
OS_RETURN_E ustar_remove_file(const vfs_partition_t* part, const char* path);

/**
 * @brief Renames a file in the USTAR partition.
 *
 * @details Renames a file in the USTAR partition. The file is searched in the
 * partition to be removed. The partition's integrity is validated prior to the
 * file's renaming.
 *
 * @param[in] part The partition to rename the file in.
 * @param[in] old_path The old path to the file to rename.
 * @param[in] new_path The new path to the file to rename.
 *
 * @return The error or success status is returned.
 */
OS_RETURN_E ustar_rename_file(const vfs_partition_t* part,
                              const char* old_path,
                              const char* new_path);

/**
 * @brief Truncates a file in the USTAR partition.
 *
 * @details Truncates a file in the USTAR partition. The new file size must be
 * smaller that its previous size. The partition's status and validity is
 * checked prior to truncating the file.
 *
 * @param[in] partition The partition that contains the file.
 * @param[in] path The path to the file to truncate.
 * @param[in] new_size The new size to give to the file.
 *
 * @return The error or success status is returned.
 */
OS_RETURN_E ustar_truncate_file(const vfs_partition_t* partition,
                                const char* path,
                                const size_t new_size);

/**
 * @brief Lists the content of a directory.
 *
 * @details Lists the content of a directory. The partition is checked for
 * integrity before liting the files. Each item is listed in the buffer and
 * separated with a NULL terminator.
 *
 * @param vnode The vnode taht represents the directory to list.
 * @param item_count The number of item that is contained in the directory.
 * @param buffer The buffer that receive the list of items.
 * @param buff_size The maximal size of the buffer used to receive the list.
 *
 * @return The error or success status is returned.
 */
OS_RETURN_E ustar_list_directory(const vfs_vnode_t* vnode,
                                 size_t* item_count,
                                 char* buffer,
                                 const size_t buff_size);

#endif /* #ifndef __FS_USTAR_FS_H_ */

/************************************ EOF *************************************/