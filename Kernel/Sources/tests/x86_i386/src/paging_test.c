

#include <test_bank.h>

#if PAGING_TEST == 1

#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <kernel_output.h>

volatile int display = 0;

void paging_test(void)
{
   int* wrong_addr = (int*)0x3000;
   *wrong_addr = 0;

    kernel_printf("[TESTMODE] Test passed\n");
    /* Kill QEMU */
    kill_qemu();
}
#else
void paging_test(void)
{
}
#endif