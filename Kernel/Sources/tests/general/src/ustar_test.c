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
    size_t          write_size;
    size_t          item_count;

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

    node.partition = &vfs_initrd_part;

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

    /* Open file tests */
    err = ustar_open_file("file1.test", NULL);
    if(err != OS_ERR_NULL_POINTER)
    {
        kernel_error("[TESTMODE] Open file 1 %d\n", err);
        kill_qemu();
        return;
    }
    else 
    {
        kernel_printf("[TESTMODE] Open file 1\n");
    }
    err = ustar_open_file("thishouldbelongerthanthemaxvaluewhichis150charlongsowehavetomakeaverylongstringtoensurethatwereachthislimitwhichisnottoomuchbutnotthatmucheeitherbecausenewfsdonthavethislimit", &node);
    if(err != OS_ERR_FILE_NOT_FOUND)
    {
        kernel_error("[TESTMODE] Open file 2 %d\n", err);
        kill_qemu();
        return;
    }
    else 
    {
        kernel_printf("[TESTMODE] Open file 2\n");
    }
    err = ustar_open_file("notfound.unknown", &node);
    if(err != OS_ERR_FILE_NOT_FOUND)
    {
        kernel_error("[TESTMODE] Open file 3 %d\n", err);
        kill_qemu();
        return;
    }
    else 
    {
        kernel_printf("[TESTMODE] Open file 3\n");
    }
    err = ustar_open_file("fil1.test", &node);
    if(err != OS_NO_ERR)
    {
        kernel_error("[TESTMODE] Open file 4 %d\n", err);
        kill_qemu();
        return;
    }
    else 
    {
        kernel_printf("[TESTMODE] Open file 4\n");
    }

    /* Close files  */
    err = ustar_close_file(NULL);
    if(err != OS_ERR_NULL_POINTER)
    {
        kernel_error("[TESTMODE] Close file 1 %d\n", err);
        kill_qemu();
        return;
    }
    else 
    {
        kernel_printf("[TESTMODE] Close file 1\n");
    }
    err = ustar_close_file(&node);
    if(err != OS_NO_ERR)
    {
        kernel_error("[TESTMODE] Close file 2 %d\n", err);
        kill_qemu();
        return;
    }
    else 
    {
        kernel_printf("[TESTMODE] Close file 2\n");
    }

    /* Open folder */
    err = ustar_open_file("folder1/", &node);
    if(err != OS_NO_ERR)
    {
        kernel_error("[TESTMODE] Could not open USTAR folder %d\n", err);
        kill_qemu();
        return;
    }

    kernel_printf("[TESTMODE] %s, %s, %s, %d, %d, %d, %s, %s\n", 
                  node.name, node.path, 
                  node.short_name, node.size, 
                  node.type, node.access_rights,
                  node.owner_name, node.group_name);
    kernel_printf("[TESTMODE MANUAL] %s, %s, %s, %d, %d, %d, %s, %s, %d\n", 
                  node.name, node.path, 
                  node.short_name, node.size, 
                  node.type, node.access_rights,
                  node.owner_name, node.group_name,
                  node.creation_datetime);

    /* List folder */
    memset(buff, 0, 512);
    err = ustar_list_directory(&node, &item_count, buff, 512);
    if(err != OS_NO_ERR)
    {
        kernel_error("[TESTMODE] Could not list USTAR folder %d\n", err);
        kill_qemu();
        return;
    }

    read_size = 0;
    while(item_count)
    {
        kernel_printf("[TESTMODE] %s\n", buff + read_size);
        read_size += strlen(buff + read_size) + 1;
        --item_count;
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

    /* Remove file */
    err = ustar_remove_file(node.partition, "folder1/smallfile.txt");
    if(err != OS_NO_ERR)
    {
        kernel_error("[TESTMODE] Could not remove USTAR file %d\n", err);
        kill_qemu();
        return;
    }
    err = ustar_remove_file(node.partition, "unknown.test");
    if(err != OS_ERR_FILE_NOT_FOUND)
    {
        kernel_error("[TESTMODE] Could not remove USTAR file %d\n", err);
        kill_qemu();
        return;
    }
    else 
    {
        kernel_printf("[TESTMODE] Remove file 2\n");
    }

    /* Open folder */
    err = ustar_open_file("folder1/", &node);
    if(err != OS_NO_ERR)
    {
        kernel_error("[TESTMODE] Could not open USTAR folder %d\n", err);
        kill_qemu();
        return;
    }

    kernel_printf("[TESTMODE] %s, %s, %s, %d, %d, %d, %s, %s\n", 
                  node.name, node.path, 
                  node.short_name, node.size, 
                  node.type, node.access_rights,
                  node.owner_name, node.group_name);
    kernel_printf("[TESTMODE MANUAL] %s, %s, %s, %d, %d, %d, %s, %s, %d\n", 
                  node.name, node.path, 
                  node.short_name, node.size, 
                  node.type, node.access_rights,
                  node.owner_name, node.group_name,
                  node.creation_datetime);

    /* List folder */
    memset(buff, 0, 512);
    err = ustar_list_directory(&node, &item_count, buff, 512);
    if(err != OS_NO_ERR)
    {
        kernel_error("[TESTMODE] Could not list USTAR folder %d\n", err);
        kill_qemu();
        return;
    }

    read_size = 0;
    while(item_count)
    {
        kernel_printf("[TESTMODE] %s\n", buff + read_size);
        read_size += strlen(buff + read_size) + 1;
        --item_count;
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

    kernel_printf("[TESTMODE] %s, %s, %s, %d, %d, %d, %s, %s\n", 
                  node.name, node.path, 
                  node.short_name, node.size, 
                  node.type, node.access_rights,
                  node.owner_name, node.group_name);
    kernel_printf("[TESTMODE MANUAL] %s, %s, %s, %d, %d, %d, %s, %s, %d\n", 
                  node.name, node.path, 
                  node.short_name, node.size, 
                  node.type, node.access_rights,
                  node.owner_name, node.group_name,
                  node.creation_datetime);


    /* Read file */
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

    kernel_printf("\n====================================");

    /* Write file */
    strncpy(buff, "This should replace the old text\n", 33);
    err = ustar_write_file(&node, buff, 33, &write_size);
    if(write_size != 0 || err != OS_NO_ERR)
    {
        kernel_error("[TESTMODE] Could not write USTAR file %d %d\n", err, write_size);
        kill_qemu();
        return;
    }
    node.cursor = 0;
    err = ustar_write_file(&node, buff, 33, &write_size);
    if(write_size != 33 || err != OS_NO_ERR)
    {
        kernel_error("[TESTMODE] Could not write USTAR file %d %d\n", err, write_size);
        kill_qemu();
        return;
    }
    node.cursor = 33;
    err = ustar_write_file(&node, buff, 33, &write_size);
    if(write_size != 33 || err != OS_NO_ERR)
    {
        kernel_error("[TESTMODE] Could not write USTAR file %d %d\n", err, write_size);
        kill_qemu();
        return;
    }
    node.cursor = 1024;
    err = ustar_write_file(&node, buff, 33, &write_size);
    if(write_size != 33 || err != OS_NO_ERR)
    {
        kernel_error("[TESTMODE] Could not write USTAR file %d %d\n", err, write_size);
        kill_qemu();
        return;
    }
    node.cursor = 0;
    kernel_printf("\n[TESTMODE] ");
    do
    {
        memset(buff, 0, 1025);
        err = ustar_read_file(&node, buff, 1024, &read_size);
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

    err = ustar_close_file(&node);
    if(err != OS_NO_ERR)
    {
        kernel_error("[TESTMODE] Could not close file %d\n", err);
        kill_qemu();
    }


    /* Open folder */
    err = ustar_open_file("folder1/anotherfolder/", &node);
    if(err != OS_NO_ERR)
    {
        kernel_error("[TESTMODE] Could not open USTAR folder %d\n", err);
        kill_qemu();
        return;
    }

    /* List folder */
    memset(buff, 0, 512);
    err = ustar_list_directory(&node, &item_count, buff, 512);
    if(err != OS_NO_ERR)
    {
        kernel_error("[TESTMODE] Could not list USTAR folder %d\n", err);
        kill_qemu();
        return;
    }

    read_size = 0;
    while(item_count)
    {
        kernel_printf("[TESTMODE] %s\n", buff + read_size);
        read_size += strlen(buff + read_size) + 1;
        --item_count;
    } 

    kernel_printf("\n");

    err = ustar_rename_file(node.partition, "folder1/anotherfolder/myfileinfolder.txt", "folder1/anotherfolder/newfilenew.txt");
    if(err != OS_NO_ERR)
    {
        kernel_error("[TESTMODE] Could not rename USTAR file %d\n", err);
        kill_qemu();
        return;
    }

    memset(buff, 0, 512);
    err = ustar_list_directory(&node, &item_count, buff, 512);
    if(err != OS_NO_ERR)
    {
        kernel_error("[TESTMODE] Could not list USTAR folder %d\n", err);
        kill_qemu();
        return;
    }

    read_size = 0;
    while(item_count)
    {
        kernel_printf("[TESTMODE] %s\n", buff + read_size);
        read_size += strlen(buff + read_size) + 1;
        --item_count;
    }  

    err = ustar_close_file(&node);
    if(err != OS_NO_ERR)
    {
        kernel_error("[TESTMODE] Could not close USTAR file %d\n", err);
        kill_qemu();
        return;
    }

    err = ustar_open_file("folder1/", &node);
    if(err != OS_NO_ERR)
    {
        kernel_error("[TESTMODE] Could not open USTAR folder %d\n", err);
        kill_qemu();
        return;
    }

    kernel_printf("[TESTMODE] \n");

    memset(buff, 0, 512);
    err = ustar_list_directory(&node, &item_count, buff, 512);
    if(err != OS_NO_ERR)
    {
        kernel_error("[TESTMODE] Could not list USTAR folder %d\n", err);
        kill_qemu();
        return;
    }

    read_size = 0;
    while(item_count)
    {
        kernel_printf("[TESTMODE] %s\n", buff + read_size);
        read_size += strlen(buff + read_size) + 1;
        --item_count;
    }  

    kernel_printf("\n");

    err = ustar_rename_file(node.partition, "folder1/anotherfolder/", "folder1/mylittlefolder/");
    if(err != OS_NO_ERR)
    {
        kernel_error("[TESTMODE] Could not rename USTAR file %d\n", err);
        kill_qemu();
        return;
    }

    memset(buff, 0, 512);
    err = ustar_list_directory(&node, &item_count, buff, 512);
    if(err != OS_NO_ERR)
    {
        kernel_error("[TESTMODE] Could not list USTAR folder %d\n", err);
        kill_qemu();
        return;
    }

    read_size = 0;
    while(item_count)
    {
        kernel_printf("[TESTMODE] %s\n", buff + read_size);
        read_size += strlen(buff + read_size) + 1;
        --item_count;
    } 

    kernel_printf("\n");

    err = ustar_close_file(&node);
    if(err != OS_NO_ERR)
    {
        kernel_error("[TESTMODE] Could not close USTAR file %d\n", err);
        kill_qemu();
        return;
    }

    /* Open folder */
    err = ustar_open_file("folder1/mylittlefolder/", &node);
    if(err != OS_NO_ERR)
    {
        kernel_error("[TESTMODE] Could not open USTAR folder %d\n", err);
        kill_qemu();
        return;
    }

    /* List folder */
    memset(buff, 0, 512);
    err = ustar_list_directory(&node, &item_count, buff, 512);
    if(err != OS_NO_ERR)
    {
        kernel_error("[TESTMODE] Could not list USTAR folder %d\n", err);
        kill_qemu();
        return;
    }

    read_size = 0;
    while(item_count)
    {
        kernel_printf("[TESTMODE] %s\n", buff + read_size);
        read_size += strlen(buff + read_size) + 1;
        --item_count;
    } 

    kernel_printf("\n");

    err = ustar_close_file(&node);
    if(err != OS_NO_ERR)
    {
        kernel_error("[TESTMODE] Could not close USTAR file %d\n", err);
        kill_qemu();
        return;
    }

    err = ustar_open_file("fil1.test", &node);
    if(err != OS_NO_ERR)
    {
        kernel_error("[TESTMODE] Could not open USTAR file %d\n", err);
        kill_qemu();
        return;
    }
    node.cursor = 0;

    kernel_printf("\n[TESTMODE] ");
    do
    {
        memset(buff, 0, 1025);
        err = ustar_read_file(&node, buff, 1024, &read_size);
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
    kernel_printf("\n[TESTMODE] ");

    err = ustar_close_file(&node);
    if(err != OS_NO_ERR)
    {
        kernel_error("[TESTMODE] Could not close USTAR file %d\n", err);
        kill_qemu();
        return;
    }

    err = ustar_truncate_file(node.partition, "fil1.test", 1024);
    if(err != OS_ERR_UNAUTHORIZED_ACTION)
    {
        kernel_error("[TESTMODE] Could not truncate USTAR file %d\n", err);
    }
    err = ustar_truncate_file(node.partition, "fil1.test", 7);
    if(err != OS_NO_ERR)
    {
        kernel_error("[TESTMODE] Could not truncate USTAR file %d\n", err);
    }

    err = ustar_open_file("fil1.test", &node);
    if(err != OS_NO_ERR)
    {
        kernel_error("[TESTMODE] Could not open USTAR file %d\n", err);
        kill_qemu();
        return;
    }
    node.cursor = 0;

    kernel_printf("\n[TESTMODE] ");
    do
    {
        memset(buff, 0, 1025);
        err = ustar_read_file(&node, buff, 1024, &read_size);
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

    err = ustar_close_file(&node);
    if(err != OS_NO_ERR)
    {
        kernel_error("[TESTMODE] Could not close USTAR file %d\n", err);
        kill_qemu();
        return;
    }

    kernel_printf("\n[TESTMODE] ");

    kernel_printf("[TESTMODE] USTAR tests passed\n");
    kill_qemu();
}
#else 
void ustar_test(void)
{
}
#endif