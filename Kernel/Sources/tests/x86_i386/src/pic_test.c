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

#if PIC_TEST == 1

#include <interrupts.h>
#include <kernel_output.h>
#include <cpu.h>
#include <pic.h>

/** @brief Offset of the first line of an IRQ interrupt from PIC. */
#define INT_PIC_IRQ_OFFSET     0x30
/** @brief Offset of the first line of an IRQ interrupt from IO-APIC. */
#define INT_IOAPIC_IRQ_OFFSET  0x40

/** @brief Master PIC CPU command port. */
#define PIC_MASTER_COMM_PORT 0x20
/** @brief Master PIC CPU data port. */
#define PIC_MASTER_DATA_PORT 0x21
/** @brief Slave PIC CPU command port. */
#define PIC_SLAVE_COMM_PORT  0xa0
/** @brief Slave PIC CPU data port. */
#define PIC_SLAVE_DATA_PORT  0xa1

#define PIC_MAX_IRQ_LINE 15

/** @brief Master PIC spurious IRQ number. */
#define PIC_SPURIOUS_IRQ_MASTER 0x07
/** @brief Slave PIC spurious IRQ number. */
#define PIC_SPURIOUS_IRQ_SLAVE  0x0F

void pic_test(void)
{
    uint8_t  pic0_mask;
    uint8_t  pic1_mask;
    uint8_t  pic0_mask_save;
    uint8_t  pic1_mask_save;
    uint32_t i;

    kernel_success("[TESTMODE] TEST_PIC 0\n");
    kernel_success("[TESTMODE] TEST_PIC 1\n");

    /* Save current PIC mask */
    pic0_mask_save = cpu_inb(PIC_MASTER_DATA_PORT);
    pic1_mask_save = cpu_inb(PIC_SLAVE_DATA_PORT);

    /* TEST MASK SET */
    for(i = 0; i <= PIC_MAX_IRQ_LINE; ++i)
    {
        pic_set_irq_mask(i, 1);
        kernel_success("[TESTMODE] TEST_PIC 2\n");
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
        pic_set_irq_mask(i, 0);
        kernel_success("[TESTMODE] TEST_PIC 4\n");
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
    kill_qemu();
}
#else 
void pic_test(void)
{
}
#endif