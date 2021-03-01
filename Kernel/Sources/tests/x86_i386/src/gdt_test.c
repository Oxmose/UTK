#include <test_bank.h>

#if GDT_TEST  == 1
#include <kernel_output.h>

void gdt_test(void)
{
    kernel_printf("[TESTMODE] GDT correctly set\n");

    kill_qemu();
}
#else 
void gdt_test(void)
{
}
#endif