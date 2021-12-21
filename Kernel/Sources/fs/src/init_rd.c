/*******************************************************************************
 * @file init_rd.c
 *
 * @see init_rd.h
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

#include <kernel_error.h>  /* Kernel error codes */
#include <stdint.h>        /* Generic int types */
#include <stddef.h>        /* Standard definitions */
#include <string.h>        /* Memory manipulation */
#include <kernel_output.h> /* Kernel output mathods */

/* UTK configuration file */
#include <config.h>

/* Tests header file */
#include <test_bank.h>

/* Header file */
#include <init_rd.h>

/*******************************************************************************
 * CONSTANTS
 ******************************************************************************/

#define UTK_INITRD_MAGIC 0x4452494E494B5455ULL /* UTKINIRD */

/*******************************************************************************
 * STRUCTURES
 ******************************************************************************/

struct initrd_master_block
{
    uint64_t magic;

    uint32_t size;

    uint8_t padding[500];
};

typedef struct initrd_master_block initrd_master_block_t;

/*******************************************************************************
 * GLOBAL VARIABLES
 ******************************************************************************/

extern initrd_master_block_t _KERNEL_INITRD_MEM_BASE;

extern uint32_t _KERNEL_INITRD_MEM_SIZE;

static initrd_device_t current_dev;

/*******************************************************************************
 * STATIC FUNCTIONS DECLARATION
 ******************************************************************************/

/* None */

/*******************************************************************************
 * FUNCTIONS
 ******************************************************************************/

OS_RETURN_E initrd_init_device(initrd_device_t* device)
{
    initrd_master_block_t* master_block;

    device->start_addr = (uintptr_t)&_KERNEL_INITRD_MEM_BASE;
    master_block = (initrd_master_block_t*)&_KERNEL_INITRD_MEM_BASE;

    KERNEL_DEBUG(INITRD_DEBUG_ENABLED,
                 "[INITRD] Initializing INITRD at 0x%p",
                 device->start_addr);

    if((device->start_addr & (KERNEL_PAGE_SIZE - 1)) != 0)
    {
        KERNEL_ERROR("INIT Ram disk must be aligned on page boudaries\n");
        return OS_ERR_ALIGN;
    }

    KERNEL_DEBUG(INITRD_DEBUG_ENABLED,
                 "[INITRD] Magic is 0x%llx",
                 master_block->magic);
    KERNEL_DEBUG(INITRD_DEBUG_ENABLED,
                 "[INITRD] Size is 0x%X",
                 master_block->size);

    /* The UTK init ram disk starts with a magic block of 512 bytes (based on
     * USTAR block size) with metadata.
     */
    if(master_block->magic != UTK_INITRD_MAGIC)
    {
        KERNEL_ERROR("INIT Ram disk has wrong magic number\n");
        return OS_ERR_WRONG_SIGNATURE;
    }
    if(master_block->size > (uintptr_t)&_KERNEL_INITRD_MEM_SIZE)
    {
        KERNEL_ERROR("Memory space allocated for INIT Ram disk is"
                     " insufficient\n");
        return OS_ERR_UNAUTHORIZED_ACTION;
    }

    device->size = master_block->size;
    device->end_addr = device->start_addr + device->size;
    current_dev.size = device->size;
    current_dev.start_addr = device->start_addr;
    current_dev.end_addr = device->end_addr;

    KERNEL_DEBUG(INITRD_DEBUG_ENABLED,
                 "[INITRD] Initialized INITRD at 0x%p->0x%p, size: 0x%X",
                 device->start_addr,
                 device->end_addr,
                 device->size);

    return OS_NO_ERR;
}

OS_RETURN_E initrd_get_device(initrd_device_t* device)
{
    /* INITRD was not initialized */
    if(current_dev.size == 0)
    {
        return OS_ERR_NOT_INITIALIZED;
    }

    *device = current_dev;

    return OS_NO_ERR;
}

OS_RETURN_E initrd_read_blocks(const vfs_device_t* device,
                               const uint32_t block_id,
	                           void* buffer,
                               const size_t size,
                               const size_t offset)
{
    initrd_device_t* initrd_dev;
    uintptr_t        block_addr;
    uintptr_t        block_end_addr;

    initrd_dev = (initrd_device_t*)device->device_data;

    KERNEL_DEBUG(INITRD_DEBUG_ENABLED,
                 "[INITRD] Reading block 0x%p, size 0x%p, offset: 0x%X",
                 block_id,
                 size,
                 offset);

    if(initrd_dev->start_addr != current_dev.start_addr ||
       initrd_dev->end_addr != current_dev.end_addr ||
       initrd_dev->size != current_dev.size)
    {
        KERNEL_ERROR("Wrong INITRD device\n");
        return OS_ERR_UNAUTHORIZED_ACTION;
    }

    /* We always skip the master block */
    block_addr = current_dev.start_addr +
                 block_id +
                 sizeof(initrd_master_block_t) +
                 offset;
    block_end_addr = block_addr + size;

    /* Check bounds */
    if(block_addr > current_dev.end_addr ||
       block_end_addr > current_dev.end_addr)
    {
        return OS_ERR_OUT_OF_BOUND;
    }

    memcpy(buffer, (void*)block_addr, size);

    return OS_NO_ERR;
}

OS_RETURN_E initrd_write_blocks(const vfs_device_t* device,
                                const uint32_t block_id,
	                            const void* buffer,
                                const size_t size,
                                const size_t offset)
{
    initrd_device_t* initrd_dev;
    uintptr_t        block_addr;
    uintptr_t        block_end_addr;

    initrd_dev = (initrd_device_t*)device->device_data;

    if(initrd_dev->start_addr != current_dev.start_addr ||
       initrd_dev->end_addr != current_dev.end_addr ||
       initrd_dev->size != current_dev.size)
    {
        KERNEL_ERROR("Wrong INITRD device\n");
        return OS_ERR_UNAUTHORIZED_ACTION;
    }

    /* We always skip the master block */
    block_addr = current_dev.start_addr +
                 block_id +
                 sizeof(initrd_master_block_t) +
                 offset;
    block_end_addr = block_addr + size;

    /* Check bounds */
    if(block_addr > current_dev.end_addr ||
       block_end_addr > current_dev.end_addr)
    {
        return OS_ERR_OUT_OF_BOUND;
    }

    memcpy((void*)block_addr, buffer, size);

    return OS_NO_ERR;
}

OS_RETURN_E initrd_flush(const vfs_device_t* device,
                         const uint32_t block_id,
                         const size_t size,
                         const size_t offset)
{
    (void)device;
    (void)block_id;
    (void)size;
    (void)offset;
    /* INIT Ram has no flush method */
    return OS_NO_ERR;
}