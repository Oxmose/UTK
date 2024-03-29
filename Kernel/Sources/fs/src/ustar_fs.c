/*******************************************************************************
 * @file ustar_fs.c
 *
 * @see ustar_fs.h
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

/*******************************************************************************
 * INCLUDES
 ******************************************************************************/

/* Included headers */
#include <kernel_error.h>  /* Kernel error codes */
#include <stdint.h>        /* Generic int types */
#include <string.h>        /* String and momeory anipulation */
#include <kernel_output.h> /* Kernel output */
#include <kheap.h>         /* Kernel heap */
#include <panic.h>         /* Kernel panic */

/* Configuration files */
#include <config.h>
#include <test_bank.h>

/* Header file */
#include <ustar_fs.h>

/*******************************************************************************
 * CONSTANTS
 ******************************************************************************/

/** @brief USTAR magic value. */
#define USTAR_MAGIC "ustar "
/** @brief USTAR maximal filename length. */
#define USTAR_FILENAME_MAX_LENGTH 100
/** @brief USTAR block size. */
#define USTAR_BLOCK_SIZE 512
/** @brief USTAR file size maximal length. */
#define USTAR_FSIZE_FIELD_LENGTH 12
/** @brief USTAR last edit maximal length. */
#define USTAR_LEDIT_FIELD_LENGTH 12
/** @brief USTAR file user ID maximal length. */
#define USTAR_UID_FIELD_LENGTH 8
/** @brief USTAR file mode maximal length. */
#define USTAR_MODE_FIELD_LENGTH 8
/** @brief USTAR file prefix maximal length. */
#define USTAR_PREFIX_NAME_LENGTH 155

/** @brief USTAR User read permission bitmask. */
#define T_UREAD  0x100
/** @brief USTAR User write permission bitmask. */
#define T_UWRITE 0x080
/** @brief USTAR User execute permission bitmask. */
#define T_UEXEC  0x040

/** @brief USTAR Group read permission bitmask. */
#define T_GREAD  0x020
/** @brief USTAR Group write permission bitmask. */
#define T_GWRITE 0x010
/** @brief USTAR Group execute permission bitmask. */
#define T_GEXEC  0x008

/** @brief USTAR Other read permission bitmask. */
#define T_OREAD  0x004
/** @brief USTAR Other write permission bitmask. */
#define T_OWRITE 0x002
/** @brief USTAR Other execute permission bitmask. */
#define T_OEXEC  0x001

/*******************************************************************************
 * STRUCTURES AND TYPES
 ******************************************************************************/

/**
 * @brief Defines the USTAR operation allowed on the devise.
 */
typedef enum
{
    /** @brief Write operation */
    USTAR_DEV_OP_WRITE,
    /** @brief Read operation */
    USTAR_DEV_OP_READ,
    /** @brief Flush operation */
    USTAR_DEV_OP_FLUSH
} USTAR_DEV_OP_E;

/**
 * @brief USTAR header block definition as per USTAR standard.
 */
typedef struct
{
    /** @brief USTAR file name */
    char file_name[USTAR_FILENAME_MAX_LENGTH];
    /** @brief USTAR file mode */
    char mode[8];
    /** @brief USTAR owner user id */
    char user_id[8];
    /** @brief USTAR owner group id */
    char group_id[8];
    /** @brief Length of the file in bytes */
    char size[USTAR_FSIZE_FIELD_LENGTH];
    /** @brief Modify time of file */
    char last_edited[12];
    /** @brief Checksum for header */
    char checksum[8];
    /** @brief Type of file */
    char type;
    /** @brief Name of linked file */
    char linked_file_name[USTAR_FILENAME_MAX_LENGTH];
    /** @brief USTAR magic value */
    char magic[6];
    /** @brief USTAR version */
    char ustar_version[2];
    /** @brief Owner user name */
    char user_name[32];
    /** @brief Owner group name */
    char group_name[32];
    /** @brief Device major number */
    char dev_major[8];
    /** @brief Device minor number */
    char dev_minor[8];
    /** @brief Prefix for file name */
    char prefix[USTAR_PREFIX_NAME_LENGTH];
    /** @brief Unused padding */
    char padding[12];
} ustar_block_t;

/*******************************************************************************
 * MACROS
 ******************************************************************************/

/**
 * @brief Assert macro used by the USTAR driver to ensure correctness of
 * execution.
 *
 * @details Assert macro used by the USTAR driver to ensure correctness of
 * execution. Due to the critical nature of the USTAR driver, any error
 * generates a kernel panic.
 *
 * @param[in] COND The condition that should be true.
 * @param[in] MSG The message to display in case of kernel panic.
 * @param[in] ERROR The error code to use in case of kernel panic.
 */
#define USTAR_ASSERT(COND, MSG, ERROR) {                      \
    if((COND) == FALSE)                                       \
    {                                                         \
        PANIC(ERROR, "USTASR", MSG, TRUE);                    \
    }                                                         \
}

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
 * STATIC FUNCTIONS DECLARATIONS
 ******************************************************************************/

/**
 * @brief Get the number of items in a directory.
 *
 * @details Get the number of items in a directory. This function parses the
 * USTAR partition to find all the items in a directory.
 *
 * @param[in] partition The USTAR partition to use.
 * @param[in] path The directory path.
 * @param[out] item_count The buffer to receive the number of items.
 *
 * @return The success or error status is returned.
 */
static OS_RETURN_E ustar_get_item_count(const vfs_partition_t* partition,
                                        const char* path,
                                        size_t* item_count);

/**
 * @brief Searches a file for a given path.
 *
 * @details Searches a file for the given path and fills the USTAR block once
 * found.
 *
 * @param[in] partition The USTAR partition to use.
 * @param[in] path The file path.
 * @param[out] block The USTAR glock to use for the search.
 * @param[out] block_id The USTAR block id buffer to receive the block id of the
 * file.
 *
 * @return The success or error status is returned.
 */
static OS_RETURN_E ustar_search_file(const vfs_partition_t* partition,
                                     const char* path,
                                     ustar_block_t* block,
                                     uint32_t* block_id);

/**
 * @brief Fills a virtual node from a USTAR header block.
 *
 * @details Fills a virtual node from a USTAR header block. This function will
 * fill the virtual node by parsing the USTAR block header given as parameter.
 *
 * @param[out] vnode The virtual node to fill with the parsed data.
 * @param[in] block The USTAR header block containing the file information.
 * @param[in] block_id The USTAR block id corresponding to the file.
 * @param[in] path The path of the file.
 */
static void ustar_set_vnode(vfs_vnode_t* vnode,
                            ustar_block_t* block,
                            const uint32_t block_id,
                            const char* path);

/**
 * @brief Tranlates the USTAR permission coding to the VFS permission coding.
 *
 * @details Tranlates the USTAR permission coding to the VFS permission coding.
 *
 * @param[in] mode The USTAR string mode.
 *
 * @return The VFS permission value is returned.
 */
inline static vfs_access_right_t ustar_to_vfs_rights(const char* mode);

/**
 * @brief Tranlates the USTAR type coding to the VFS type coding.
 *
 * @details Tranlates the USTAR type coding to the VFS type coding.
 *
 * @param[in] type The USTAR string type.
 *
 * @return The VFS type value is returned.
 */
inline static VFS_FILE_TYPE_E ustar_to_vfs_type(const char type);

/**
 * @brief Returns if the current path and file is in the root of the USTAR
 * partition.
 *
 * @details Returns if the current path and file is in the root of the USTAR
 * partition.
 *
 * @param[in] path The path to the file to check.
 * @param[in] file The name of the file to check.
 *
 * @return Returns TRUE if the current path and file is in the root of the USTAR
 * partition, FALSE otherwise.
 */
inline static bool_t is_root(const char* path, const char* file);

/**
 * @brief Extract the path to a file from a given string.
 *
 * @details Extract the path to a file from a given string. The path does not
 * contain the file name.
 *
 * @param[in] path The path to analyse.
 * @param[out] buffer The buffer to receive only the extracted path without the
 * filename.
 */
inline static void ustar_get_path(const char* path, char* buffer);

/**
 * @brief Extract the filename of a file from a given string.
 *
 * @details Extract the filename of a file from a given string.
 *
 * @param[in] path The path to analyse.
 * @param[out] buffer The buffer to receive only the extracted filename.
 */
inline static void ustar_get_filename(const char* path, char* buffer);

/**
 * @brief Fills the USTAR block with the next file in the partition.
 *
 * @details Fills the USTAR block with the next file in the partition. This
 * file might not be in the same folder as the previous file.
 *
 * @param[in] partition The USTAR partition to use.
 * @param[in,out] block The filer header's block to use to get the next header.
 * This will be filled with the next header once the operation completed.
 * @param[in,out] inode The inode of the current file. This will be filled with
 * the inode of the next file in the partition.
 */
static void ustar_get_next_file(const vfs_partition_t* partition,
                                ustar_block_t* block,
                                uint32_t* inode);

/**
 * @brief Translates the decimal value given as parameter to an octal value
 * stored in a string.
 *
 * @details Translates the decimal value given as parameter to an octal value
 * stored in a string.
 *
 * @param[out] oct The buffer used to store the octal value.
 * @param[in] value The value to translate.
 * @param[in] size The size of the buffer used to receive the octal value.
 */
inline static void uint2oct(char* oct, uint32_t value, size_t size);

/**
 * @brief Translates the octal value giben as parameter to its decimal value.
 *
 * @details Translates the octal value giben as parameter to its decimal value.
 *
 * @param[in] oct The octal value to translated.
 * @param[in] size The dize of the buffer that contains the octal value.
 *
 * @return The decimal value of the octal value stored in the string is
 * returned.
 */
inline static uint32_t oct2uint(const char* oct, size_t size);

/**
 * @brief Checks if a USTAR header block is valid.
 *
 * @details Checks if a USTAR header block is valid. The integrity of the header
 * is validated with the checksum, magix and other values contained in the
 * header block.
 *
 * @param[in] block The header block to check.
 *
 * @return OS_NO_ERR is retuned is the block is valid. Otherwise, an error is
 * returned.
 */
inline static OS_RETURN_E ustar_check_block(const ustar_block_t* block);

/**
 * @brief Accesses a block to from the USTAR partition.
 *
 * @details Accesses a block to from the USTAR partition. This function allows
 * to perform different operations on the USTAR block.
 *
 * @param[in] partition The USTAR partition to use.
 * @param[in,out] buffer The buffer used to read or write the USTAR block.
 * @param[in] inode The inode to access.
 * @param[in] block_counts The number of block to access.
 * @param[in] operation The operation to perform. Refer to the USTAR_DEV_OP_E
 * values to know which operations are possible.
 *
 * @return The success or error status is returned.
 */
inline static OS_RETURN_E ustar_access_blocks_from_device(
                                            const vfs_partition_t* partition,
                                            void* buffer,
                                            const uint32_t inode,
                                            const size_t block_counts,
                                            const USTAR_DEV_OP_E operation);

/*******************************************************************************
 * FUNCTIONS
 ******************************************************************************/

inline static OS_RETURN_E ustar_access_blocks_from_device(
                                            const vfs_partition_t* partition,
                                            void* buffer,
                                            const uint32_t inode,
                                            const size_t block_counts,
                                            const USTAR_DEV_OP_E operation)
{
    size_t   phys_block_size;
    uint32_t first_phys_block;
    uint32_t first_phys_block_offset;

    phys_block_size = partition->device->block_size;

    /* Get the physical block on the device */
    first_phys_block = partition->first_block +
                         (inode * USTAR_BLOCK_SIZE) / phys_block_size;
    if(phys_block_size > USTAR_BLOCK_SIZE)
    {
        first_phys_block_offset = inode % (phys_block_size / USTAR_BLOCK_SIZE);
    }
    else
    {
        first_phys_block_offset = 0;
    }
    first_phys_block_offset *= USTAR_BLOCK_SIZE;

    KERNEL_DEBUG(USTAR_DEBUG_ENABLED, "USTAR",
                 "Reading blocks 0x%p (%d blocks)", inode, block_counts);

    /* Read / Write blocks */
    if(operation == USTAR_DEV_OP_WRITE)
    {
        return partition->device->write_blocks(partition->device,
                                               first_phys_block,
                                               buffer,
                                               block_counts * USTAR_BLOCK_SIZE,
                                               first_phys_block_offset);
    }
    else if(operation == USTAR_DEV_OP_READ)
    {
        return partition->device->read_blocks(partition->device,
                                              first_phys_block,
                                              buffer,
                                              block_counts * USTAR_BLOCK_SIZE,
                                              first_phys_block_offset);
    }
    else if(operation == USTAR_DEV_OP_FLUSH)
    {
        return partition->device->flush_blocks(partition->device,
                                               first_phys_block,
                                               block_counts * USTAR_BLOCK_SIZE,
                                               first_phys_block_offset);
    }
    else
    {
        return OS_ERR_UNAUTHORIZED_ACTION;
    }
}

inline static OS_RETURN_E ustar_check_block(const ustar_block_t* block)
{
    if(strncmp(block->magic, USTAR_MAGIC, 6) != 0)
    {
        return OS_ERR_WRONG_PARTITION_TYPE;
    }

    return OS_NO_ERR;
}

inline static uint32_t oct2uint(const char* oct, size_t size)
{
    uint32_t out = 0;
    uint32_t i   = 0;
    while (i < size && oct[i])
    {
        out = (out << 3) | (uint32_t)(oct[i++] - '0');
    }
    return out;
}

inline static void uint2oct(char* oct, uint32_t value, size_t size)
{
    char tmp[32];

    uint32_t pos  = 0;

    memset(oct, '0', size);

    if (value == 0)
    {
         return;
    }

    /* Fill temp buffer */
    while (value != 0)
    {
        tmp[pos++] = '0' + (value % 8);
        value /= 8;
    }

    while(pos != 0)
    {
        oct[size - pos - 1] = tmp[pos - 1];
        --pos;
    }

    /* NULL terminate */
    oct[size - 1] = 0;
}

static void ustar_get_next_file(const vfs_partition_t* partition,
                                ustar_block_t* block,
                                uint32_t* inode)
{
    OS_RETURN_E err;
    uint32_t    size;

    KERNEL_DEBUG(USTAR_DEBUG_ENABLED, "USTAR", "Current file %s",
                 block->file_name);

    /* We loop over all possible empty names (removed files) */
    do
    {
        err = ustar_check_block(block);
        if(err != OS_NO_ERR)
        {
            /* Block not found */
            block->file_name[0] = 0;
            return;
        }

        /* Next block is current block + file size / block_size */
        size = oct2uint(block->size, USTAR_FSIZE_FIELD_LENGTH - 1) +
               USTAR_BLOCK_SIZE;

        if(size % USTAR_BLOCK_SIZE != 0)
        {
            size += USTAR_BLOCK_SIZE;
        }

        *inode = *inode + size / USTAR_BLOCK_SIZE;

        /* Read the first 512 bytes (USTAR block) */
        err = ustar_access_blocks_from_device(partition,
                                            block,
                                            *inode,
                                            1,
                                            USTAR_DEV_OP_READ);


        if(err != OS_NO_ERR)
        {
            block->file_name[0] = 0;
            return;
        }
    }while(block->file_name[0] == 0);

    KERNEL_DEBUG(USTAR_DEBUG_ENABLED, "USTAR", "Next file %s",
                 block->file_name);
}

inline static void ustar_get_filename(const char* path, char* buffer)
{
    size_t len;

    len = strlen(path);

    if(path[len - 1] != '/')
    {
        --len;
        while(len > 0 && path[len] != '/')
        {
            --len;
        }
        if(len != 0)
        {
            ++len;
        }
    }
    else
    {
        len = 0;
    }
    strncpy(buffer, &path[len], USTAR_FILENAME_MAX_LENGTH - len);
}

inline static void ustar_get_path(const char* path, char* buffer)
{
    size_t len;

    len = strlen(path) - 1;

    --len;
    while(len > 0 && path[len] != '/')
    {
        --len;
    }
    if(len != 0)
    {
        ++len;
    }

    strncpy(buffer, path, len);
}

inline static bool_t is_root(const char* path, const char* file)
{
    char*  buffer;
    size_t len;
    bool_t ret_val;

    buffer = kmalloc(sizeof(char*) * strlen(file));
    if(buffer == NULL)
    {
        return FALSE;
    }
    memset(buffer, 0, strlen(file));

    ustar_get_path(file, buffer);

    len = strlen(path);

    ret_val = (len == strlen(buffer) &&
               strncmp(path, buffer, len) == 0);

    kfree(buffer);
    return ret_val;
}

inline static VFS_FILE_TYPE_E ustar_to_vfs_type(const char type)
{
    switch(type)
    {
        case'0':
            return VFS_FILE_TYPE_FILE;
        case'1':
            return VFS_FILE_TYPE_HARD_LINK;
        case'2':
            return VFS_FILE_TYPE_SYM_LINK;
        case'3':
            return VFS_FILE_TYPE_CHAR_DEV;
        case'4':
            return VFS_FILE_TYPE_BLOCK_DEV;
        case'5':
            return VFS_FILE_TYPE_DIR;
        case'6':
            return VFS_FILE_TYPE_NAMED_PIPE;
        default:
            return VFS_FILE_TYPE_OTHER;

    }
}

inline static vfs_access_right_t ustar_to_vfs_rights(const char* mode)
{
    vfs_access_right_t rights;
    uint32_t           uint_mode;

    uint_mode = oct2uint(mode, USTAR_MODE_FIELD_LENGTH);

    rights =  uint_mode & T_UREAD  ? VFS_RIGHTS_UREAD  : 0;
    rights |= uint_mode & T_UWRITE ? VFS_RIGHTS_UWRITE : 0;
    rights |= uint_mode & T_UEXEC  ? VFS_RIGHTS_UEXEC  : 0;
    rights |= uint_mode & T_GREAD  ? VFS_RIGHTS_GREAD  : 0;
    rights |= uint_mode & T_GWRITE ? VFS_RIGHTS_GWRITE : 0;
    rights |= uint_mode & T_GEXEC  ? VFS_RIGHTS_GEXEC  : 0;
    rights |= uint_mode & T_OREAD  ? VFS_RIGHTS_OREAD  : 0;
    rights |= uint_mode & T_OWRITE ? VFS_RIGHTS_OWRITE : 0;
    rights |= uint_mode & T_OEXEC  ? VFS_RIGHTS_OEXEC  : 0;

    return rights;
}

static void ustar_set_vnode(vfs_vnode_t* vnode,
                            ustar_block_t* block,
                            const uint32_t block_id,
                            const char* path)
{
    char filename[USTAR_FILENAME_MAX_LENGTH];

    /* We only have files with this FS */
    vnode->type = ustar_to_vfs_type(block->type);

    ustar_get_filename(path, filename);

    strncpy(vnode->name, filename, VFS_FILE_NAME_LENGTH);
    strncpy(vnode->short_name, filename, VFS_FILE_NAME_LENGTH);

    if(block->prefix[0] != '\0')
    {
        strncpy(vnode->path, block->prefix, USTAR_PREFIX_NAME_LENGTH);
    }
    else
    {
        strncpy(vnode->path, path, VFS_FILE_NAME_LENGTH);
        vnode->path[strlen(vnode->path) - strlen(vnode->name)] = 0;
    }

    vnode->access_rights = ustar_to_vfs_rights(block->mode);

    vnode->owner_id = oct2uint(block->user_id, USTAR_UID_FIELD_LENGTH);
    vnode->group_id = oct2uint(block->group_id, USTAR_UID_FIELD_LENGTH);

    strncpy(vnode->owner_name, block->user_name, VFS_OWNER_NAME_LENGTH);
    strncpy(vnode->group_name, block->group_name, VFS_GROUP_NAME_LENGTH);

    vnode->size = oct2uint(block->size, USTAR_FSIZE_FIELD_LENGTH);

    vnode->creation_datetime = oct2uint(block->last_edited,
                                        USTAR_LEDIT_FIELD_LENGTH);
    vnode->last_access_datetime = vnode->creation_datetime;
    vnode->last_modification_datetime = vnode->creation_datetime;

    /* With this FS, the inode is the block ID */
    vnode->fs_inode = (void*)block_id;
}

static OS_RETURN_E ustar_search_file(const vfs_partition_t* partition,
                                     const char* path,
                                     ustar_block_t* block,
                                     uint32_t* block_id)
{
    bool_t        found;
    OS_RETURN_E   err;

    /* USTAR max path is 100 character */
    if(strlen(path) > USTAR_FILENAME_MAX_LENGTH)
    {
        return OS_ERR_FILE_NOT_FOUND;
    }

    /* Read the first 512 bytes (USTAR block) */
    err = ustar_access_blocks_from_device(partition,
                                          block,
                                          0,
                                          1,
                                          USTAR_DEV_OP_READ);
    if(err != OS_NO_ERR)
    {
        return err;
    }

    err = ustar_check_block(block);
    if(err != OS_NO_ERR)
    {
        return err;
    }

    found    = FALSE;
    *block_id = 0;

    /* Search for the file, if first filename character is NULL, we reached the
     * end of the search
     */
    while(block->file_name[0] != 0)
    {
        if(strncmp(path,
                   block->file_name,
                   USTAR_FILENAME_MAX_LENGTH) == 0)
        {
            found = TRUE;
            err = ustar_check_block(block);
            if(err != OS_NO_ERR)
            {
                return err;
            }
            break;
        }
        ustar_get_next_file(partition, block, block_id);
    }

    if(found == TRUE)
    {
        KERNEL_DEBUG(USTAR_DEBUG_ENABLED, "USTAR", "Found file %s", path);
        return OS_NO_ERR;
    }
    else
    {
        KERNEL_DEBUG(USTAR_DEBUG_ENABLED, "USTAR", "Did not find file %s",
                     path);
        return OS_ERR_FILE_NOT_FOUND;
    }
}

static OS_RETURN_E ustar_get_item_count(const vfs_partition_t* partition,
                                        const char* path,
                                        size_t* item_count)
{
    OS_RETURN_E   err;
    size_t        dir_path_length;
    ustar_block_t block;
    uint32_t      block_id;

    *item_count  = 0;

    /* Search for the folder */
    err = ustar_search_file(partition, path, &block, &block_id);

    if(err != OS_NO_ERR)
    {
        return err;
    }

    if(ustar_to_vfs_type(block.type) != VFS_FILE_TYPE_DIR)
    {
        return OS_ERR_FILE_NOT_FOUND;
    }

    dir_path_length = strlen(path);

    /* Search for the file, if first filename character is NULL, we reached the
     * end of the search
     */
    while(block.file_name[0] != 0)
    {
        err = ustar_check_block(&block);
        if(err != OS_NO_ERR)
        {
            return err;
        }

        /* Is path is equal and is not the exact dir we are checking */
        if(strncmp(block.file_name,
                   path,
                   dir_path_length) == 0 &&
           strlen(block.file_name) != dir_path_length)
        {
            ++*item_count;
        }
        ustar_get_next_file(partition, &block, &block_id);
    }

    return OS_NO_ERR;
}

OS_RETURN_E ustar_mount(vfs_partition_t* partition,
                        const char* mount_pt)
{
    ustar_block_t block;
    OS_RETURN_E   err;

    /* We don't use mount_pt at the moment */
    (void)mount_pt;

    if(partition == NULL || mount_pt == NULL)
    {
        return OS_ERR_NULL_POINTER;
    }

    /* Check if the device is compatible */
    if((partition->device->block_size > USTAR_BLOCK_SIZE &&
        partition->device->block_size % USTAR_BLOCK_SIZE != 0) ||
       (partition->device->block_size < USTAR_BLOCK_SIZE &&
        USTAR_BLOCK_SIZE % partition->device->block_size != 0))
    {
        return OS_ERR_ALIGN;
    }

    /* First we need to check if the partition is of the correct format, for
     * the USART, this is pretty straight forward, we need to check if the
     * first block is of usart type. There might be previous entries but we
     * don't check them */

    /* Read the first 512 bytes (USTAR block) */
    err = ustar_access_blocks_from_device(partition,
                                          &block,
                                          0,
                                          1,
                                          USTAR_DEV_OP_READ);
    if(err != OS_NO_ERR)
    {
        return err;
    }

    return ustar_check_block(&block);
}

OS_RETURN_E ustar_umount(vfs_partition_t* partition)
{
    ustar_block_t block;
    OS_RETURN_E   err;

    /* First we need to check if the partition is of the correct format, for
     * the USART, this is pretty straight forward, we need to check if the
     * first block is of usart type. There might be previous entries but we
     * don't check them */

    /* Read the first 512 bytes (USTAR block) */
    err = ustar_access_blocks_from_device(partition,
                                          &block,
                                          0,
                                          1,
                                          USTAR_DEV_OP_READ);
    if(err != OS_NO_ERR)
    {
        return err;
    }

    err = ustar_check_block(&block);
    if(err != OS_NO_ERR)
    {
        return err;
    }

    /* Flush the partition */
    return ustar_access_blocks_from_device(partition,
                                           &block,
                                           0,
                                           1,
                                           USTAR_DEV_OP_FLUSH);
}

OS_RETURN_E ustar_open_file(const char* path,
                            vfs_vnode_t* vnode)
{
    ustar_block_t current_block;
    bool_t        found;
    OS_RETURN_E   err;
    uint32_t      block_id;

    if(vnode == NULL || path == NULL)
    {
        return OS_ERR_NULL_POINTER;
    }

    /* USTAR max path is 100 character */
    if(strlen(path) > USTAR_FILENAME_MAX_LENGTH)
    {
        return OS_ERR_FILE_NOT_FOUND;
    }

    KERNEL_DEBUG(USTAR_DEBUG_ENABLED, "USTAR", "Opening %s", path);

    /* Read the first 512 bytes (USTAR block) */
    err = ustar_access_blocks_from_device(vnode->partition,
                                          &current_block,
                                          0,
                                          1,
                                          USTAR_DEV_OP_READ);
    if(err != OS_NO_ERR)
    {
        return err;
    }

    err = ustar_check_block(&current_block);
    if(err != OS_NO_ERR)
    {
        return err;
    }

    found    = FALSE;
    block_id = 0;

    /* Search for the file, if first filename character is NULL, we reached the
     * end of the search
     */
    while(current_block.file_name[0] != 0)
    {
        KERNEL_DEBUG(USTAR_DEBUG_ENABLED, "USTAR", "Checking %s",
                     current_block.file_name);

        if(strncmp(path,
                   current_block.file_name,
                   USTAR_FILENAME_MAX_LENGTH) == 0)
        {
            found = TRUE;
            err = ustar_check_block(&current_block);
            if(err != OS_NO_ERR)
            {
                return err;
            }

            /* For the USTAR, the inode is just the block id of the current
             * partition */
            ustar_set_vnode(vnode, &current_block, block_id, path);
            break;
        }
        ustar_get_next_file(vnode->partition, &current_block, &block_id);
    }

    if(found == TRUE)
    {
        err = OS_NO_ERR;
    }
    else
    {
        err = OS_ERR_FILE_NOT_FOUND;
    }

    return err;
}

OS_RETURN_E ustar_close_file(const vfs_vnode_t* vnode)
{
    ustar_block_t block;
    OS_RETURN_E   err;
    uint32_t      block_count;

    if(vnode == NULL)
    {
        return OS_ERR_NULL_POINTER;
    }

    /* Check the inode format */
    err = ustar_access_blocks_from_device(vnode->partition,
                                          &block,
                                          (const uint32_t)vnode->fs_inode,
                                          1,
                                          USTAR_DEV_OP_READ);
    if(err != OS_NO_ERR)
    {
        return err;
    }

    err = ustar_check_block(&block);
    if(err != OS_NO_ERR)
    {
        return err;
    }

    /* Flush the files data */
    block_count = oct2uint(block.size, USTAR_FSIZE_FIELD_LENGTH);
    if(block_count % USTAR_BLOCK_SIZE != 0)
    {
        block_count = block_count / USTAR_BLOCK_SIZE + 1;
    }
    else
    {
        block_count = block_count / USTAR_BLOCK_SIZE;
    }
    return ustar_access_blocks_from_device(vnode->partition,
                                           NULL,
                                           (const uint32_t)vnode->fs_inode,
                                           block_count,
                                           USTAR_DEV_OP_FLUSH);
}

OS_RETURN_E ustar_read_file(const vfs_vnode_t* vnode,
                            void* buffer,
                            size_t size,
                            size_t* actual_size)
{
    ustar_block_t block;
    OS_RETURN_E   err;
    size_t        file_size;
    size_t        blocks_to_read;
    size_t        offset;
    uint32_t      real_inode;
    char*         block_buffer;

    if(vnode == NULL || buffer == NULL)
    {
        return OS_ERR_NULL_POINTER;
    }

    /* Check the inode format */
    err =  ustar_access_blocks_from_device(vnode->partition,
                                           &block,
                                           (const uint32_t)vnode->fs_inode,
                                           1,
                                           USTAR_DEV_OP_READ);
    if(err != OS_NO_ERR)
    {
        return err;
    }

    err = ustar_check_block(&block);
    if(err != OS_NO_ERR)
    {
        return err;
    }

    if(ustar_to_vfs_type(block.type) != VFS_FILE_TYPE_FILE)
    {
        return OS_ERR_FILE_NOT_FOUND;
    }

    /* Read until we reach the end of the file or the read size */
    file_size = oct2uint(block.size, USTAR_FSIZE_FIELD_LENGTH);
    offset = vnode->cursor;

    if(offset >= file_size || size == 0)
    {
        if(actual_size != NULL)
        {
            *actual_size = 0;
        }
        return OS_NO_ERR;
    }

    /* Can't read more than the size of the file */
    if(offset + size > file_size)
    {
        size = file_size - offset;
    }

    /* Check the number of blocks to read */
    blocks_to_read = size / USTAR_BLOCK_SIZE;
    if(size % USTAR_BLOCK_SIZE != 0)
    {
        ++blocks_to_read;
    }
    /* If we are overlaping on two blocks because of offset */
    if(offset % USTAR_BLOCK_SIZE + size % USTAR_BLOCK_SIZE > USTAR_BLOCK_SIZE)
    {
        ++blocks_to_read;
    }

    /* Update to first data inode */
    real_inode = (uint32_t)vnode->fs_inode + 1;
    real_inode += offset / USTAR_BLOCK_SIZE;
    offset     = offset % USTAR_BLOCK_SIZE;

    /* First we read entier blocks */
    block_buffer = kmalloc(blocks_to_read * USTAR_BLOCK_SIZE);
    if(block_buffer == NULL)
    {
        return OS_ERR_MALLOC;
    }

    err = ustar_access_blocks_from_device(vnode->partition,
                                          block_buffer,
                                          real_inode,
                                          blocks_to_read,
                                          USTAR_DEV_OP_READ);
    if(err != OS_NO_ERR)
    {
        kfree(block_buffer);
        return err;
    }

    /* Move the data */
    memcpy(buffer, block_buffer + offset, size);

    if(actual_size != NULL)
    {
        *actual_size = size;
    }

    kfree(block_buffer);
    return err;
}

OS_RETURN_E ustar_write_file(const vfs_vnode_t* vnode,
                             const void* buffer,
                             size_t size,
                             size_t* actual_size)
{
    ustar_block_t block;
    OS_RETURN_E   err;
    size_t        file_size;
    size_t        blocks_to_write;
    size_t        offset;
    size_t        write;
    uint32_t      real_inode;

    if(vnode == NULL || buffer == NULL)
    {
        return OS_ERR_NULL_POINTER;
    }

    /* Check the inode format */
    err =  ustar_access_blocks_from_device(vnode->partition,
                                           &block,
                                           (const uint32_t)vnode->fs_inode,
                                           1,
                                           USTAR_DEV_OP_READ);
    if(err != OS_NO_ERR)
    {
        return err;
    }

    err = ustar_check_block(&block);
    if(err != OS_NO_ERR)
    {
        return err;
    }

    if(ustar_to_vfs_type(block.type) != VFS_FILE_TYPE_FILE)
    {
        return OS_ERR_FILE_NOT_FOUND;
    }

    /* Write until we reach the end of the file or the write size */
    file_size = oct2uint(block.size, USTAR_FSIZE_FIELD_LENGTH);
    offset = vnode->cursor;

    if(offset >= file_size || size == 0)
    {
        if(actual_size != NULL)
        {
            *actual_size = 0;
        }
        return OS_NO_ERR;
    }

    /* Can't write more than the size of the file */
    if(offset + size > file_size)
    {
        size = file_size - offset;
    }

    /* Update to first data inode */
    real_inode = (uint32_t)vnode->fs_inode + 1;
    real_inode += offset / USTAR_BLOCK_SIZE;
    offset     = offset % USTAR_BLOCK_SIZE;

    /* If the offset is not null, we need to read the first block and edit it */
    write = 0;
    if(offset != 0)
    {
        err = ustar_access_blocks_from_device(vnode->partition,
                                            &block,
                                            real_inode,
                                            1,
                                            USTAR_DEV_OP_READ);
        if(err != OS_NO_ERR)
        {
            return err;
        }

        write = MIN(USTAR_BLOCK_SIZE - offset, size);

        memcpy(((char*)&block) + offset, buffer, write);

        err = ustar_access_blocks_from_device(vnode->partition,
                                            &block,
                                            real_inode,
                                            1,
                                            USTAR_DEV_OP_WRITE);
        if(err != OS_NO_ERR)
        {
            return err;
        }
        size -= write;
        ++real_inode;
    }

    /* Write full blocks (overwrite completely) */
    if(size > USTAR_BLOCK_SIZE)
    {
        blocks_to_write = size / USTAR_BLOCK_SIZE;
        err = ustar_access_blocks_from_device(vnode->partition,
                                            (char*)buffer + write,
                                            real_inode,
                                            blocks_to_write,
                                            USTAR_DEV_OP_WRITE);
        if(err != OS_NO_ERR)
        {
            if(actual_size != NULL)
            {
                *actual_size = write;
            }
            return err;
        }
        write += blocks_to_write * USTAR_BLOCK_SIZE;
        size  -= blocks_to_write * USTAR_BLOCK_SIZE;
        real_inode += blocks_to_write;
    }

    /* Write last block if needed, we need to read it to edit it */
    if(size != 0)
    {
        err = ustar_access_blocks_from_device(vnode->partition,
                                              &block,
                                              real_inode,
                                              1,
                                              USTAR_DEV_OP_READ);
        if(err != OS_NO_ERR)
        {
            if(actual_size != NULL)
            {
                *actual_size = write;
            }
            return err;
        }

        memcpy(&block, (char*)buffer + write, size);

        err = ustar_access_blocks_from_device(vnode->partition,
                                              &block,
                                              real_inode,
                                              1,
                                              USTAR_DEV_OP_WRITE);
        if(err != OS_NO_ERR)
        {
            if(actual_size != NULL)
            {
                *actual_size = write;
            }
            return err;
        }
        write += size;
    }

    if(actual_size != NULL)
    {
        *actual_size = write;
    }

    if(write != 0)
    {
        /* Update last edited */
        uint2oct(block.last_edited,
                 vnode->last_modification_datetime,
                 USTAR_LEDIT_FIELD_LENGTH);
    }

    return err;
}

OS_RETURN_E ustar_create_file(const char* path,
                              const VFS_FILE_TYPE_E type,
                              const vfs_access_right_t access_rights,
                              const char* owner_name,
                              const char* group_name)
{
    (void)path;
    (void)type;
    (void)access_rights;
    (void)owner_name;
    (void)group_name;
    return OS_ERR_NOT_SUPPORTED;
}

OS_RETURN_E ustar_remove_file(const vfs_partition_t* part, const char* path)
{
    ustar_block_t current_block;
    OS_RETURN_E   err;
    uint32_t      block_id;
    size_t        item_counts;

    if(part == NULL || path == NULL)
    {
        return OS_ERR_NULL_POINTER;
    }

    err = ustar_search_file(part, path, &current_block, &block_id);
    KERNEL_DEBUG(USTAR_DEBUG_ENABLED, "USTAR", "Deleteing file %s at %d",
                 path, block_id);

    if(err == OS_NO_ERR)
    {
        /* Check that it is a file, if folder, check that it is empty */
        if(ustar_to_vfs_type(current_block.type) == VFS_FILE_TYPE_DIR)
        {
            err = ustar_get_item_count(part, path, &item_counts);
            if(err != OS_NO_ERR)
            {
                return err;
            }

            if(item_counts != 0)
            {
                return OS_ERR_DIR_NOT_EMPTY;
            }
        }

        /* Set the first file block to NULL */
        memset(&current_block.file_name, 0, USTAR_FILENAME_MAX_LENGTH);
        err = ustar_access_blocks_from_device(part,
                                              &current_block,
                                              block_id,
                                              1,
                                              USTAR_DEV_OP_WRITE);
        if(err != OS_NO_ERR)
        {
            return err;
        }
        return ustar_access_blocks_from_device(part,
                                               NULL,
                                               block_id,
                                               1,
                                               USTAR_DEV_OP_FLUSH);
    }
    else
    {
        return err;
    }
}

OS_RETURN_E ustar_rename_file(const vfs_partition_t* partition,
                              const char* old_path,
                              const char* new_path)
{
    ustar_block_t current_block;
    OS_RETURN_E   err;
    uint32_t      block_id;
    size_t        path_len;
    char          short_path[USTAR_FILENAME_MAX_LENGTH];
    char          old_short_path[USTAR_FILENAME_MAX_LENGTH];
    char*         name_table;
    size_t        item_count;
    size_t        table_size;
    size_t        old_path_len;
    vfs_vnode_t   node;
    uint32_t      i;

    if(partition == NULL || old_path == NULL || new_path == NULL)
    {
        return OS_ERR_NULL_POINTER;
    }

    /* USTAR max path is 100 character */
    path_len = strlen(new_path);
    old_path_len = strlen(old_path);
    if(path_len > USTAR_FILENAME_MAX_LENGTH)
    {
        return OS_ERR_FILE_NOT_FOUND;
    }

    /* Check if path already exists */
    memset(short_path, 0, sizeof(char) * path_len);
    ustar_get_path(new_path, short_path);
    if(strlen(short_path) != 0)
    {
        err = ustar_search_file(partition,
                                short_path,
                                &current_block,
                                &block_id);
        if(err != OS_NO_ERR ||
        ustar_to_vfs_type(current_block.type) != VFS_FILE_TYPE_DIR)
        {
            return OS_ERR_FILE_NOT_FOUND;
        }
    }

    err = ustar_search_file(partition,
                            old_path,
                            &current_block,
                            &block_id);

    if(err == OS_NO_ERR)
    {
        /* Check that if folder, try to rename all files in the folder */
        if(ustar_to_vfs_type(current_block.type) == VFS_FILE_TYPE_DIR)
        {
            if(path_len > 0 && new_path[path_len - 1] != '/')
            {
                return OS_ERR_FILE_NOT_FOUND;
            }

            node.partition = partition;
            err = ustar_open_file(old_path, &node);
            if(err != OS_NO_ERR)
            {
                return err;
            }
            err = ustar_get_item_count(partition, old_path, &item_count);
            if(err != OS_NO_ERR)
            {
                return err;
            }
            table_size = sizeof(char) * item_count *
                         (USTAR_FILENAME_MAX_LENGTH + 1);
            name_table = kmalloc(sizeof(char) * table_size);
            err = ustar_list_directory(&node, &item_count,
                                       name_table, table_size);
            if(err != OS_NO_ERR)
            {
                kfree(name_table);
                return err;
            }

            /* Check if we can rename all the files */
            if(path_len > old_path_len)
            {
                i = 0;
                table_size = item_count;
                while(table_size > 0)
                {
                    if(path_len + strlen(name_table + i) >
                    USTAR_FILENAME_MAX_LENGTH)
                    {
                        kfree(name_table);
                        return OS_ERR_NAME_TOO_LONG;
                    }
                    i += strlen(name_table + i) + 1;
                    --table_size;
                }
            }

            /* Set the first file block names */
            strncpy(current_block.file_name,
                    new_path,
                    MIN(path_len, USTAR_FILENAME_MAX_LENGTH));
            current_block.file_name[path_len] = '\0';
            err = ustar_access_blocks_from_device(partition,
                                                &current_block,
                                                block_id,
                                                1,
                                                USTAR_DEV_OP_WRITE);
            if(err != OS_NO_ERR)
            {
                return err;
            }

            /* Rename all files */
            i = 0;
            table_size = item_count;

            strncpy(short_path, new_path, path_len);
            strncpy(old_short_path, old_path, old_path_len);

            while(table_size)
            {

                /* Concatenate the name with the new path */
                memset(short_path + path_len, 0,
                       USTAR_FILENAME_MAX_LENGTH - path_len);
                memset(old_short_path + old_path_len, 0,
                       USTAR_FILENAME_MAX_LENGTH - old_path_len);
                strncpy(short_path + path_len, name_table + i,
                        strlen(name_table + i) + 1);
                strncpy(old_short_path + old_path_len, name_table + i,
                        strlen(name_table + i) + 1);

                KERNEL_DEBUG(USTAR_DEBUG_ENABLED, "USTAR", "Renaming %s to %s",
                             old_short_path, short_path);

                err = ustar_rename_file(partition, old_short_path, short_path);
                USTAR_ASSERT(err == OS_NO_ERR,
                             "Error while renaming regular file.",
                             err);

                i += strlen(name_table + i) + 1;
                --table_size;
            }

            kfree(name_table);
        }
        else
        {
            /* Set the first file block names */
            strncpy(current_block.file_name,
                    new_path,
                    MIN(path_len, USTAR_FILENAME_MAX_LENGTH));
            current_block.file_name[path_len] = '\0';
            err = ustar_access_blocks_from_device(partition,
                                                &current_block,
                                                block_id,
                                                1,
                                                USTAR_DEV_OP_WRITE);
            if(err != OS_NO_ERR)
            {
                return err;
            }
        }

        return ustar_access_blocks_from_device(partition,
                                               NULL,
                                               block_id,
                                               1,
                                               USTAR_DEV_OP_FLUSH);
    }
    else
    {
        return err;
    }
}

OS_RETURN_E ustar_truncate_file(const vfs_partition_t* partition,
                                const char* path,
                                const size_t new_size)
{
    ustar_block_t current_block;
    OS_RETURN_E   err;
    uint32_t      block_id;

    if(partition == NULL || path == NULL)
    {
        return OS_ERR_NULL_POINTER;
    }

    err = ustar_search_file(partition, path, &current_block, &block_id);

    if(err == OS_NO_ERR)
    {
        if(ustar_to_vfs_type(current_block.type) != VFS_FILE_TYPE_FILE)
        {
            return OS_ERR_FILE_NOT_FOUND;
        }

        /* USTAR cannot grow files larger */
        if(new_size > oct2uint(current_block.size, USTAR_FSIZE_FIELD_LENGTH))
        {
            return OS_ERR_UNAUTHORIZED_ACTION;
        }
        /* Set the new size */
        uint2oct(current_block.size, new_size, USTAR_FSIZE_FIELD_LENGTH);
        KERNEL_DEBUG(USTAR_DEBUG_ENABLED, "USTAR", "Truncated file to %u",
                     current_block.size);

        return ustar_access_blocks_from_device(partition,
                                               &current_block,
                                               block_id,
                                               1,
                                               USTAR_DEV_OP_WRITE);
    }
    else
    {
        return err;
    }
}

OS_RETURN_E ustar_list_directory(const vfs_vnode_t* vnode,
                                 size_t* item_count,
                                 char* buffer,
                                 const size_t buff_size)
{
    OS_RETURN_E   err;
    size_t        size_left;
    size_t        file_name_length;
    size_t        dir_path_length;
    ustar_block_t block;
    uint32_t      block_id;

    if(buffer == NULL || item_count == NULL)
    {
        return OS_ERR_NULL_POINTER;
    }

    size_left   = buff_size;
    *item_count  = 0;

    /* Read the first 512 bytes (USTAR block) */
    err = ustar_access_blocks_from_device(vnode->partition,
                                          &block,
                                          (uint32_t)vnode->fs_inode,
                                          1,
                                          USTAR_DEV_OP_READ);
    if(err != OS_NO_ERR)
    {
        return err;
    }

    err = ustar_check_block(&block);
    if(err != OS_NO_ERR)
    {
        return err;
    }

    block_id = (uint32_t)vnode->fs_inode;

    if(ustar_to_vfs_type(block.type) != VFS_FILE_TYPE_DIR)
    {
        return OS_ERR_FILE_NOT_FOUND;
    }

    dir_path_length = strlen(vnode->name);

    /* Search for the file, if first filename character is NULL, we reached the
     * end of the search
     */
    while(block.file_name[0] != 0)
    {
        err = ustar_check_block(&block);
        if(err != OS_NO_ERR)
        {
            return err;
        }
        file_name_length = strlen(block.file_name);
        if(strncmp(block.file_name,
                   vnode->name,
                   dir_path_length) == 0)
        {
            /* If self or file in folder skip */
            if(file_name_length != dir_path_length &&
               is_root(vnode->name, block.file_name))
            {
                if(size_left > file_name_length + 1)
                {
                    strncpy(&buffer[buff_size - size_left],
                            block.file_name + dir_path_length,
                            file_name_length - dir_path_length);

                    size_left -= file_name_length - dir_path_length;
                    buffer[buff_size - size_left] = '\0';
                    --size_left;
                    ++*item_count;
                }
                else
                {
                    buffer[size_left] = 0;
                    return OS_ERR_OUT_OF_BOUND;
                }
            }
        }
        ustar_get_next_file(vnode->partition, &block, &block_id);
    }

    if(size_left < buff_size)
    {
        buffer[buff_size - size_left - 1] = 0;
    }

    return OS_NO_ERR;
}

/************************************ EOF *************************************/