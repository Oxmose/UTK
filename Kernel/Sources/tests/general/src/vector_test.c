#include <test_bank.h>

#if VECTOR_TEST == 1
#include <kernel_output.h>
#include <stdlib.h>
#include <vector.h>
#include <kheap.h>

void vector_test(void)
{
    size_t i;
    uint32_t data;
    uint32_t data2;
    vector_t* vector;
    vector_t* vector_cpy;
    OS_RETURN_E err;

    vector = vector_create(VECTOR_ALLOCATOR(kmalloc, kfree), (void*)0, 0, &err);
    if(vector == NULL || err != OS_NO_ERR)
    {
        kernel_error("Error. Create %d\n", err);
        kill_qemu();
    }

    kernel_printf("[TESTMODE] Size: %d, Capacity: %d\n", vector->size, vector->capacity);
    kernel_printf("[TESTMODE] ==== Push\n");
    for(i = 0; i < 20; ++i)
    {
        err = vector_push(vector, (void*)i);
        if(err != OS_NO_ERR)
        {
            kernel_error("Error. Push %d\n", err);
            kill_qemu();
        }
    }
    for(i = 0; i < vector->size; ++i)
    {
        vector_get(vector, i, (void**)&data);
        if(err != OS_NO_ERR)
        {
            kernel_error("Error. Get %d\n", err);
            kill_qemu();
        }
        kernel_printf("[TESTMODE] %d | %d | %d\n", data, vector->array[i], i);
    }
    kernel_printf("[TESTMODE] Size: %d, Capacity: %d\n", vector->size, vector->capacity);

    kernel_printf("[TESTMODE] ==== Insert\n");
    for(i = 0; i < 30; i += 2)
    {
        err = vector_insert(vector, (void*)(i + 100), i);
        if(err != OS_NO_ERR)
        {
            kernel_error("Error. insert %d\n", err);
            kill_qemu();
        }
    }
    for(i = 0; i < vector->size; ++i)
    {
        vector_get(vector, i, (void**)&data);
        if(err != OS_NO_ERR)
        {
            kernel_error("Error. Get %d\n", err);
            kill_qemu();
        }
        kernel_printf("[TESTMODE] %d | %d | %d\n", data, vector->array[i], i);
    }
    kernel_printf("[TESTMODE] Size: %d, Capacity: %d\n", vector->size, vector->capacity);

    kernel_printf("[TESTMODE] ==== Pop\n");
    for(i = 0; i < 6; i++)
    {
        err = vector_pop(vector, (void**)&data);
        if(err != OS_NO_ERR)
        {
            kernel_error("Error. pop %d\n", err);
            kill_qemu();
        }
        kernel_printf("[TESTMODE] %d | %d\n", data, i);
    }
    for(i = 0; i < vector->size; ++i)
    {
        vector_get(vector, i, (void**)&data);
        if(err != OS_NO_ERR)
        {
            kernel_error("Error. Get %d\n", err);
            kill_qemu();
        }
        kernel_printf("[TESTMODE] %d | %d | %d\n", data, vector->array[i], i);
    }
    kernel_printf("[TESTMODE] Size: %d, Capacity: %d\n", vector->size, vector->capacity);

    kernel_printf("[TESTMODE] ==== Set\n");
    for(i = 0; i < vector->size; i++)
    {
        err = vector_set(vector, i, (void*)i);
        if(err != OS_NO_ERR)
        {
            kernel_error("Error. set %d\n", err);
            kill_qemu();
        }
    }
    for(i = 0; i < vector->size; ++i)
    {
        vector_get(vector, i, (void**)&data);
        if(err != OS_NO_ERR)
        {
            kernel_error("Error. Get %d\n", err);
            kill_qemu();
        }
        kernel_printf("[TESTMODE] %d | %d | %d\n", data, vector->array[i], i);
    }
    kernel_printf("[TESTMODE] Size: %d, Capacity: %d\n", vector->size, vector->capacity);

    kernel_printf("[TESTMODE] ==== Resize\n");

    err = vector_resize(vector, 20);
    if(err != OS_NO_ERR)
    {
        kernel_error("Error. Resize %d\n", err);
        kill_qemu();
    }
    for(i = 0; i < vector->size; ++i)
    {
        vector_get(vector, i, (void**)&data);
        if(err != OS_NO_ERR)
        {
            kernel_error("Error. Get %d\n", err);
            kill_qemu();
        }
        kernel_printf("[TESTMODE] %d | %d | %d\n", data, vector->array[i], i);
    }
    kernel_printf("[TESTMODE] Size: %d, Capacity: %d\n", vector->size, vector->capacity);

    err = vector_resize(vector, 80);
    if(err != OS_NO_ERR)
    {
        kernel_error("Error. Resize %d\n", err);
        kill_qemu();
    }
    for(i = 0; i < vector->size; ++i)
    {
        vector_get(vector, i, (void**)&data);
        if(err != OS_NO_ERR)
        {
            kernel_error("Error. Get %d\n", err);
            kill_qemu();
        }
        kernel_printf("[TESTMODE] %d | %d | %d\n", data, vector->array[i], i);
    }
    kernel_printf("[TESTMODE] Size: %d, Capacity: %d\n", vector->size, vector->capacity);

    kernel_printf("[TESTMODE] ==== Shrink to fit\n");
    err = vector_resize(vector, 20);
    if(err != OS_NO_ERR)
    {
        kernel_error("Error. Resize %d\n", err);
        kill_qemu();
    }
    kernel_printf("[TESTMODE] Size: %d, Capacity: %d\n", vector->size, vector->capacity);
    err = vector_shrink_to_fit(vector);
    if(err != OS_NO_ERR)
    {
        kernel_error("Error. Shrink %d\n", err);
        kill_qemu();
    }
    for(i = 0; i < vector->size; ++i)
    {
        vector_get(vector, i, (void**)&data);
        if(err != OS_NO_ERR)
        {
            kernel_error("Error. Get %d\n", err);
            kill_qemu();
        }
        kernel_printf("[TESTMODE] %d | %d | %d\n", data, vector->array[i], i);
    }
    kernel_printf("[TESTMODE] Size: %d, Capacity: %d\n", vector->size, vector->capacity);

    kernel_printf("[TESTMODE] ==== Copy\n");
    vector_cpy = vector_copy(vector, &err);
    if(vector_cpy == NULL || err != OS_NO_ERR)
    {
        kernel_error("Error. Copy %d\n", err);
        kill_qemu();
    }
    for(i = 0; i < vector->size; ++i)
    {
        vector_get(vector, i, (void**)&data);
        vector_get(vector_cpy, i, (void**)&data2);
        if(err != OS_NO_ERR)
        {
            kernel_error("Error. Get %d\n", err);
            kill_qemu();
        }
        kernel_printf("[TESTMODE] %d | %d | %d\n", data, data2, i);
    }
    kernel_printf("[TESTMODE] Size: %d, Capacity: %d\n", vector->size, vector->capacity);
    kernel_printf("[TESTMODE] Size: %d, Capacity: %d\n", vector_cpy->size, vector_cpy->capacity);

    kernel_printf("[TESTMODE] ==== Clear\n");
    err = vector_clear(vector);
    if(err != OS_NO_ERR)
    {
        kernel_error("Error. Clear %d\n", err);
        kill_qemu();
    }
    for(i = 0; i < vector->size; ++i)
    {
        vector_get(vector, i, (void**)&data);
        if(err != OS_NO_ERR)
        {
            kernel_error("Error. Get %d\n", err);
            kill_qemu();
        }
        kernel_printf("[TESTMODE] %d | %d\n", data, i);
    }
    kernel_printf("[TESTMODE] Size: %d, Capacity: %d\n", vector->size, vector->capacity);

    kernel_printf("[TESTMODE] ==== Destroy\n");
    err = vector_destroy(vector);
    if(err != OS_NO_ERR)
    {
        kernel_error("Error. Destroy %d\n", err);
        kill_qemu();
    }
    for(i = 0; i < vector->size; ++i)
    {
        vector_get(vector, i, (void**)&data);
        if(err != OS_NO_ERR)
        {
            kernel_error("Error. Get %d\n", err);
            kill_qemu();
        }
        kernel_printf("[TESTMODE] %d | %d\n", data, i);
    }
    kernel_printf("[TESTMODE] Size: %d, Capacity: %d | 0x%p\n", vector->size, vector->capacity, vector->array);

    kill_qemu();
}
#else 
void vector_test(void)
{
}
#endif