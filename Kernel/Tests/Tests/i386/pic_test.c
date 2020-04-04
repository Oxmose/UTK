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

#include <interrupt/interrupts.h>
#include <io/kernel_output.h>
#include <cpu.h>
#include <pic.h>
#include <Tests/test_bank.h>

#if PIC_TEST == 1
void pic_test(void)
{
    uint8_t  pic0_mask;
    uint8_t  pic1_mask;
    uint8_t  pic0_mask_save;
    uint8_t  pic1_mask_save;
    uint32_t i;
    OS_RETURN_E err;

    /* TEST MASK > MAX */
    if((err = pic_set_irq_mask(PIC_MAX_IRQ_LINE + 1, 0)) != 
       OS_ERR_NO_SUCH_IRQ_LINE)
    {
        kernel_error("[TESTMODE] TEST_PIC 0\n");
    }
    else 
    {
        kernel_success("[TESTMODE] TEST_PIC 0\n");
    }

    /* TEST EOI > MAX */
    if((err = pic_set_irq_eoi(PIC_MAX_IRQ_LINE + 1)) != OS_ERR_NO_SUCH_IRQ_LINE)
    {
        kernel_error("[TESTMODE] TEST_PIC 1\n");
    }
    else 
    {
        kernel_success("[TESTMODE] TEST_PIC 1\n");
    }

    /* Save current PIC mask */
    pic0_mask_save = cpu_inb(PIC_MASTER_DATA_PORT);
    pic1_mask_save = cpu_inb(PIC_SLAVE_DATA_PORT);

    /* TEST MASK SET */
    for(i = 0; i <= PIC_MAX_IRQ_LINE; ++i)
    {
        if((err = pic_set_irq_mask(i, 1)) != OS_NO_ERR)
        {
            kernel_error("[TESTMODE] TEST_PIC 2\n");
        }
        else 
        {
            kernel_success("[TESTMODE] TEST_PIC 2\n");
        }
    }

    pic0_mask = cpu_inb(PIC_MASTER_DATA_PORT);
    pic1_mask = cpu_inb(PIC_SLAVE_DATA_PORT);

    if(pic0_mask != 0 || pic1_mask != 0)
    {
        kernel_error("[TESTMODE] TEST_PIC 3\n");
    }
    else 
    {
        kernel_success("[TESTMODE] TEST_PIC 3\n");
    }

    /* TEST MASK CLEAR */
    for(i = 0; i <= PIC_MAX_IRQ_LINE; ++i)
    {
        if((err = pic_set_irq_mask(i, 0)) != OS_NO_ERR)
        {
            kernel_error("[TESTMODE] TEST_PIC 4\n");
        }
        else 
        {
            kernel_success("[TESTMODE] TEST_PIC 4\n");
        }
    }

    pic0_mask = cpu_inb(PIC_MASTER_DATA_PORT);
    pic1_mask = cpu_inb(PIC_SLAVE_DATA_PORT);

    if(pic0_mask != 0xFF || pic1_mask != 0xFF)
    {
        kernel_error("[TESTMODE] TEST_PIC %d %d 5\n", pic0_mask, pic1_mask);
    }
    else 
    {
        kernel_success("[TESTMODE] TEST_PIC 5\n");
    }

    /* Restore mask */
    cpu_outb(pic0_mask_save, PIC_MASTER_DATA_PORT);
    cpu_outb(pic1_mask_save, PIC_SLAVE_DATA_PORT);

    /* Test spurious detection */
    for(i = INT_PIC_IRQ_OFFSET; i <= PIC_MAX_IRQ_LINE + INT_PIC_IRQ_OFFSET; ++i)
    {
        INTERRUPT_TYPE_E val = pic_handle_spurious_irq(i);
        if(i == PIC_SPURIOUS_IRQ_MASTER + INT_PIC_IRQ_OFFSET || 
           i == INT_PIC_IRQ_OFFSET + PIC_SPURIOUS_IRQ_SLAVE)
        {
            if(val != INTERRUPT_TYPE_SPURIOUS)
            {
                kernel_error("[TESTMODE] TEST_PIC6 (false neg)\n");
            }
        }
        else 
        {
            if(val != INTERRUPT_TYPE_REGULAR)
            {
                kernel_error("[TESTMODE] TEST_PIC6 (false pos)\n");
            }
        }
    }

    kernel_success("[TESTMODE] PIC tests passed\n");

    /* Kill QEMU */
    cpu_outw(0x2000, 0x604);    
    while(1)
    {
        __asm__ ("hlt");
    }
}
#else 
void pic_test(void)
{
}
#endif