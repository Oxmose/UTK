
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
    OS_RETURN_E err;

    if((err = lapic_set_int_eoi(0)) != OS_NO_ERR)
    {
        kernel_error("TEST_LAPIC 0\n");
        KERNEL_PANIC(err);
    }

    /* TEST OEI > MAX */
    if((err = lapic_set_int_eoi(MAX_INTERRUPT_LINE + 1)) !=
       OS_ERR_NO_SUCH_IRQ_LINE)
    {
        kernel_error("TEST_LAPIC 1\n");
        KERNEL_PANIC(err);
    }

    kernel_printf("[TESTMODE] Local APIC tests passed\n");

    /* Kill QEMU */
    kill_qemu();
}
#else 
void lapic_test(void)
{
}
#endif