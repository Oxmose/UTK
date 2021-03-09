#include <test_bank.h>

#if UART_TEST == 1

#include <uart.h>

void uart_test(void)
{
    uart_put_string("\n[TESTMODE] Serial test passed\n");

    kill_qemu();
}
#else 
void uart_test(void)
{
}
#endif