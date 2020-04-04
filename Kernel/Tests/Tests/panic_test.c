
#include <lib/stdio.h>
#include <core/panic.h>
#include <cpu.h>
#include <Tests/test_bank.h>

#if PANIC_TEST  == 1
void panic_test(void)
{
    kernel_panic(666);

    /* Kill QEMU */
    cpu_outw(0x2000, 0x604);    
    while(1)
    {
        __asm__ ("hlt");
    }
}
#else 
void panic_test(void)
{
}
#endif