
#include <test_bank.h>

#if LAPIC_TEST2 == 1

#include <interrupts.h>
#include <kernel_output.h>
#include <cpu.h>
#include <lapic.h>
#include <panic.h>
#include <interrupt_settings.h>

void lapic_test2(void)
{
    /* TEST OEI > MAX shoudl raise a panic */
    lapic_set_int_eoi(MAX_INTERRUPT_LINE + 1);

    kernel_printf("[TESTMODE] Local APIC tests ERROR\n");

    /* Kill QEMU */
    kill_qemu();
}
#else 
void lapic_test2(void)
{
}
#endif