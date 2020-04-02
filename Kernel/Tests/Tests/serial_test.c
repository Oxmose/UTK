
#include <lib/stdio.h>
#include <cpu.h>

#include <Tests/test_bank.h>

#if SERIAL_TEST  == 1
void serial_test(void)
{
    printf("[TESTMODE] Serial test passed\n");

    /* Kill QEMU */
    cpu_outw(0x2000, 0x604);    
    while(1)
    {
        __asm__ ("hlt");
    }
}
#else 
void serial_test(void)
{
}
#endif