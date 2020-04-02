
#include <io/kernel_output.h>
#include <cpu.h>
#include <memory/kheap.h>

#include <Tests/test_bank.h>

#if KHEAP_TEST  == 1
void kheap_test(void)
{
    uint32_t i;
    void* address[20]   = {NULL};
    void* f_address[20] = {NULL};
    uint32_t sizes[20];

    for(i = 0; i < 20; ++i)
    {
        sizes[i] = sizeof(int32_t) * (i + 1);
        address[i] = kmalloc(sizes[i]);
        f_address[i] = address[i];
        
    }
    for(i = 0; i < 20; ++i)
    {
        if(i == 5 || i == 10)
        {
            kernel_printf("\n");
        }
        kernel_printf("[TESTMODE] Kheap alloc %uB\n", sizes[i]);
    }
    for(i = 0; i < 20; ++i)
    {
        kfree(address[i]);
    }
    for(i = 0; i < 20; ++i)
    {
        sizes[i] = sizeof(int32_t) * (i + 1);
        address[i] = kmalloc(sizes[i]);
        if(f_address[i] != address[i])
        {
            kernel_error("[TESTMODE] Error on address allocation\n");
        }
    }
    
    /* Kill QEMU */
    cpu_outw(0x2000, 0x604);    
    while(1)
    {
        __asm__ ("hlt");
    }
}
#else 
void kheap_test(void)
{
}
#endif