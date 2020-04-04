#include <interrupt/interrupts.h>
#include <io/kernel_output.h>
#include <cpu.h>
#include <io_apic.h>
#include <Tests/test_bank.h>
#include <core/panic.h>

#if IO_APIC_TEST == 1
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
    if((err = io_apic_set_irq_mask(IO_APIC_MAX_IRQ_LINE , 1)) != OS_NO_ERR)
    {
        kernel_error("TEST_IOAPIC 1\n");
        kernel_panic(err);
    }

    if((err = io_apic_set_irq_mask(IO_APIC_MAX_IRQ_LINE , 0)) != OS_NO_ERR)
    {
        kernel_error("TEST_IOAPIC 2\n");
        kernel_panic(err);
    }


    kernel_debug("[TESTMODE] IO-APIC tests passed\n");

    /* Kill QEMU */
    cpu_outw(0x2000, 0x604);    
    while(1)
    {
        __asm__ ("hlt");
    }
}
#else 
void io_apic_test(void)
{
}
#endif