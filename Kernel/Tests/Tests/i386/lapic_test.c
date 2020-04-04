#include <interrupt/interrupts.h>
#include <io/kernel_output.h>
#include <cpu.h>
#include <lapic.h>
#include <Tests/test_bank.h>
#include <core/panic.h>
#include <interrupt_settings.h>

#if LAPIC_TEST == 1
void lapic_test(void)
{
    OS_RETURN_E err;

    /* TEST OEI > MAX */
    if((err = lapic_set_int_eoi(MAX_INTERRUPT_LINE + 1)) !=
       OS_ERR_NO_SUCH_IRQ_LINE)
    {
        kernel_error("TEST_LAPIC 0\n");
        kernel_panic(err);
    }

    kernel_debug("[TESTMODE] Local APIC tests passed\n");

    /* Kill QEMU */
    cpu_outw(0x2000, 0x604);    
    while(1)
    {
        __asm__ ("hlt");
    }
}
#else 
void lapic_test(void)
{
}
#endif