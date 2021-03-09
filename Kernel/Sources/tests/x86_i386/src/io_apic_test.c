#include <test_bank.h>

#if IO_APIC_TEST == 1

#include <panic.h>
#include <interrupts.h>
#include <kernel_output.h>
#include <cpu.h>
#include <io_apic.h>

void io_apic_test(void)
{
    OS_RETURN_E err;

    /* TEST MASK > MAX */
    if((err = io_apic_set_irq_mask(255, 0)) !=
       OS_ERR_NO_SUCH_IRQ_LINE)
    {
        kernel_error("TEST_IOAPIC 0\n");
        kernel_panic(err);
    }

    /* TEST MASK <= MAX */
    if((err = io_apic_set_irq_mask(0 , 1)) != OS_NO_ERR)
    {
        kernel_error("TEST_IOAPIC 1\n");
        kernel_panic(err);
    }

    if((err = io_apic_set_irq_mask(0 , 0)) != OS_NO_ERR)
    {
        kernel_error("TEST_IOAPIC 2\n");
        kernel_panic(err);
    }

    kernel_printf("[TESTMODE] IO-APIC tests passed\n");

    /* Kill QEMU */
    kill_qemu();
}
#else 
void io_apic_test(void)
{
}
#endif