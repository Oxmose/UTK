#include <test_bank.h>

#if PANIC_TEST  == 1
#include <stdio.h>
#include <panic.h>

void panic_test(void)
{
    kernel_panic(666);

    kill_qemu();
}
#else 
void panic_test(void)
{
}
#endif