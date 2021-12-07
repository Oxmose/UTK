#include <test_bank.h>

#if USER_HEAP_TEST  == 1
#include <kernel_output.h>
#include <stdlib.h>

void user_heap_test(void)
{
    uint32_t i;
    void* address[20]   = {NULL};
    void* f_address[20] = {NULL};
    uint32_t sizes[20];

    for(i = 0; i < 20; ++i)
    {
        sizes[i] = sizeof(int32_t) * (i + 1);
        address[i] = malloc(sizes[i]);
        f_address[i] = address[i];
        
    }
    for(i = 0; i < 20; ++i)
    {
        if(i == 5 || i == 10)
        {
            kernel_printf("\n");
        }
        kernel_printf("[TESTMODE] heap alloc %uB at 0x%p\n", sizes[i], address[i]);
    }
    for(i = 0; i < 20; ++i)
    {
        free(address[i]);
    }
    for(i = 0; i < 20; ++i)
    {
        sizes[i] = sizeof(int32_t) * (i + 1);
        address[i] = malloc(sizes[i]);
        if(f_address[i] != address[i] || address[i] == 0)
        {
            kernel_error("[TESTMODE] Error on address allocation\n");
        }
    }
    
    kill_qemu();
}
#else 
void kheap_test(void)
{
}
#endif