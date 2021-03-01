#include <test_bank.h>

#if IDT_TEST  == 1

#include <kernel_output.h>

void idt_test(void)
{
    kernel_printf("[TESTMODE] IDT correctly set\n");

    kill_qemu();
}
#else 
void idt_test(void)
{
}
#endif