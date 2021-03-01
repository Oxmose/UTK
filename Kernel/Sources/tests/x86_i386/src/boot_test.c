#include <test_bank.h>

#if BOOT_TEST  == 1

#include <stdio.h>

void boot_test(void)
{
    kill_qemu();
}
#else 
void boot_test(void)
{
}
#endif