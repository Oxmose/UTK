#include <test_bank.h>

#if USTAR_TEST  == 1
#include <kernel_output.h>
#include <ustar_fs.h>
#include <init_rd.h>
#include <virt_fs.h>
#include <string.h>

/** @brief Kernel init ram disk memory address. */
extern uint8_t _KERNEL_INITRD_MEM_ADDR;
/** @brief Kernel init ram disk memory size. */
extern uint8_t _KERNEL_INITRD_MEM_SIZE;

void ustar_test(void)
{
    initrd_device_t initrd_dev;
    vfs_device_t    vfs_initrd_dev;
    vfs_partition_t vfs_initrd_part;
    vfs_vnode_t     node;
    uint32_t        i;
    size_t          read_size;

    char buff[1025];

    OS_RETURN_E err;

    err = initrd_get_device(&initrd_dev);
    if(err != OS_NO_ERR)
    {
        kernel_error("[TESTMODE] Could not mount get INITRD device %d\n", err);
        kill_qemu();
        return;
    }

    /* Create the INITRD partition and VFS partition */
    vfs_initrd_dev.flush_blocks = initrd_flush;
    vfs_initrd_dev.read_blocks  = initrd_read_blocks;
    vfs_initrd_dev.write_blocks = initrd_write_blocks;
    vfs_initrd_dev.block_size   = 1;
    vfs_initrd_dev.device_data  = (void*)&initrd_dev;

    vfs_initrd_part.device      = &vfs_initrd_dev;
    vfs_initrd_part.first_block = 0;
    vfs_initrd_part.size        = initrd_dev.size;

    kernel_printf("[TESTMODE] USTAR Test begin\n");

    /* Open root */
    err = ustar_mount(&vfs_initrd_part, "/");
    if(err != OS_NO_ERR)
    {
        kernel_error("[TESTMODE] Could not mount USTAR partition %d\n", err);
        kill_qemu();
        return;
    }

    kernel_printf("[TESTMODE] Mounted USTAR partition\n");

    /* Open folder */
    node.partition = &vfs_initrd_part;
    err = ustar_open_file("folder1/", &node);
    if(err != OS_NO_ERR)
    {
        kernel_error("[TESTMODE] Could not open USTAR folder %d\n", err);
        kill_qemu();
        return;
    }

    kernel_printf("[TESTMODE] %s, %s, %s, %d, %d, %d, %s, %s, %d\n", 
                  node.name, node.path, 
                  node.short_name, node.size, 
                  node.type, node.access_rights,
                  node.owner_name, node.group_name,
                  node.creation_datetime);

    /* List folder */
    memset(buff, 0, 512);
    err = ustar_list_directory(&node, buff, 512);
    if(err != OS_NO_ERR)
    {
        kernel_error("[TESTMODE] Could not list USTAR folder %d\n", err);
        kill_qemu();
        return;
    }

    kernel_printf("[TESTMODE] ");
    i = 0;
    while(buff[i])
    {
        if(buff[i] == ';')
        {
            kernel_printf("\n[TESTMODE] ");
        }
        else 
        {
            kernel_printf("%c", buff[i]);
        }
        ++i;
    } 

    kernel_printf("\n");

    /* Close folder */
    err = ustar_close_file(&node);
    if(err != OS_NO_ERR)
    {
        kernel_error("[TESTMODE] Could not close USTAR folder %d\n", err);
        kill_qemu();
        return;
    }


    /* Open file */
    err = ustar_open_file("folder1/newfile3.txt", &node);
    node.cursor = 0;
    if(err != OS_NO_ERR)
    {
        kernel_error("[TESTMODE] Could not open USTAR folder %d\n", err);
        kill_qemu();
        return;
    }

    kernel_printf("[TESTMODE] %s, %s, %s, %d, %d, %d, %s, %s, %d\n", 
                  node.name, node.path, 
                  node.short_name, node.size, 
                  node.type, node.access_rights,
                  node.owner_name, node.group_name,
                  node.creation_datetime);

    kernel_printf("\n[TESTMODE] ");
    do
    {
        memset(buff, 0, 1025);
        err = ustar_read_file(&node, buff, 540, &read_size);
        if(err != OS_NO_ERR)
        {
            kernel_error("[TESTMODE] Could not read USTAR file %d\n", err);
            kill_qemu();
            return;
        }
        node.cursor += read_size;
        for(i = 0; i < read_size; ++i)
        {
            if(buff[i] == '\n')
            {
                kernel_printf("\n[TESTMODE] ");
            }
            else 
            {
                kernel_printf("%c", buff[i]);
            }
        } 
    }while(read_size != 0);

    kernel_printf("\n");

    
    /* TRY read closed stuff etc... */

    kernel_printf("[TESTMODE] USTAR tests passed\n");
    kill_qemu();
}
#else 
void ustar_test(void)
{
}
#endif