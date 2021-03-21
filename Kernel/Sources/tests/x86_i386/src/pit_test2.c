
#include <test_bank.h>


#if PIT_TEST2 == 1
#include <interrupts.h>
#include <kernel_output.h>
#include <cpu.h>
#include <pit.h>
#include <panic.h>
#include <interrupt_settings.h>

void pit_test2(void)
{

    /* Should panic */
    pit_set_frequency(PIT_MIN_FREQ - 1);

    kernel_printf("[TESTMODE] PIT tests ERROR\n");


    /* Kill QEMU */
    kill_qemu();
}
#else 
void pit_test2(void)
{
}
#endif