
#include <lib/stdio.h>
#include <cpu.h>

#include <Tests/test_bank.h>

#if GDT_TEST  == 1
void gdt_test(void)
{
    printf("[TESTMODE] GDT correctly set\n");

    /* Kill QEMU */
    cpu_outw(0x2000, 0x604);
    cpu_outw(0x2000, 0xB004);
    while(1)
    {
        __asm__ ("hlt");
    }
}
#else 
void gdt_test(void)
{
}
#endif