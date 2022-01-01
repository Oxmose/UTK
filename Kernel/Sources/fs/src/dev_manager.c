/*******************************************************************************
 * @file dev_manager.c
 *
 * @see dev_manager.h
 *
 * @author Alexy Torres Aurora Dugo
 *
 * @date 31/12/2021
 *
 * @version 1.0
 *
 * @brief Kernel's device manager.
 *
 * @details Kernel's device manager. Defines the functions and structures used
 * by the kernel to manage the devices connected to the system.
 *
 * @copyright Alexy Torres Aurora Dugo
 ******************************************************************************/

/*******************************************************************************
 * INCLUDES
 ******************************************************************************/

/* Included headers */
#include <virt_fs.h>       /* Virtual Filesystem Manager */
#include <init_rd.h>       /* INIT RAM Disk API */
#include <vector.h>        /* Vector */
#include <kernel_output.h> /* Kernel output API */
#include <kheap.h>         /* Kernel heap */
#include <panic.h>         /* Kernel panic */

/* Configuration files */
#include <config.h>
#include <test_bank.h>

/* Header file */
#include <dev_manager.h>

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

/**
 * @brief Assert macro used by the device manager to ensure correctness of
 * execution.
 *
 * @details Assert macro used by the device manager to ensure correctness
 * of execution.
 * Due to the critical nature of the device manager, any error generates a
 * kernel panic.
 *
 * @param[in] COND The condition that should be true.
 * @param[in] MSG The message to display in case of kernel panic.
 * @param[in] ERROR The error code to use in case of kernel panic.
 */
#define DEVMGT_ASSERT(COND, MSG, ERROR) {               \
    if((COND) == FALSE)                                 \
    {                                                   \
        PANIC(ERROR, "DEVMGT", MSG, TRUE);              \
    }                                                   \
}

/*******************************************************************************
 * GLOBAL VARIABLES
 ******************************************************************************/

/************************* Imported global variables **************************/
/* None */

/************************* Exported global variables **************************/
/* None */

/************************** Static global variables ***************************/
static vector_t* device_table;

/*******************************************************************************
 * STATIC FUNCTIONS DECLARATIONS
 ******************************************************************************/

/* None */

/*******************************************************************************
 * FUNCTIONS
 ******************************************************************************/

static void init_initrd(void)
{
    OS_RETURN_E     err;
    vfs_device_t*   virt_fs_dev;

    KERNEL_DEBUG(DEVMGT_DEBUG_ENABLED, "DEVMGT", "Initializing INITRD");

    /* Create the device data */
    virt_fs_dev = kmalloc(sizeof(vfs_device_t));
    DEVMGT_ASSERT(virt_fs_dev != NULL, "Could not allocate INITRD resources",
                  OS_ERR_MALLOC);
    virt_fs_dev->device_data = kmalloc(sizeof(initrd_device_t));
    DEVMGT_ASSERT(virt_fs_dev->device_data != NULL,
                  "Could not allocate INITRD resources", OS_ERR_MALLOC);

    err = initrd_init_device((initrd_device_t*)virt_fs_dev->device_data);
    DEVMGT_ASSERT(err == OS_NO_ERR, "Could not init INITRD", err);

    /* Setup the VFS devices */
    virt_fs_dev->block_size   = INITRD_BLOCK_SIZE;
    virt_fs_dev->read_blocks  = initrd_read_blocks;
    virt_fs_dev->write_blocks = initrd_write_blocks;
    virt_fs_dev->flush_blocks = initrd_flush;

    /* Add the device to the table */
    err = vector_push(device_table, virt_fs_dev);
    DEVMGT_ASSERT(err == OS_NO_ERR, "Could add INITRD to device table", err);

    KERNEL_INFO("Initialized INIT RAM Disk device\n");
}

void dev_manager_init(void)
{
    OS_RETURN_E err;

    /* Setup manager's tables */
    device_table = vector_create(VECTOR_ALLOCATOR(kmalloc, kfree), NULL, 0,
                                 &err);
    DEVMGT_ASSERT(err == OS_NO_ERR, "Could not create device table", err);

    /* Detect and setup INITRD */
    init_initrd();
}

/************************************ EOF *************************************/
