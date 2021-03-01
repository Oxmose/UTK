#include <test_bank.h>

#if TSS_TEST  == 1
#include <kernel_output.h>

void tss_test(void)
{
    kernel_printf("[TESTMODE] TSS correctly set\n");

    kill_qemu();
}
#else 
void tss_test(void)
{
}
#endif