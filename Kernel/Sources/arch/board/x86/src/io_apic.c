/*******************************************************************************
 * @file io_apic.c
 *
 * @see io_apic.h
 *
 * @author Alexy Torres Aurora Dugo
 *
 * @date 08/03/2021
 *
 * @version 1.0
 *
 * @brief IO-APIC (IO advanced programmable interrupt controler) driver.
 *
 * @details IO-APIC (IO advanced programmable interrupt controler) driver.
 * Allows to remap the IO-APIC IRQ, set the IRQs mask and manage EoI for the
 * X86 IO-APIC.
 *
 * @warning This driver also use the LAPIC driver to function correctly.
 *
 * @copyright Alexy Torres Aurora Dugo
 ******************************************************************************/

#include <mmio.h>               /* Memory mapped IOs */
#include <interrupt_settings.h> /* Interrupts settings */
#include <stdint.h>             /* Generic int types */
#include <stddef.h>             /* Standard definitions */
#include <kernel_output.h>      /* Kernel output methods */
#include <acpi.h>               /* ACPI driver */
#include <lapic.h>              /* LAPIC driver */
#include <pic.h>                /* PIC driver */
#include <memmgt.h>             /* Memory management */
#include <critical.h>           /* Critical sections */
#include <kheap.h>              /* Kernel heap */
#include <queue.h>              /* Queue library */
#include <panic.h>              /* Kernel panic */

/* UTK configuration file */
#include <config.h>

/* Tests header file */
#ifdef TEST_MODE_ENABLED
#include <test_bank.h>
#endif

/* Header file */
#include <io_apic.h>

/*******************************************************************************
 * GLOBAL VARIABLES
 ******************************************************************************/

/** @brief Stores the IO APIC state. */
static uint8_t enabled = 0;

/** @brief Stores the IO APICS structures */
static io_apic_data_t*  io_apics;

/** @brief Stores the number of IO APICS */
static uint32_t io_apic_count;

/** @brief IO_PIC driver instance. */
interrupt_driver_t io_apic_driver = {
    .driver_set_irq_mask     = io_apic_set_irq_mask,
    .driver_set_irq_eoi      = io_apic_set_irq_eoi,
    .driver_handle_spurious  = io_apic_handle_spurious_irq,
    .driver_get_irq_int_line = io_apic_get_irq_int_line
};

/*******************************************************************************
 * FUNCTIONS
 ******************************************************************************/

/**
 * @brief Writes to the IO APIC controller memory.
 *
 * @details Writes to the IO APIC controller memory.
 *
 * @param[in] base_addr The IO APIC base address.
 * @param[in] reg The register to write.
 * @param[in] val The value to write to the register.
 */
__inline__ static void io_apic_write(const uintptr_t base_addr,
                                     const uint32_t reg, 
                                     const uint32_t val)
{
    mapped_io_write_32((uint32_t*)(base_addr + IOREGSEL), reg);
    mapped_io_write_32((uint32_t*)(base_addr + IOWIN), val);
}

/**
 * @brief Reads into the IO APIC controller memory.
 *
 * @details Reads into the IO APIC controller memory.
 *
 * @param[in] base_addr The IO APIC base address.
 * @param[in] reg The register to read.
 *
 * @return The value contained in the register.
 */
__inline__ static uint32_t io_apic_read(const uintptr_t base_addr,
                                        const uint32_t reg)
{
    mapped_io_write_32((uint32_t*)(base_addr + IOREGSEL), reg);
    return mapped_io_read_32((uint32_t*)(base_addr + IOWIN));
}

OS_RETURN_E io_apic_init(void)
{
    uint32_t            i;
    uint32_t            j;
    uint32_t            read_count;
    const queue_node_t* acpi_io_apic;
    io_apic_t*          cursor_apic;
    OS_RETURN_E         err;

    io_apics = NULL;

    /* Check IO-APIC support */
    io_apic_count = acpi_get_io_apic_count();
    if(io_apic_count == 0 || acpi_get_lapic_count() == 0)
    {
        return OS_ERR_NOT_SUPPORTED;
    }

    /* Initialize all IO-APIC */
    io_apics = kmalloc(sizeof(io_apic_data_t) * io_apic_count);
    if(io_apics == NULL)
    {
        KERNEL_ERROR("Could not allocate memory for IO-APIC\n");
        KERNEL_PANIC(OS_ERR_MALLOC);
    }

    acpi_io_apic = acpi_get_io_apics()->head;
    for(i = 0; i < io_apic_count; ++i)
    {
        cursor_apic = (io_apic_t*)acpi_io_apic->data;
        /* Get IO APIC data*/
        io_apics[i].base_addr = cursor_apic->io_apic_addr;
        io_apics[i].id        = cursor_apic->apic_id;
        io_apics[i].gsib      = cursor_apic->global_system_interrupt_base;

        /* Map the IO-APIC */
        err = memory_declare_hw(io_apics[i].base_addr, KERNEL_PAGE_SIZE);
        if(err != OS_NO_ERR)
        {
            KERNEL_ERROR("Could not declare IO-APIC region\n");
            return err;
        }
        memory_mmap_direct((void*)io_apics[i].base_addr, 
                            (void*)io_apics[i].base_addr, 
                            0x1000, 
                            0, 
                            0,
                            1);

        KERNEL_DEBUG(IOAPIC_DEBUG_ENABLED, 
                     "[IO-APIC] Address mapped to 0x%p on IO-APIC",
                     io_apics[i].base_addr, io_apics[i].id);

        /* Maximum entry count */
        read_count = io_apic_read(io_apics[i].base_addr, IOAPICVER);
        io_apics[i].max_redirect_count = ((read_count >> 16) & 0xff) + 1;

        /* Redirect and disable all interrupts */
        for (j = 0; j < io_apics[i].max_redirect_count; ++j)
        {
            io_apic_set_irq_mask(j, 0);
        }
        acpi_io_apic = acpi_io_apic->next;
    }

#ifdef TEST_MODE_ENABLED
    io_apic_test();
    io_apic_test2();
#endif

    enabled = 1;

    return OS_NO_ERR;
}

void io_apic_set_irq_mask(const uint32_t irq_number, const uint32_t enabled)
{
    uint32_t  entry_lo;
    uint32_t  entry_hi;
    uint32_t  actual_irq;
    uint32_t  int_state;
    uint32_t  i;
    uintptr_t base_addr;

    /* Find the IO APIC that manage this IRQ */
    base_addr = 0;
    for(i = 0; i < io_apic_count; ++i)
    {
        if(io_apics[i].gsib <= irq_number && 
           io_apics[i].gsib + io_apics[i].max_redirect_count > irq_number)
        {
            base_addr = io_apics[i].base_addr;
            KERNEL_DEBUG(IOAPIC_DEBUG_ENABLED, 
                 "[IO-APIC] Mask IRQ on IO APIC %d",
                 io_apics[i].id);
        }
    }

    if(base_addr == 0)
    {
        KERNEL_ERROR("Could not find IO APIC IRQ %d", irq_number);
        KERNEL_PANIC(OS_ERR_NO_SUCH_IRQ_LINE);
    }

    /* Set the interrupt line */
    entry_lo = irq_number + INT_IOAPIC_IRQ_OFFSET;
    entry_lo |= (~enabled & 0x1) << 16;
    entry_hi = 0;

    ENTER_CRITICAL(int_state);

    /* Get the remapped value */
    actual_irq = acpi_get_remaped_irq(irq_number);

    io_apic_write(base_addr, IOREDTBL + actual_irq * 2, entry_lo);
    io_apic_write(base_addr, IOREDTBL + actual_irq * 2 + 1, entry_hi);

    EXIT_CRITICAL(int_state);

    KERNEL_DEBUG(IOAPIC_DEBUG_ENABLED, 
                 "[IO-APIC] Mask IRQ %d (%d): %d",
                 irq_number, actual_irq, enabled);
}

void io_apic_set_irq_eoi(const uint32_t irq_number)
{
    KERNEL_DEBUG(IOAPIC_DEBUG_ENABLED, "[IO-APIC] Set IRQ EOI %d", irq_number);

    lapic_set_int_eoi(irq_number);
}

INTERRUPT_TYPE_E io_apic_handle_spurious_irq(const uint32_t int_number)
{
    INTERRUPT_TYPE_E int_type;
    int32_t          irq_id;

    int_type = INTERRUPT_TYPE_REGULAR;

    irq_id = int_number - INT_PIC_IRQ_OFFSET;
    if(irq_id >= 0 && irq_id <= PIC_MAX_IRQ_LINE)
    {
        /* If we received a PIC spurious interrupt. */
        if(int_number >= INT_PIC_IRQ_OFFSET &&
           int_number >= INT_PIC_IRQ_OFFSET + 0x0F)
        {
            lapic_set_int_eoi(int_number);
            int_type = INTERRUPT_TYPE_SPURIOUS;
        }

        /* Check for LAPIC spurious interrupt. */
        if(int_number == LAPIC_SPURIOUS_INT_LINE)
        {
            lapic_set_int_eoi(int_number);
            int_type = INTERRUPT_TYPE_SPURIOUS;
        }
    }

    KERNEL_DEBUG(IOAPIC_DEBUG_ENABLED, 
                 "[IO-APIC] Spurious IRQ ? %d : %d", 
                 int_number, int_type);

    return int_type;
}

int32_t io_apic_get_irq_int_line(const uint32_t irq_number)
{
    uint32_t i;
    for(i = 0; i < io_apic_count; ++i)
    {
        if(io_apics[i].gsib <= irq_number && 
           io_apics[i].gsib + io_apics[i].max_redirect_count > irq_number)
        {
            return irq_number + INT_IOAPIC_IRQ_OFFSET;
        }
    }
    
    return -1;
}

uint8_t io_apic_capable(void)
{
    return (acpi_get_io_apic_count() != 0 && acpi_get_lapic_count() != 0);
}