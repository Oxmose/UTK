
#include <lib/stdio.h>
#include <cpu.h>

#include <Tests/test_bank.h>

#if IDT_TEST  == 1
void idt_test(void)
{
    printf("[TESTMODE] IDT correctly set\n");

    /* Kill QEMU */
    cpu_outw(0x2000, 0x604);    
    while(1)
    {
        __asm__ ("hlt");
    }
}
#else 
void idt_test(void)
{
}
#endif