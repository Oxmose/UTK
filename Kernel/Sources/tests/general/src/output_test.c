#include <test_bank.h>

#if OUTPUT_TEST  == 1
#include <kernel_output.h>
#include <stddef.h>

void output_test(void)
{
    uint32_t i = 0;

    kernel_printf("[TESTMODE] This tag should be empty: %d.\n", i++);

    kernel_error("[TESTMODE] This tag should be ERROR: %d.\n", i++);

    kernel_success("[TESTMODE] This tag should be OK: %d.\n", i++);

    kernel_info("[TESTMODE] This tag should be INFO: %d.\n", i++);

    kernel_uart_debug("[TESTMODE] This tag should be DEBUG: %d.\n", i++);
    
    kill_qemu();
}
#else 
void output_test(void)
{
}
#endif