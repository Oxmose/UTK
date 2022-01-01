#include <test_bank.h>

#if SHASHTABLE_TEST == 1
#include <kernel_output.h>
#include <stdlib.h>
#include <shashtable.h>
#include <kheap.h>
#include <string.h>

static uint32_t g_seed = 0x21025;

uint32_t random_get(void);
void inc_data(char* data);

uint32_t random_get(void)
{
    g_seed = (214013*g_seed+2531011);
    return g_seed;
}

void inc_data(char* data)
{
    size_t i;

    for(i = strlen(data); i > 3; --i)
    {
        if(data[i - 1] == '9')
        {
            data[i - 1] = '0';
        }
        else
        {
            ++data[i - 1];
            break;
        }
    }
    if(i == 3)
    {
        kernel_error("Could not increment data %s\n", data);
        kill_qemu();
    }
}

void shashtable_test(void)
{
    size_t       i;
    shashtable_t* table;
    uint32_t     data;
    OS_RETURN_E  err;
    char* base_data = "TEST000000";

    table = shashtable_create(SHASHTABLE_ALLOCATOR(kmalloc, kfree), &err);
    if(table == NULL || err != OS_NO_ERR)
    {
        kernel_error("Could not initialize hashtable: %d\n", err);
        kill_qemu();
    }
    kernel_printf("[TESTMODE] Size: %d, Capacity: %d\n", table->size, table->capacity);

    kernel_printf("[TESTMODE] ==== Set\n");
    for(i = 0; i < 26; ++i)
    {
        err = shashtable_set(table, base_data, (void*)(i * 10));
        if(err != OS_NO_ERR)
        {
            kernel_error("Could not set hashtable: %d\n", err);
            kill_qemu();
        }
        inc_data(base_data);
    }
    kernel_printf("[TESTMODE] Size: %d, Capacity: %d\n", table->size, table->capacity);

    strncpy(base_data, "TEST000000\0", 11);
    kernel_printf("[TESTMODE] ==== Get\n");
    for(i = 0; i < 26; ++i)
    {
        err = shashtable_get(table, base_data, (void**)&data);
        if(err != OS_NO_ERR)
        {
            kernel_error("Could not get hashtable: %d\n", err);
            kill_qemu();
        }
        kernel_printf("[TESTMODE] Key: %s | Value: %d\n", base_data, data);
        inc_data(base_data);
    }
    kernel_printf("[TESTMODE] Size: %d, Capacity: %d\n", table->size, table->capacity);

    strncpy(base_data, "TEST000000\0", 11);
    kernel_printf("[TESTMODE] ==== Set\n");
    for(i = 0; i < 26; i += 2)
    {
        err = shashtable_set(table, base_data, (void*)(i * 100));
        if(err != OS_NO_ERR)
        {
            kernel_error("Could not set hashtable: %d\n", err);
            kill_qemu();
        }
        inc_data(base_data);
    }
    for(i = 0; i < 26; i += 2)
    {
        err = shashtable_set(table, base_data, (void*)(i * 1000));
        if(err != OS_NO_ERR)
        {
            kernel_error("Could not set hashtable: %d\n", err);
            kill_qemu();
        }
        inc_data(base_data);
    }
    kernel_printf("[TESTMODE] Size: %d, Capacity: %d\n", table->size, table->capacity);

    strncpy(base_data, "TEST000000\0", 11);
    kernel_printf("[TESTMODE] ==== Get\n");
    for(i = 0; i < 26; ++i)
    {
        err = shashtable_get(table, base_data, (void**)&data);
        kernel_printf("[TESTMODE] Key: %s | Value: %d\n", base_data, data);
        if(err != OS_NO_ERR)
        {
            kernel_error("Could not set hashtable: %d\n", err);
            kill_qemu();
        }
        inc_data(base_data);
    }
    kernel_printf("[TESTMODE] Size: %d, Capacity: %d\n", table->size, table->capacity);

    strncpy(base_data, "TEST000000\0", 11);
    kernel_printf("[TESTMODE] ==== Remove\n");
    for(i = 0; i < 26; ++i)
    {
        if(i % 2 == 0)
        {
            err = shashtable_remove(table, base_data, NULL);

            if(err != OS_NO_ERR)
            {
                kernel_error("Could not set hashtable: %d\n", err);
                kill_qemu();
            }
        }
        inc_data(base_data);
    }
    kernel_printf("[TESTMODE] Size: %d, Capacity: %d\n", table->size, table->capacity);

    strncpy(base_data, "TEST000000\0", 11);
    kernel_printf("[TESTMODE] ==== Get\n");
    for(i = 0; i < 30; ++i)
    {
        err = shashtable_get(table, base_data, (void**)&data);

        if(err != OS_NO_ERR)
        {
            if((err == OS_ERR_NO_SUCH_ID && i % 2 == 0) ||
               (err == OS_ERR_NO_SUCH_ID && i > 25))
            {
                kernel_printf("[TESTMODE] Removed / Not found correctly\n");
            }
            else
            {
                kernel_error("Could not get hashtable: %d\n", err);
                kill_qemu();
            }
        }
        else
        {
            kernel_printf("[TESTMODE] Key: %s | Value: %d\n", base_data, data);
        }
        inc_data(base_data);
    }
    kernel_printf("[TESTMODE] Size: %d, Capacity: %d\n", table->size, table->capacity);

    kernel_printf("[TESTMODE] ==== Destroy\n");

    err = shashtable_destroy(table);
    if(err != OS_NO_ERR)
    {
        kernel_error("Could not destroy hashtable: %d\n", err);
        kill_qemu();
    }
    kernel_printf("[TESTMODE] Size: %d, Capacity: %d\n", table->size, table->capacity);

    strncpy(base_data, "TEST000000\0", 11);
    kernel_printf("[TESTMODE] ==== Get\n");
    for(i = 0; i < 30; ++i)
    {
        err = shashtable_get(table, base_data, (void**)&data);

        if(err != OS_ERR_NULL_POINTER)
        {
            kernel_error("Could not get hashtable: %d\n", err);
            kill_qemu();
        }
        inc_data(base_data);
    }
    kernel_printf("[TESTMODE] Size: %d, Capacity: %d\n", table->size, table->capacity);


    table = shashtable_create(SHASHTABLE_ALLOCATOR(kmalloc, kfree), &err);
    if(table == NULL || err != OS_NO_ERR)
    {
        kernel_error("Could not initialize hashtable: %d\n", err);
        kill_qemu();
    }
    kernel_printf("[TESTMODE] Size: %d, Capacity: %d\n", table->size, table->capacity);

    uint32_t* table_data = kmalloc(sizeof(uint32_t) * 20000);
    if(table_data == NULL)
    {
        kernel_error("Could not allocated data set: %d\n", OS_ERR_MALLOC);
        kill_qemu();
    }

    strncpy(base_data, "TEST000000\0", 11);
    for(i = 0; i < 20000; ++i)
    {
        table_data[i] = random_get();
        err = shashtable_set(table, base_data, (void*)table_data[i]);
        if(err != OS_NO_ERR)
        {
            kernel_error("Could not set hashtable: %d\n", err);
            kill_qemu();
        }
        inc_data(base_data);
    }
    kernel_printf("[TESTMODE] Size: %d, Capacity: %d\n", table->size, table->capacity);
    strncpy(base_data, "TEST000000\0", 11);
    for(i = 0; i < 20000; ++i)
    {
        err = shashtable_get(table, base_data, (void**)&data);

        if(err != OS_NO_ERR)
        {
            kernel_error("Could not get hashtable: %d\n", err);
            kill_qemu();
        }
        if(table_data[i] != data)
        {
            kernel_error("Wrong value detected: %d -> %d\n", data, table_data[i]);
            kill_qemu();
        }
        inc_data(base_data);
    }

    kernel_printf("[TESTMODE] ==== Destroy\n");

    err = shashtable_destroy(table);
    if(err != OS_NO_ERR)
    {
        kernel_error("Could not destroy hashtable: %d\n", err);
        kill_qemu();
    }
    kernel_printf("[TESTMODE] Size: %d, Capacity: %d\n", table->size, table->capacity);

    kfree(table_data);

    kernel_printf("[TESTMODE] Passed\n");

    kill_qemu();
}
#else
void shashtable_test(void)
{
}
#endif