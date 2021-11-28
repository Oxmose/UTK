/*******************************************************************************
 *
 * File: test_pic.c
 *
 * Author: Alexy Torres Aurora Dugo
 *
 * Date: 09/01/2018
 *
 * Version: 1.0
 *
 * Kernel tests bank: Programmable interrupt controler tests
 ******************************************************************************/

/*
 * !!! THESE TESTS MUST BE DONE BEFORE INITIALIZING ANY INTERRUPT HANDLER
 *     BETWEEN MIN_INTERRUPT_LINE AND MAX_INTERRUPT_LINE !!!
 * !!! THESE TESTS MUST BE DONE AFTER INITIALIZING THE PIC AND BEFORE THE
 *     IOAPIC!!!
 */

#include <test_bank.h>

#if PIC_TEST2 == 1

#include <interrupts.h>
#include <kernel_output.h>
#include <cpu.h>
#include <pic.h>

#define PIC_MAX_IRQ_LINE 15

void pic_test2(void)
{
    /* TEST MASK > MAX should panic */
    pic_set_irq_mask(PIC_MAX_IRQ_LINE + 1, 0);

    kernel_success("[TESTMODE] PIC tests ERROR\n");

    /* Kill QEMU */
    kill_qemu();
}
#else 
void pic_test2(void)
{
}
#endif