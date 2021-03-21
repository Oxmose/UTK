#include <test_bank.h>

#if IO_APIC_TEST2 == 1

#include <panic.h>
#include <interrupts.h>
#include <kernel_output.h>
#include <cpu.h>
#include <io_apic.h>

void io_apic_test2(void)
{

    /* TEST MASK > MAX , this should generate a kernel panic */
    io_apic_set_irq_mask(255, 0);


    kernel_printf("[TESTMODE] IO-APIC tests error\n");

    /* Kill QEMU */
    kill_qemu();
}
#else 
void io_apic_test2(void)
{
}
#endif