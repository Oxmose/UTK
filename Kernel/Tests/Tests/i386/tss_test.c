
#include <lib/stdio.h>
#include <cpu.h>

#include <Tests/test_bank.h>

#if TSS_TEST  == 1
void tss_test(void)
{
    printf("[TESTMODE] TSS correctly set\n");

    /* Kill QEMU */
    cpu_outw(0x2000, 0x604);    
    while(1)
    {
        __asm__ ("hlt");
    }
}
#else 
void tss_test(void)
{
}
#endif