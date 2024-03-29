
#include <test_bank.h>

#if LAPIC_TEST == 1

#include <interrupts.h>
#include <kernel_output.h>
#include <cpu.h>
#include <lapic.h>
#include <panic.h>
#include <interrupt_settings.h>

void lapic_test(void)
{
    lapic_set_int_eoi(0);

    kernel_printf("[TESTMODE] Local APIC tests passed\n");

    /* Kill QEMU */
    kill_qemu();
}
#else 
void lapic_test(void)
{
}
#endif