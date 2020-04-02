
#include <lib/stdio.h>
#include <cpu.h>

#include <Tests/test_bank.h>

#if BOOT_TEST  == 1
void boot_test(void)
{
    /* Kill QEMU */
    cpu_outw(0x2000, 0x604);    
    while(1)
    {
        __asm__ ("hlt");
    }
}
#else 
void boot_test(void)
{
}
#endif