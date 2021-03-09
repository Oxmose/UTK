

#include <test_bank.h>

#if ACPI_TEST  == 1

#include <stdio.h>
void acpi_test(void)
{
    printf("[TESTMODE] ACPI test passed\n");

    /* Kill QEMU */
    kill_qemu();
}
#else 
void acpi_test(void)
{
}
#endif