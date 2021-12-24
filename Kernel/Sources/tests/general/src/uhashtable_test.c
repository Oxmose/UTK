#include <test_bank.h>

#if UHASHTABLE_TEST == 1
#include <kernel_output.h>
#include <stdlib.h>
#include <uhashtable.h>
#include <kheap.h>

static uint32_t g_seed = 0x21025;

uint32_t random_get(void);

uint32_t random_get(void)
{
    g_seed = (214013*g_seed+2531011);
    return g_seed;
}

void uhashtable_test(void)
{
    size_t       i;
    uhashtable_t* table;
    uint32_t     data;
    OS_RETURN_E  err;

    table = uhashtable_create(UHASHTABLE_ALLOCATOR(kmalloc, kfree), &err);
    if(table == NULL || err != OS_NO_ERR)
    {
        kernel_error("Could not initialize hashtable: %d\n", err);
        kill_qemu();
    }
    kernel_printf("[TESTMODE] Size: %d, Capacity: %d\n", table->size, table->capacity);

    kernel_printf("[TESTMODE] ==== Set\n");
    for(i = 0; i < 26; ++i)
    {
        err = uhashtable_set(table, i, (void*)(i * 10));
        if(err != OS_NO_ERR)
        {
            kernel_error("Could not set hashtable: %d\n", err);
            kill_qemu();
        }
    }
    kernel_printf("[TESTMODE] Size: %d, Capacity: %d\n", table->size, table->capacity);


    kernel_printf("[TESTMODE] ==== Get\n");
    for(i = 0; i < 26; ++i)
    {
        err = uhashtable_get(table, i, (void**)&data);
        kernel_printf("[TESTMODE] Key: %d | Value: %d\n", i, data);
        if(err != OS_NO_ERR)
        {
            kernel_error("Could not set hashtable: %d\n", err);
            kill_qemu();
        }
    }
    kernel_printf("[TESTMODE] Size: %d, Capacity: %d\n", table->size, table->capacity);

    kernel_printf("[TESTMODE] ==== Set\n");
    for(i = 0; i < 26; i += 2)
    {
        err = uhashtable_set(table, i, (void*)(i * 100));
        if(err != OS_NO_ERR)
        {
            kernel_error("Could not set hashtable: %d\n", err);
            kill_qemu();
        }
    }
    for(i = 0; i < 26; i += 2)
    {
        err = uhashtable_set(table, i, (void*)(i * 1000));
        if(err != OS_NO_ERR)
        {
            kernel_error("Could not set hashtable: %d\n", err);
            kill_qemu();
        }
    }
    kernel_printf("[TESTMODE] Size: %d, Capacity: %d\n", table->size, table->capacity);

    kernel_printf("[TESTMODE] ==== Get\n");
    for(i = 0; i < 26; ++i)
    {
        err = uhashtable_get(table, i, (void**)&data);
        kernel_printf("[TESTMODE] Key: %d | Value: %d\n", i, data);
        if(err != OS_NO_ERR)
        {
            kernel_error("Could not set hashtable: %d\n", err);
            kill_qemu();
        }
    }
    kernel_printf("[TESTMODE] Size: %d, Capacity: %d\n", table->size, table->capacity);

    kernel_printf("[TESTMODE] ==== Remove\n");
    for(i = 0; i < 26; ++i)
    {
        if(i % 2 == 0)
        {
            err = uhashtable_remove(table, i, NULL);

            if(err != OS_NO_ERR)
            {
                kernel_error("Could not set hashtable: %d\n", err);
                kill_qemu();
            }
        }
    }
    kernel_printf("[TESTMODE] Size: %d, Capacity: %d\n", table->size, table->capacity);

    kernel_printf("[TESTMODE] ==== Get\n");
    for(i = 0; i < 30; ++i)
    {
        err = uhashtable_get(table, i, (void**)&data);

        if(err != OS_NO_ERR)
        {
            if((err == OS_ERR_NO_SUCH_ID && i % 2 == 0) ||
               (err == OS_ERR_NO_SUCH_ID && i > 25))
            {
                kernel_printf("[TESTMODE] Removed / Not found correctly\n");
            }
            else
            {
                kernel_error("Could not set hashtable: %d\n", err);
                kill_qemu();
            }
        }
        else
        {
            kernel_printf("[TESTMODE] Key: %d | Value: %d\n", i, data);
        }
    }
    kernel_printf("[TESTMODE] Size: %d, Capacity: %d\n", table->size, table->capacity);

    kernel_printf("[TESTMODE] ==== Destroy\n");

    err = uhashtable_destroy(table);
    if(err != OS_NO_ERR)
    {
        kernel_error("Could not destroy hashtable: %d\n", err);
        kill_qemu();
    }
    kernel_printf("[TESTMODE] Size: %d, Capacity: %d\n", table->size, table->capacity);

    kernel_printf("[TESTMODE] ==== Get\n");
    for(i = 0; i < 30; ++i)
    {
        err = uhashtable_get(table, i, (void**)&data);

        if(err != OS_ERR_NULL_POINTER)
        {
            kernel_error("Could not get hashtable: %d\n", err);
            kill_qemu();
        }
    }
    kernel_printf("[TESTMODE] Size: %d, Capacity: %d\n", table->size, table->capacity);


    table = uhashtable_create(UHASHTABLE_ALLOCATOR(kmalloc, kfree), &err);
    if(table == NULL || err != OS_NO_ERR)
    {
        kernel_error("Could not initialize hashtable: %d\n", err);
        kill_qemu();
    }
    kernel_printf("[TESTMODE] Size: %d, Capacity: %d\n", table->size, table->capacity);

    uint32_t* table_data = kmalloc(sizeof(uint32_t) * 200000);
    if(table_data == NULL)
    {
        kernel_error("Could not allocated data set: %d\n", OS_ERR_MALLOC);
        kill_qemu();
    }

    for(i = 0; i < 200000; ++i)
    {
        table_data[i] = random_get();
        err = uhashtable_set(table, i, (void*)table_data[i]);
        if(err != OS_NO_ERR)
        {
            kernel_error("Could not set hashtable: %d\n", err);
            kill_qemu();
        }
    }
    kernel_printf("[TESTMODE] Size: %d, Capacity: %d\n", table->size, table->capacity);
    for(i = 0; i < 200000; ++i)
    {
        err = uhashtable_get(table, i, (void**)&data);

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
    }

    kernel_printf("[TESTMODE] ==== Destroy\n");

    err = uhashtable_destroy(table);
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
void uhashtable_test(void)
{
}
#endif