#include <test_bank.h>

#if IO_APIC_TEST == 1

#include <panic.h>
#include <interrupts.h>
#include <kernel_output.h>
#include <cpu.h>
#include <io_apic.h>

void io_apic_test(void)
{
    /* TEST MASK <= MAX */
    io_apic_set_irq_mask(0 , 1);
    io_apic_set_irq_mask(0 , 0);

    kernel_printf("[TESTMODE] IO-APIC tests passed\n");

    /* Kill QEMU */
    kill_qemu();
}
#else 
void io_apic_test(void)
{
}
#endif