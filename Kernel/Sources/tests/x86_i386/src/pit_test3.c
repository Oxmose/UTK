
#include <test_bank.h>


#if PIT_TEST3 == 1
#include <interrupts.h>
#include <kernel_output.h>
#include <cpu.h>
#include <pit.h>
#include <panic.h>
#include <interrupt_settings.h>

void pit_test3(void)
{

    /* Should panic */
    pit_set_frequency(PIT_MAX_FREQ + 1);

    kernel_printf("[TESTMODE] PIT tests ERROR\n");


    /* Kill QEMU */
    kill_qemu();
}
#else 
void pit_test3(void)
{
}
#endif