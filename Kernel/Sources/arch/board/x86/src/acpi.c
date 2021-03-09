/*******************************************************************************
 * @file acpi.c
 *
 * @see acpi.h
 *
 * @author Alexy Torres Aurora Dugo
 *
 * @date 22/12/2017
 *
 * @version 1.0
 *
 * @brief Kernel ACPI management.
 *
 * @details Kernel ACPI management, detects and parse the ACPI for the kernel.
 *
 * @copyright Alexy Torres Aurora Dugo
 ******************************************************************************/

#include <stdint.h>        /* Generic int types */
#include <stddef.h>        /* Standard definitions */
#include <string.h>        /* String manipualtion */
#include <kernel_output.h> /* Kernel output methods */
#include <paging.h>        /* Memory management */
#include <arch_paging.h>   /* Memory information */
#include <panic.h>         /* Kernel panic */

/* UTK configuration file */
#include <config.h>

/* Tests header file */
#ifdef TEST_MODE_ENABLED
#include <test_bank.h>
#endif

/* Header file */
#include <acpi.h>

/*******************************************************************************
 * GLOBAL VARIABLES
 ******************************************************************************/

/* CPU informations */
/** @brief Stores the number of detected CPU. */
static uint32_t cpu_count = 0;

/** @brief Stores the detected CPUs' lapic. */
static local_apic_t* cpu_lapic[MAX_CPU_COUNT];

/* IO APIC */
/** @brief Stores the number of detected IO APIC. */
static uint32_t   io_apic_count;
/** @brief Stores the detected IO APICs' information table. */
static io_apic_t* io_apic_tables[MAX_IO_APIC_COUNT];

/* ACPI Tables pointers */
/** @brief Stores the RSDP parsing state. */
static uint8_t              rsdp_parse_success;
/** @brief Stores the RSDP descriptor's address in memory. */
static rsdp_descriptor_2_t* rsdp;
/** @brief Tells if the ACPI has a XSDT structure. */
static uint8_t              has_xsdt;
/** @brief Stores the RSDT parsing state. */
static uint8_t              rsdt_parse_success;
/** @brief Stores the RSDT descriptor's address in memory. */
static rsdt_descriptor_t*   rsdt;
/** @brief Stores the XSDT parsing state. */
static uint8_t              xsdt_parse_success;
/** @brief Stores the XSDT descriptor's address in memory. */
static xsdt_descriptor_t*   xsdt;
/** @brief Stores the FADT parsing state. */
static uint8_t              fadt_parse_success;
/** @brief Stores the FADT descriptor's address in memory. */
static acpi_fadt_t*         fadt;
/** @brief Stores the FACS parsing state. */
static uint8_t              facs_parse_success;
/** @brief Stores the FACS descriptor's address in memory. */
static acpi_facs_t*         facs;
/** @brief Stores the DSDT parsing state. */
static uint8_t              dsdt_parse_success;
/** @brief Stores the DSDT descriptor's address in memory. */
static acpi_dsdt_t*         dsdt;
/** @brief Stores the MADT parsing state. */
static uint8_t              madt_parse_success;
/** @brief Stores the MADT descriptor's address in memory. */
static acpi_madt_t*         madt;

/** @brief Stores the ACPI initialization state. */
static uint8_t  acpi_initialized = 0;

/*******************************************************************************
 * FUNCTIONS
 ******************************************************************************/
OS_RETURN_E acpi_init(void)
{
    uint8_t*    range_begin;
    uint8_t*    range_end;
    uint8_t     i;
    OS_RETURN_E err;
    
    err = OS_NO_ERR;

    /* Init pointers */
    rsdp = NULL;
    rsdt = NULL;
    xsdt = NULL;
    fadt = NULL;
    facs = NULL;
    dsdt = NULL;
    madt = NULL;

    /* Init data */
    has_xsdt           = 0;
    rsdp_parse_success = 0;
    xsdt_parse_success = 0;
    rsdt_parse_success = 0;
    fadt_parse_success = 0;
    facs_parse_success = 0;
    dsdt_parse_success = 0;
    madt_parse_success = 0;

    for(i = 0; i < MAX_CPU_COUNT; ++i)
    {
        cpu_lapic[i] = NULL;
    }

    for(i = 0; i < MAX_IO_APIC_COUNT; ++i)
    {
        io_apic_tables[i] = NULL;
    }

    cpu_count     = 0;
    io_apic_count = 0;

    /* Define ACPI table search address range */
    range_begin = (uint8_t*)0x000E0000;
    range_end   = (uint8_t*)0x000FFFFF;
    
    /* Map the memory */
    err = paging_kmmap_hw((void*)0xE0000, (void*)0xE0000, 0x20000, 1, 0);
    if(err != OS_NO_ERR)
    {
        return err;
    }

    /* Search for ACPI table */
    while (range_begin < range_end)
    {
        uint64_t signature = *(uint64_t*)range_begin;

        /* Checking the RSDP signature */
        if(signature == ACPI_RSDP_SIG)
        {
            KERNEL_DEBUG(ACPI_DEBUG_ENABLED, 
                         "[ACPI] RSDP found at 0x%p", 
                         range_begin);

            /* Parse RSDP */
            //err = acpi_parse_rsdp((rsdp_descriptor_t*)range_begin);
            if(err == OS_NO_ERR)
            {
                rsdp = (rsdp_descriptor_2_t*)range_begin;
                rsdp_parse_success = 1;
                break;
            }
        }

        range_begin += sizeof(uint64_t);
    }

    /* Unmap ACPI search range */
    err = paging_kmunmap((void*)0xE0000, 0x20000);
    if(err != OS_NO_ERR)
    {
        return err;
    }

#ifdef TEST_MODE_ENABLED
    acpi_test();
#endif

    acpi_initialized = 1;
    
    return err;
}

int32_t acpi_get_io_apic_available(void)
{
    if(acpi_initialized != 1)
    {
        return -1;
    }

    return io_apic_count;
}

int32_t acpi_get_lapic_available(void)
{
    if(acpi_initialized != 1)
    {
        return -1;
    }

    return cpu_count;
}

int32_t acpi_get_remmaped_irq(const uint32_t irq_number)
{
    uint8_t* base;
    uint8_t* limit;
    apic_interrupt_override_t* int_override;

    if(acpi_initialized != 1)
    {
        return -1;
    }

    if(madt_parse_success == 0)
    {
        return irq_number;
    }

    base  = (uint8_t*)(madt + 1);
    limit = ((uint8_t*) madt) + madt->header.length;

    /* Walk the table */
    while (base < limit)
    {
        apic_header_t* header = (apic_header_t*)base;

        /* Check for type */
        if (header->type == APIC_TYPE_INTERRUPT_OVERRIDE)
        {
            int_override = (apic_interrupt_override_t*)base;

            /* Return remaped IRQ number */
            if (int_override->source == irq_number)
            {
                KERNEL_DEBUG(ACPI_DEBUG_ENABLED,
                             "[ACPI] Interrupt override found %d -> %d",
                             int_override->source,
                             int_override->interrupt);

                return int_override->interrupt;
            }
        }

        base += header->length;
    }

    return irq_number;
}

const void* acpi_get_io_apic_address(const uint32_t io_apic_id)
{
    if(acpi_initialized != 1 || 
       madt_parse_success == 0 || 
       io_apic_id >= io_apic_count)
    {
        return NULL;
    }

    return (void*)io_apic_tables[io_apic_id]->io_apic_addr;
}

void* acpi_get_lapic_addr(void)
{
    if(acpi_initialized != 1 || madt_parse_success == 0)
    {
        return NULL;
    }

    return (void*)madt->local_apic_addr;
}

OS_RETURN_E acpi_check_lapic_id(const uint32_t lapic_id)
{
    uint32_t i;

    if(acpi_initialized != 1)
    {
        return OS_ACPI_NOT_INITIALIZED;
    }

    for(i = 0; i < cpu_count; ++i)
    {
        if(cpu_lapic[i]->apic_id == lapic_id)
        {
            return OS_NO_ERR;
        }
    }

    return OS_ERR_NO_SUCH_LAPIC_ID;
}

int32_t acpi_get_detected_cpu_count(void)
{
    return cpu_count;
}

const local_apic_t** acpi_get_cpu_lapics(void)
{
    return (const local_apic_t**)cpu_lapic;
}