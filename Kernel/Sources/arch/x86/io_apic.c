/*******************************************************************************
 * @file io_apic.c
 *
 * @see io_apic.h
 *
 * @author Alexy Torres Aurora Dugo
 *
 * @date 25/12/2017
 *
 * @version 1.0
 *
 * @brief IO-APIC (IO advanced programmable interrupt controler) driver.
 *
 * @details IO-APIC (IO advanced programmable interrupt controler) driver.
 * Allows to remmap the IO-APIC IRQ, set the IRQs mask and manage EoI for the
 * X86 IO-APIC.
 *
 * @warning This driver also use the LAPIC driver to function correctly.
 *
 * @copyright Alexy Torres Aurora Dugo
 ******************************************************************************/

#include <io/mmio.h>              /* Memory mapped IOs */
#include <interrupt_settings.h>   /* Interrupts settings */
#include <lib/stdint.h>           /* Generic int types */
#include <lib/stddef.h>           /* Standard definitions */
#include <io/kernel_output.h>     /* Kernel output methods */
#include <acpi.h>                 /* ACPI driver */
#include <lapic.h>                /* LAPIC driver */
#include <pic.h>                  /* PIC driver */
#include <memory/paging.h>        /* Memory management */
#include <memory/memalloc.h>      /* Page allocation */
#include <sync/critical.h>        /* Critical sections */

/* UTK configuration file */
#include <config.h>

/* Tests header file */
#if TEST_MODE_ENABLED
#include <Tests/test_bank.h>
#endif

/* Header file */
#include <io_apic.h>

/*******************************************************************************
 * GLOBAL VARIABLES
 ******************************************************************************/

/** @brief Stores the IO APIC state. */
static uint8_t enabled = 0;

/** @brief The IO APIC base address */
static const uint8_t* io_apic_base_addr;

/** @brief IO-APIC IRQ redirection count. */
static uint32_t max_redirect_count;

/** @brief IO_PIC driver instance. */
interrupt_driver_t io_apic_driver = {
    .driver_set_irq_mask     = io_apic_set_irq_mask,
    .driver_set_irq_eoi      = io_apic_set_irq_eoi,
    .driver_handle_spurious  = io_apic_handle_spurious_irq,
    .driver_get_irq_int_line = io_apic_get_irq_int_line
};

#if MAX_CPU_COUNT > 1
/** @brief Critical section spinlock. */
static spinlock_t lock = SPINLOCK_INIT_VALUE;
#endif

/*******************************************************************************
 * FUNCTIONS
 ******************************************************************************/

/**
 * @brief Writes to the IO APIC controller memory.
 *
 * @details Writes to the IO APIC controller memory.
 *
 * @param[in] reg The register to write.
 * @param[in] val The value to write to the register.
 */
__inline__ static void io_apic_write(const uint32_t reg, const uint32_t val)
{
    mapped_io_write_32((uint32_t*)(io_apic_base_addr + IOREGSEL), reg);
    mapped_io_write_32((uint32_t*)(io_apic_base_addr + IOWIN), val);
}

/**
 * @brief Reads into the IO APIC controller memory.
 *
 * @details Reads into the IO APIC controller memory.
 *
 * @param[in] reg The register to read.
 *
 * @return The value contained in the register.
 */
__inline__ static uint32_t io_apic_read(const uint32_t reg)
{
    mapped_io_write_32((uint32_t*)(io_apic_base_addr + IOREGSEL), reg);
    return mapped_io_read_32((uint32_t*)(io_apic_base_addr + IOWIN));
}

OS_RETURN_E io_apic_init(void)
{
#if IOAPIC_KERNEL_DEBUG == 1
    kernel_serial_debug("IOAPIC initialization\n");
#endif

    uint32_t    i;
    uint32_t    read_count;
    OS_RETURN_E err;

    /* Check IO-APIC support */
    if(acpi_get_io_apic_available() == 0 || acpi_get_lapic_available() == 0)
    {
        return OS_ERR_NOT_SUPPORTED;
    }

    /* Get IO APIC base address */
    io_apic_base_addr = acpi_get_io_apic_address(0);

    /* Map the IO-APIC */
    err = kernel_mmap_hw(io_apic_base_addr, io_apic_base_addr, 0x1000, 0, 0);
    if(err != OS_NO_ERR)
    {
        return err;
    }

#if IOAPIC_KERNEL_DEBUG == 1
    kernel_serial_debug("IOAPIC address mapped to 0x%p\n",
                        io_apic_base_addr);
#endif

    /* Maximum entry count */
    read_count = io_apic_read(IOAPICVER);

    max_redirect_count = ((read_count >> 16) & 0xff) + 1;

    /* Redirect and disable all interrupts */
    for (i = 0; i < max_redirect_count; ++i)
    {
        err = io_apic_set_irq_mask(i, 0);
        if(err != OS_NO_ERR)
        {
            kernel_munmap(io_apic_base_addr, 0x1000);
            return err;
        }
    }

#if TEST_MODE_ENABLED
    io_apic_test();
#endif

    enabled = 1;

    return OS_NO_ERR;
}

OS_RETURN_E io_apic_set_irq_mask(const uint32_t irq_number,
                                 const uint32_t enabled)
{
    uint32_t entry_lo   = 0;
    uint32_t entry_hi   = 0;
    uint32_t actual_irq = 0;
    uint32_t int_state;

    if(irq_number >= max_redirect_count)
    {
        return OS_ERR_NO_SUCH_IRQ_LINE;
    }

    /* Set the interrupt line */
    entry_lo |= irq_number + INT_IOAPIC_IRQ_OFFSET;

    /* Set enable mask */
    entry_lo |= (~enabled & 0x1) << 16;

#if MAX_CPU_COUNT > 1
    ENTER_CRITICAL(int_state, &lock);
#else
    ENTER_CRITICAL(int_state);
#endif

    /* Get the remapped value */
    actual_irq = acpi_get_remmaped_irq(irq_number);

    io_apic_write(IOREDTBL + actual_irq * 2, entry_lo);
    io_apic_write(IOREDTBL + actual_irq * 2 + 1, entry_hi);

#if MAX_CPU_COUNT > 1
    EXIT_CRITICAL(int_state, &lock);
#else
    EXIT_CRITICAL(int_state);
#endif

#if IOAPIC_KERNEL_DEBUG == 1
    kernel_serial_debug("IOAPIC mask IRQ %d (%d): %d\n",
                        irq_number, actual_irq, enabled);
#endif

    return OS_NO_ERR;
}

OS_RETURN_E io_apic_set_irq_eoi(const uint32_t irq_number)
{
    OS_RETURN_E err;

#if IOAPIC_KERNEL_DEBUG == 1
    kernel_serial_debug("IOAPIC set IRQ EOI %d\n",
                        irq_number);
#endif

    err = lapic_set_int_eoi(irq_number);

    return err;
}

INTERRUPT_TYPE_E io_apic_handle_spurious_irq(const uint32_t int_number)
{
    INTERRUPT_TYPE_E int_type;
    int32_t          irq_id;

#if IOAPIC_KERNEL_DEBUG == 1
    kernel_serial_debug("IOAPIC spurious IRQ %d\n",
                        int_number);
#endif

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

    return int_type;
}

int32_t io_apic_get_irq_int_line(const uint32_t irq_number)
{
    if(irq_number > IO_APIC_MAX_IRQ_LINE)
    {
        return -1;
    }

    return irq_number + INT_IOAPIC_IRQ_OFFSET;
}

uint8_t io_apic_capable(void)
{
    /* Check IO-APIC support */
    if(acpi_get_io_apic_available() == 0 || acpi_get_lapic_available() == 0)
    {
        
        return 0;
    }
    return 1;
}