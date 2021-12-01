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

#include <kernel_error.h>  /* Kernel error codes */
#include <stdint.h>        /* Generic int types */
#include <string.h>        /* String and momeory anipulation */
#include <kernel_output.h> /* Kernel output */

/* UTK configuration file */
#include <config.h>

/* Tests header file */
#ifdef TEST_MODE_ENABLED
#include <test_bank.h>
#endif

/* Header file */
#include <ustar_fs.h>

/*******************************************************************************
 * CONSTANTS
 ******************************************************************************/

#define USTAR_MAGIC "ustar "
#define USTAR_FILENAME_MAX_LENGTH 100
#define USTAR_BLOCK_SIZE 512
#define USTAR_FSIZE_FIELD_LENGTH 12
#define USTAR_LEDIT_FIELD_LENGTH 12
#define USTAR_UID_FIELD_LENGTH 8
#define USTAR_MODE_FIELD_LENGTH 8
#define USTAR_PREFIX_NAME_LENGTH 155

#define T_UREAD  0x100
#define T_UWRITE 0x080
#define T_UEXEC  0x040

#define T_GREAD  0x020
#define T_GWRITE 0x010
#define T_GEXEC  0x008

#define T_OREAD  0x004
#define T_OWRITE 0x002
#define T_OEXEC  0x001


/*******************************************************************************
 * STRUCTURES
 ******************************************************************************/

struct ustar_block
{
    char file_name[USTAR_FILENAME_MAX_LENGTH];
    char mode[8];
    char user_id[8];
    char group_id[8];
    char size[USTAR_FSIZE_FIELD_LENGTH];
    char last_edited[12];
    char checksum[8];
    char type;
    char linked_file_name[USTAR_FILENAME_MAX_LENGTH];
    char magic[6];
    char ustar_version[2];
    char user_name[32];
    char group_name[32];
    char dev_major[8];
    char dev_minor[8];
    char prefix[USTAR_PREFIX_NAME_LENGTH];
    char padding[12];
};
typedef struct ustar_block ustar_block_t;

enum USTAR_DEV_OP
{
    USTAR_DEV_OP_WRITE,
    USTAR_DEV_OP_READ,
    USTAR_DEV_OP_FLUSH
};

typedef enum USTAR_DEV_OP USTAR_DEV_OP_E;
/*******************************************************************************
 * GLOBAL VARIABLES
 ******************************************************************************/

/* None */

/*******************************************************************************
 * STATIC FUNCTIONS DECLARATION
 ******************************************************************************/

/* None */

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

    KERNEL_DEBUG(USTAR_DEBUG_ENABLED, "Reading blocks 0x%p, (%d blocks)", 
                 inode, block_counts);

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
    uint32_t opos = 0;
    uint32_t top  = 0;

    memset(oct, 0, size);

    if (value == 0)
    {
         *oct++ = '0';
         *oct = '\0';
         return;
     }

    /* Fill temp buffer */
    while (value != 0)
    {
        tmp[pos++] = '0' + (value % 8);
        value /= 8;
    }

    top = pos--;
    /* Fill buffer */
    for (opos = 0; opos < top && opos > size; --pos, ++opos)
    {
        oct[opos] = tmp[pos];
    }

    /* Null termitate */
    oct[opos] = 0;
}

static void ustar_get_next_file(const vfs_partition_t* partition,
                                ustar_block_t* block, 
                                uint32_t* inode)
{
    OS_RETURN_E err;
    uint32_t    size;

    KERNEL_DEBUG(USTAR_DEBUG_ENABLED, "Current file %s", block->file_name);

    err = ustar_check_block(block);
    if(err != OS_NO_ERR)
    {
        /* Block not found */
        block->file_name[0] = 0;
        return;
    }

    /* Next block is current block + file size / block_size */
    size = oct2uint(block->size, USTAR_FSIZE_FIELD_LENGTH - 1) + USTAR_BLOCK_SIZE;

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
    KERNEL_DEBUG(USTAR_DEBUG_ENABLED, "Next file %s", block->file_name);
}

inline static void ustar_get_filename(const char* path, char* buffer)
{
    int32_t len;

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

inline static void ustar_set_vnode(vfs_vnode_t* vnode, 
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

inline static OS_RETURN_E ustar_search_file(const vfs_vnode_t* vnode,
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
    err = ustar_access_blocks_from_device(vnode->partition, 
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
        ustar_get_next_file(vnode->partition, block, block_id);
    }

    if(found == TRUE)
    {
        return OS_NO_ERR;
    }
    else 
    {
        return OS_ERR_FILE_NOT_FOUND;
    }
}

OS_RETURN_E ustar_mount(vfs_partition_t* partition,
                        const char* mount_pt)
{
    ustar_block_t block;
    OS_RETURN_E   err;

    /* We don't use mount_pt at the moment */
    (void)mount_pt;

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
    
    return ustar_check_block(&block);
}

OS_RETURN_E ustar_open_file(const char* path, 
                            vfs_vnode_t* vnode)
{
    ustar_block_t current_block;
    bool_t        found;
    OS_RETURN_E   err;
    uint32_t      block_id;

    if(vnode == NULL)
    {
        return OS_ERR_NULL_POINTER;
    }

    /* USTAR max path is 100 character */
    if(strlen(path) > USTAR_FILENAME_MAX_LENGTH)
    {
        return OS_ERR_FILE_NOT_FOUND;
    }

    KERNEL_DEBUG(USTAR_DEBUG_ENABLED, "Opening %s", path);

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
        KERNEL_DEBUG(USTAR_DEBUG_ENABLED, "Checking %s", current_block.file_name);
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
    size_t        read;
    size_t        block_read;
    size_t        blocks_to_read;
    size_t        offset;
    uint32_t      real_inode;

    if(vnode == NULL)
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

    /* Read until we reach the end of the file or the read size */
    file_size = oct2uint(block.size, USTAR_FSIZE_FIELD_LENGTH);
    offset = vnode->cursor;

    if(offset >= file_size || size == 0)
    {
        *actual_size = 0;
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
    while(offset >= USTAR_BLOCK_SIZE)
    {
        ++real_inode;
        offset -= USTAR_BLOCK_SIZE;
    }
    
    read = 0;

    /* First we read entier blocks */
    while(blocks_to_read > 0)
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

        block_read = USTAR_BLOCK_SIZE - offset;
        block_read = MIN(block_read, size);

        /* The first time we have an offset, move the data */
        memcpy(buffer, ((char*)&block) + offset, block_read);
        offset = 0;

        read += block_read;
        size -= block_read;
        buffer += block_read;
        ++real_inode;    
        --blocks_to_read;    
    }
    
    if(actual_size != NULL)
    {
        *actual_size = read;
    }

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
    size_t        write;
    uint32_t      real_inode;
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

    /* TODO: Check that it is a file */

    /* Update last edited */
    uint2oct(block.last_edited, 
             vnode->last_modification_datetime, 
             USTAR_LEDIT_FIELD_LENGTH);
    
    err = ustar_access_blocks_from_device(vnode->partition, 
                                          &block,
                                          (const uint32_t)vnode->fs_inode, 
                                          1, 
                                          USTAR_DEV_OP_WRITE);
    if(err != OS_NO_ERR)
    {
        return err;
    }

    /* Write until we reach the end of the file or the Write size */
    file_size = oct2uint(block.size, USTAR_FSIZE_FIELD_LENGTH);
    if(size > file_size)
    {
        size = file_size;
    }

    /* Update to first data inode */
    real_inode = (uint32_t)vnode->fs_inode;
    ++real_inode;
    
    write = 0;

    /* First we write entier blocks */
    if(size >= USTAR_BLOCK_SIZE)
    {
        block_count = size / USTAR_BLOCK_SIZE;
        err = ustar_access_blocks_from_device(vnode->partition, 
                                              (void*)buffer,
                                              real_inode, 
                                              block_count, 
                                              USTAR_DEV_OP_WRITE);
        if(err != OS_NO_ERR)
        {
            return err;
        }
        write += block_count * USTAR_BLOCK_SIZE;
        size -= write;
        buffer += write;
        real_inode += block_count;        
    }

    /* Write the last block */
    if(size > 0)
    {
        memcpy(&block, buffer, size);
        err = ustar_access_blocks_from_device(vnode->partition, 
                                              &block,
                                              real_inode, 
                                              1, 
                                              USTAR_DEV_OP_WRITE);
        if(err != OS_NO_ERR)
        {
            return err;
        }
        write += size;
    }
    
    if(actual_size != NULL)
    {
        *actual_size = write;
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

OS_RETURN_E ustar_remove_file(const vfs_vnode_t* vnode, const char* path)
{
    ustar_block_t current_block;
    OS_RETURN_E   err;
    uint32_t      block_id;

    if(vnode == NULL)
    {
        return OS_ERR_NULL_POINTER;
    }

    err = ustar_search_file(vnode, path, &current_block, &block_id);

    if(err == OS_NO_ERR)
    {
        /* TODO: Check that it is a file, if forlder, check that it is empty */
        
        /* Set the first file block to NULL */
        memset(&current_block, 0, USTAR_BLOCK_SIZE);
        err = ustar_access_blocks_from_device(vnode->partition, 
                                              &current_block,
                                              block_id, 
                                              1, 
                                              USTAR_DEV_OP_WRITE);
        if(err != OS_NO_ERR)
        {
            return err;
        }
        return ustar_access_blocks_from_device(vnode->partition, 
                                               NULL,
                                               (const uint32_t)vnode->fs_inode, 
                                               1, 
                                               USTAR_DEV_OP_FLUSH);
    }
    else 
    {
        return OS_ERR_FILE_NOT_FOUND;
    }
}

OS_RETURN_E ustar_rename_file(const vfs_vnode_t* vnode,
                              const char* old_path,
                              const char* new_path)
{
    ustar_block_t current_block;
    OS_RETURN_E   err;
    uint32_t      block_id;

    if(vnode == NULL)
    {
        return OS_ERR_NULL_POINTER;
    }

    /* USTAR max path is 100 character */
    if(strlen(new_path) > USTAR_FILENAME_MAX_LENGTH)
    {
        return OS_ERR_FILE_NOT_FOUND;
    }

    /* TODO: Check that it is a file, if folder, try to rename all files 
       in the folder */

    err = ustar_search_file(vnode, old_path, &current_block, &block_id);

    if(err == OS_NO_ERR)
    {
        /* Set the first file block to NULL */
        strncpy(current_block.file_name, new_path, VFS_FILE_NAME_LENGTH);
        err = ustar_access_blocks_from_device(vnode->partition, 
                                              &current_block,
                                              block_id, 
                                              1, 
                                              USTAR_DEV_OP_WRITE);
        if(err != OS_NO_ERR)
        {
            return err;
        }
        return ustar_access_blocks_from_device(vnode->partition, 
                                               NULL,
                                               (const uint32_t)vnode->fs_inode, 
                                               1, 
                                               USTAR_DEV_OP_FLUSH);
    }
    else 
    {
        return err;
    }
}

OS_RETURN_E ustar_truncate_file(const vfs_vnode_t* vnode,
                                const char* path,
                                const size_t new_size)
{
    ustar_block_t current_block;
    OS_RETURN_E   err;
    uint32_t      block_id;

    if(vnode == NULL)
    {
        return OS_ERR_NULL_POINTER;
    }

    err = ustar_search_file(vnode, path, &current_block, &block_id);

    /* TODO: Check that it is a file */

    if(err == OS_NO_ERR)
    {
        /* USTAR cannot gros files larger */
        if(new_size > oct2uint(current_block.size, USTAR_FSIZE_FIELD_LENGTH))
        {
            return OS_ERR_UNAUTHORIZED_ACTION;
        }
        /* Set the first file block to NULL */
        uint2oct(current_block.size, new_size, VFS_FILE_NAME_LENGTH);
        err = ustar_access_blocks_from_device(vnode->partition, 
                                              &current_block,
                                              block_id, 
                                              1, 
                                              USTAR_DEV_OP_WRITE);
        if(err != OS_NO_ERR)
        {
            return err;
        }
        return ustar_access_blocks_from_device(vnode->partition, 
                                               NULL,
                                               (const uint32_t)vnode->fs_inode, 
                                               1, 
                                               USTAR_DEV_OP_FLUSH);
    }
    else 
    {
        return err;
    }
}

OS_RETURN_E ustar_list_directory(const vfs_vnode_t* vnode, 
                                 char* buffer, 
                                 const size_t buff_size)
{
    OS_RETURN_E   err;
    size_t        size_left;
    size_t        file_name_length;
    size_t        dir_path_length;
    ustar_block_t block;
    uint32_t      block_id;

    size_left   = buff_size;

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
            if(size_left > file_name_length + 1)
            {
                strncpy(&buffer[buff_size - size_left], 
                        block.file_name,  
                        file_name_length);

                size_left -= file_name_length;
                buffer[buff_size - size_left] = ';';
                --size_left;
            }
            else 
            {
                buffer[size_left] = 0;
                return OS_ERR_OUT_OF_BOUND;
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