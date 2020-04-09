/*******************************************************************************
 * @file pic.c
 *
 * @see pic.h
 *
 * @author Alexy Torres Aurora Dugo
 *
 * @date 17/12/2017
 *
 * @version 1.0
 *
 * @brief PIC (programmable interrupt controler) driver.
 *
 * @details   PIC (programmable interrupt controler) driver. Allows to remmap
 * the PIC IRQ, set the IRQs mask and manage EoI for the X86 PIC.
 *
 * @copyright Alexy Torres Aurora Dugo
 ******************************************************************************/

#include <cpu.h>              /* CPU manmgement */
#include <lib/stdint.h>       /* Generic int types */
#include <lib/stddef.h>       /* Standard definitions */
#include <io/kernel_output.h> /* Kernel output methods */
#include <sync/critical.h>    /* Critical sections */

/* UTK configuration file */
#include <config.h>

/* Tests header file */
#if TEST_MODE_ENABLED
#include <Tests/test_bank.h>
#endif

/* Header file */
#include <pic.h>

/*******************************************************************************
 * GLOBAL VARIABLES
 ******************************************************************************/

/** @brief PIC driver instance. */
interrupt_driver_t pic_driver = {
    .driver_set_irq_mask     = pic_set_irq_mask,
    .driver_set_irq_eoi      = pic_set_irq_eoi,
    .driver_handle_spurious  = pic_handle_spurious_irq,
    .driver_get_irq_int_line = pic_get_irq_int_line
};

#if MAX_CPU_COUNT > 1
/** @brief Critical section spinlock. */
static spinlock_t lock = SPINLOCK_INIT_VALUE;
#endif

/*******************************************************************************
 * FUNCTIONS
 ******************************************************************************/

OS_RETURN_E pic_init(void)
{
#if PIC_KERNEL_DEBUG == 1
    kernel_serial_debug("PIC Initialization start\n");
#endif

    /* Initialize the master, remap IRQs */
    cpu_outb(PIC_ICW1_ICW4 | PIC_ICW1_INIT, PIC_MASTER_COMM_PORT);
    cpu_outb(PIC0_BASE_INTERRUPT_LINE, PIC_MASTER_DATA_PORT);
    cpu_outb(0x4,  PIC_MASTER_DATA_PORT);
    cpu_outb(PIC_ICW4_8086,  PIC_MASTER_DATA_PORT);

    /* Initialize the slave, remap IRQs */
    cpu_outb(PIC_ICW1_ICW4 | PIC_ICW1_INIT, PIC_SLAVE_COMM_PORT);
    cpu_outb(PIC1_BASE_INTERRUPT_LINE, PIC_SLAVE_DATA_PORT);
    cpu_outb(0x2,  PIC_SLAVE_DATA_PORT);
    cpu_outb(PIC_ICW4_8086,  PIC_SLAVE_DATA_PORT);

    /* Set EOI for both PICs. */
    cpu_outb(PIC_EOI, PIC_MASTER_COMM_PORT);
    cpu_outb(PIC_EOI, PIC_SLAVE_COMM_PORT);

    /* Disable all IRQs */
    cpu_outb(0xFF, PIC_MASTER_DATA_PORT);
    cpu_outb(0xFF, PIC_SLAVE_DATA_PORT);

#if PIC_KERNEL_DEBUG == 1
    kernel_serial_debug("PIC Initialization end\n");
#endif

#if TEST_MODE_ENABLED
    pic_test();
#endif

    return OS_NO_ERR;
}

OS_RETURN_E pic_set_irq_mask(const uint32_t irq_number, const uint32_t enabled)
{
    uint8_t  init_mask;
    uint32_t int_state;

#if PIC_KERNEL_DEBUG == 1
    kernel_serial_debug("PIC IRQ mask setting start\n");
#endif

    if(irq_number > PIC_MAX_IRQ_LINE)
    {
        return OS_ERR_NO_SUCH_IRQ_LINE;
    }

#if MAX_CPU_COUNT > 1
    ENTER_CRITICAL(int_state, &lock);
#else
    ENTER_CRITICAL(int_state);
#endif


    /* Manage master PIC */
    if(irq_number < 8)
    {
        /* Retrieve initial mask */
        init_mask = cpu_inb(PIC_MASTER_DATA_PORT);

        /* Set new mask value */
        if(!enabled)
        {
            init_mask |= 1 << irq_number;
        }
        else
        {
            init_mask &= ~(1 << irq_number);
        }

        /* Set new mask */
        cpu_outb(init_mask, PIC_MASTER_DATA_PORT);
    }

    /* Manage slave PIC. WARNING, cascading will be enabled */
    if(irq_number > 7)
    {
        /* Set new IRQ number */
        uint32_t cascading_number = irq_number - 8;

        /* Retrieve initial mask */
        init_mask = cpu_inb(PIC_MASTER_DATA_PORT);

        /* Set new mask value */
        init_mask &= ~(1 << PIC_CASCADING_IRQ);

        /* Set new mask */
        cpu_outb(init_mask, PIC_MASTER_DATA_PORT);

        /* Retrieve initial mask */
        init_mask = cpu_inb(PIC_SLAVE_DATA_PORT);

        /* Set new mask value */
        if(!enabled)
        {
            init_mask |= 1 << cascading_number;
        }
        else
        {
            init_mask &= ~(1 << cascading_number);
        }

        /* Set new mask */
        cpu_outb(init_mask, PIC_SLAVE_DATA_PORT);

        /* If all is masked then disable cascading */
        if(init_mask == 0xFF)
        {
            /* Retrieve initial mask */
            init_mask = cpu_inb(PIC_MASTER_DATA_PORT);

            /* Set new mask value */
            init_mask  |= 1 << PIC_CASCADING_IRQ;

            /* Set new mask */
            cpu_outb(init_mask, PIC_MASTER_DATA_PORT);
        }
    }

#if PIC_KERNEL_DEBUG == 1
    kernel_serial_debug("PIC Mask M: 0x%02x S: 0x%02x\n",
                        cpu_inb(PIC_MASTER_DATA_PORT),
                         cpu_inb(PIC_SLAVE_DATA_PORT));
    kernel_serial_debug("PIC IRQ mask setting end\n");
#endif

#if MAX_CPU_COUNT > 1
    EXIT_CRITICAL(int_state, &lock);
#else
    EXIT_CRITICAL(int_state);
#endif

    return OS_NO_ERR;
}

OS_RETURN_E pic_set_irq_eoi(const uint32_t irq_number)
{
#if PIC_KERNEL_DEBUG == 1
    kernel_serial_debug("PIC IRQ EOI start\n");
#endif

    if(irq_number > PIC_MAX_IRQ_LINE)
    {
        return OS_ERR_NO_SUCH_IRQ_LINE;
    }

    /* End of interrupt signal */
    if(irq_number > 7)
    {
        cpu_outb(PIC_EOI, PIC_SLAVE_COMM_PORT);
    }
    cpu_outb(PIC_EOI, PIC_MASTER_COMM_PORT);

#if PIC_KERNEL_DEBUG == 1
    kernel_serial_debug("PIC IRQ EOI end\n");
#endif

    return OS_NO_ERR;
}

INTERRUPT_TYPE_E pic_handle_spurious_irq(const uint32_t int_number)
{
    uint8_t  isr_val;
    uint32_t irq_number = int_number - INT_PIC_IRQ_OFFSET;

#if PIC_KERNEL_DEBUG == 1
    kernel_serial_debug("PIC Psurious handling %d\n", irq_number);
#endif

    /* Check if regular soft interrupt */
    if(irq_number > PIC_MAX_IRQ_LINE)
    {
        return INTERRUPT_TYPE_REGULAR;
    }

    /* Check the PIC type */
    if(irq_number > 7)
    {
        /* This is not a potential spurious irq */
        if(irq_number != PIC_SPURIOUS_IRQ_SLAVE)
        {
            return INTERRUPT_TYPE_REGULAR;
        }

        /* Read the ISR mask */
        cpu_outb(PIC_READ_ISR, PIC_SLAVE_COMM_PORT);
        isr_val = cpu_inb(PIC_SLAVE_COMM_PORT);
        if((isr_val & PIC_SPURIOUS_IRQ_MASK) != 0)
        {
            return INTERRUPT_TYPE_REGULAR;
        }
        else
        {
            /* Send EOI on master */
            pic_set_irq_eoi(PIC_CASCADING_IRQ);
            return INTERRUPT_TYPE_SPURIOUS;
        }
    }
    else
    {
        /* This is not a potential spurious irq */
        if(irq_number != PIC_SPURIOUS_IRQ_MASTER)
        {
            return INTERRUPT_TYPE_REGULAR;
        }

        /* Read the ISR mask */
        cpu_outb(PIC_READ_ISR, PIC_MASTER_COMM_PORT);
        isr_val = cpu_inb(PIC_MASTER_COMM_PORT);
        if((isr_val & PIC_SPURIOUS_IRQ_MASK) != 0)
        {
            return INTERRUPT_TYPE_REGULAR;
        }
        else
        {
            return INTERRUPT_TYPE_SPURIOUS;
        }
    }
}

OS_RETURN_E pic_disable(void)
{
    uint32_t int_state;

#if MAX_CPU_COUNT > 1
    ENTER_CRITICAL(int_state, &lock);
#else
    ENTER_CRITICAL(int_state);
#endif

    /* Disable all IRQs */
    cpu_outb(0xFF, PIC_MASTER_DATA_PORT);
    cpu_outb(0xFF, PIC_SLAVE_DATA_PORT);

#if PIC_KERNEL_DEBUG == 1
    kernel_serial_debug("PIC disabled\n");
#endif

#if MAX_CPU_COUNT > 1
    EXIT_CRITICAL(int_state, &lock);
#else
    EXIT_CRITICAL(int_state);
#endif

    return OS_NO_ERR;
}

int32_t pic_get_irq_int_line(const uint32_t irq_number)
{
    if(irq_number > PIC_MAX_IRQ_LINE)
    {
        return -1;
    }

    return irq_number + INT_PIC_IRQ_OFFSET;
}