
#include <lib/stdio.h>
#include <cpu.h>

#include <Tests/test_bank.h>

#if ACPI_TEST  == 1
void acpi_test(void)
{
    printf("[TESTMODE] ACPI test passed\n");

    /* Kill QEMU */
    cpu_outw(0x2000, 0x604);    
    while(1)
    {
        __asm__ ("hlt");
    }
}
#else 
void acpi_test(void)
{
}
#endif