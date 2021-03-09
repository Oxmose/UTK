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
#include <queue.h>         /* Queue library */
#include <kheap.h>         /* Kernel heap */

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

/** @brief Stores the detected CPUs' lapics. */
static queue_t* cpu_lapics;

/* IO APIC */
/** @brief Stores the number of detected IO APIC. */
static uint32_t io_apic_count;
/** @brief Stores the detected IO APICs' information table. */
static queue_t* io_apics;

/* ACPI Tables pointers */
/** @brief Stores the MADT descriptor's address in memory. */
static acpi_madt_t* madt;
/** @brief Stores the DSDT descriptor's address in memory. */
static acpi_dsdt_t* dsdt;

/** @brief Stores the ACPI initialization state. */
static uint8_t acpi_initialized = 0;

/*******************************************************************************
 * FUNCTIONS
 ******************************************************************************/

OS_RETURN_E acpi_map_data(const void* start_addr, size_t size)
{
    uintptr_t   addr_align;
    OS_RETURN_E err;

    /* Align address and size */
    addr_align = (uintptr_t)start_addr & PAGE_ALIGN_MASK;
    size       = size + (uintptr_t)start_addr - addr_align;

    /* Search for mapping for each pages */
    while(size)
    {
        /* Try to map, if already mapped skip */
        err = paging_kmmap_hw((void*)addr_align, (void*)addr_align, 
                             KERNEL_PAGE_SIZE, 1, 0);
        if(err != OS_NO_ERR && err != OS_ERR_MAPPING_ALREADY_EXISTS)
        {
            return err;
        }
        
        /* Update address and size */
        addr_align += KERNEL_PAGE_SIZE;
        if(size >= KERNEL_PAGE_SIZE)
        {
            size -= KERNEL_PAGE_SIZE;
        }
        else 
        {
            size = 0;
        }
    }

    return OS_NO_ERR;
}


/**
 * @brief Parses the APIC entries of the MADT table.
 *
 * @details Parse the APIC entries of the MADT table.The function will parse
 * each entry and detect two of the possible entry kind: the LAPIC entries,
 * which also determine the cpu count and the IO-APIC entries will detect the
 * different available IO-APIC of the system.
 *
 * @param[in] madt_ptr The address of the MADT entry to parse.
 *
 * @return The success state or the error code.
 * - OS_NO_ERR is returned if no error is encountered.
 * - OS_ERR_CHECKSUM_FAILED is returned if the ACPI is corrupted.
 * - OS_ERR_NULL_POINTER is returned if the ACPI contains errored memory
 *   addresses.
 */
static OS_RETURN_E acpi_parse_apic(acpi_madt_t* madt_ptr)
{
    int32_t        sum;
    uint32_t       i;
    uint8_t*       madt_entry;
    uint8_t*       madt_limit;
    apic_header_t* header;
    OS_RETURN_E    err;
    queue_node_t*  new_node;

    if(madt_ptr == NULL)
    {
        return OS_ERR_NULL_POINTER;
    }

    err = acpi_map_data(madt_ptr, sizeof(acpi_madt_t));
    if(err != OS_NO_ERR)
    {
        return err;
    }

    err = OS_NO_ERR;

    KERNEL_DEBUG(ACPI_DEBUG_ENABLED, "[ACPI] Parsing MADT at 0x%p", madt_ptr);

    /* Verify checksum */
    sum = 0;
    for(i = 0; i < madt_ptr->header.length; ++i)
    {
        sum += ((uint8_t*)madt_ptr)[i];
    }

    if((sum & 0xFF) == 0)
    {
        if(*((uint32_t*)madt_ptr->header.signature) == ACPI_APIC_SIG)
        {
            madt_entry = (uint8_t*)(madt_ptr + 1);
            madt_limit = ((uint8_t*)madt_ptr) + madt_ptr->header.length;

            err = acpi_map_data(madt_entry, 
                                (uintptr_t)madt_limit - (uintptr_t)madt_entry);
            if(err == OS_NO_ERR)
            {
                cpu_count = 0;
                io_apic_count = 0;
                while (madt_entry < madt_limit)
                {
                    uint8_t type;

                    /* Get entry header */
                    header = (apic_header_t*)madt_entry;
                    type = header->type;

                    /* Check entry type */
                    if(type == APIC_TYPE_LOCAL_APIC)
                    {
                        KERNEL_DEBUG(ACPI_DEBUG_ENABLED, 
                            "[ACPI] Found LAPIC: CPU #%d | ID #%d | FLAGS %x",
                            ((local_apic_t*)madt_entry)->acpi_cpu_id,
                            ((local_apic_t*)madt_entry)->apic_id,
                            ((local_apic_t*)madt_entry)->flags);

                        if(cpu_count < MAX_CPU_COUNT)
                        {
                            /* Add CPU info to the lapic table */
                            new_node = queue_create_node(
                                madt_entry, 
                                QUEUE_ALLOCATOR(kmalloc, kfree), 
                                &err);
                            if(err != OS_NO_ERR)
                            {
                                KERNEL_ERROR(
                                "Could not allocate node for new lapic\n");
                                continue;
                            }
                            err = queue_push(new_node, cpu_lapics);
                            if(err != OS_NO_ERR)
                            {
                                queue_delete_node(&new_node);
                                KERNEL_ERROR(
                                "Could not enqueue node for new lapic\n");
                                continue;
                            }                            
                            ++cpu_count;
                        }
                        else
                        {
                            KERNEL_INFO(
                                "Exceeded CPU count (%u), ignoring CPU %d\n",
                                MAX_CPU_COUNT,
                                ((local_apic_t*)madt_entry)->acpi_cpu_id);
                        }
                    }
                    else if(type == APIC_TYPE_IO_APIC)
                    {
                        KERNEL_DEBUG(ACPI_DEBUG_ENABLED, 
                            "[ACPI] Found IO-APIC ADDR 0x%p | ID #%d | GSIB %x",
                            ((io_apic_t*)madt_entry)->io_apic_addr,
                            ((io_apic_t*)madt_entry)->apic_id,
                            ((io_apic_t*)madt_entry)->
                            global_system_interrupt_base);

                        if(io_apic_count < MAX_IO_APIC_COUNT)
                        {
                            /* Add IO APIC info to the table */
                            new_node = queue_create_node(
                                madt_entry, 
                                QUEUE_ALLOCATOR(kmalloc, kfree), 
                                &err);
                            if(err != OS_NO_ERR)
                            {
                                KERNEL_ERROR(
                                "Could not allocate node for new IO APIC\n");
                                continue;
                            }
                            err = queue_push(new_node, io_apics);
                            if(err != OS_NO_ERR)
                            {
                                queue_delete_node(&new_node);
                                KERNEL_ERROR(
                                "Could not enqueue node for new IO APIC\n");
                                continue;
                            }    
                            ++io_apic_count;
                        }
                        else
                        {
                            KERNEL_INFO(
                                "Exceeded IO-APIC count, ignoring IO-APIC %d\n",
                                ((io_apic_t*)madt_entry)->apic_id);
                        }
                    }
                    madt_entry += header->length;
                }
               
            }
        }
        else
        {
            KERNEL_ERROR("MADT Signature comparison failed\n");
            err = OS_ERR_CHECKSUM_FAILED;
        }
    }
    else
    {
        KERNEL_ERROR("MADT Checksum failed\n");
        err = OS_ERR_CHECKSUM_FAILED;
    }

    return err;
}

/* Parse the APIC DSDT table.
 * The function will save the DSDT table address in for further use.
 *
 * @param dsdt_ptr The address of the DSDT entry to parse.
 * @returns The function will return an error if the entry cannot be parsed or
 * OS_NO_ERR in case of success.
 */
static OS_RETURN_E acpi_parse_dsdt(acpi_dsdt_t* dsdt_ptr)
{
    int32_t     sum;
    uint32_t    i;
    OS_RETURN_E err;

    if(dsdt_ptr == NULL)
    {
        return OS_ERR_NULL_POINTER;
    }

    err = acpi_map_data(dsdt_ptr, sizeof(acpi_dsdt_t));
    if(err != OS_NO_ERR)
    {
        return err;
    }

    KERNEL_DEBUG(ACPI_DEBUG_ENABLED, "[ACPI] Parsing DSDT at 0x%p", dsdt_ptr);

    err = acpi_map_data(dsdt_ptr, dsdt_ptr->header.length);
    if(err != OS_NO_ERR)
    {
        return err;
    }

    /* Verify checksum */
    sum = 0;

    for(i = 0; i < dsdt_ptr->header.length; ++i)
    {
        sum += ((uint8_t*)dsdt_ptr)[i];
    }
    if((sum & 0xFF) != 0)
    {
        KERNEL_ERROR("DSDT Checksum failed\n");
        return OS_ERR_CHECKSUM_FAILED;
    }

    if(*((uint32_t*)dsdt_ptr->header.signature) != ACPI_DSDT_SIG)
    {
        KERNEL_ERROR("DSDT Signature comparison failed\n");
        return OS_ERR_CHECKSUM_FAILED;
    }

    return OS_NO_ERR;
}

/**
 * @brief Parse the APIC FADT table.
 * 
 * @details Parse the APIC FADT table. The function will save the FADT table 
 * address in for further use. Then the FACS and DSDT addresses are extracted 
 * and both tables are parsed.
 *
 * @param[in] fadt_ptr The address of the FADT entry to parse.
 * 
 * @returns The function will return an error if the entry cannot be parsed or
 * OS_NO_ERR in case of success.
 */
static OS_RETURN_E acpi_parse_fadt(acpi_fadt_t* fadt_ptr)
{
    int32_t     sum;
    uint32_t    i;
    OS_RETURN_E err;

    if(fadt_ptr == NULL)
    {
        return OS_ERR_NULL_POINTER;
    }

    err = acpi_map_data(fadt_ptr, sizeof(acpi_fadt_t));
    if(err != OS_NO_ERR)
    {
        return err;
    }

    err = OS_NO_ERR;

    KERNEL_DEBUG(ACPI_DEBUG_ENABLED,"[ACPI] Parsing FADT at 0x%p", fadt_ptr);

    /* Verify checksum */
    sum = 0;
    for(i = 0; i < fadt_ptr->header.length; ++i)
    {
        sum += ((uint8_t*)fadt_ptr)[i];
    }

    if((sum & 0xFF) == 0)
    {
        if(*((uint32_t*)fadt_ptr->header.signature) == ACPI_FACP_SIG)
        {
            /* Parse DSDT */
            err =  acpi_parse_dsdt((acpi_dsdt_t*)fadt_ptr->dsdt);
            if(err == OS_NO_ERR)
            {
                dsdt = (acpi_dsdt_t*)fadt_ptr->dsdt;
            }
            else
            {
                KERNEL_ERROR("Failed to parse DSDT [%d]\n", err);
            }
        }
        else
        {
            KERNEL_ERROR("FADT Signature comparison failed\n");
            err = OS_ERR_CHECKSUM_FAILED;
        }
    }
    else 
    {
        KERNEL_ERROR("FADT Checksum failed\n");
        err = OS_ERR_CHECKSUM_FAILED;
    }

    return err;
}

/**
 * @brief Parse the APIC SDT table.
 * 
 * @details Parse the APIC SDT table. The function will detect the SDT given as 
 * parameter thanks to the information contained in the header. Then, if the 
 * entry is correctly detected and supported, the parsing function corresponding 
 * will be called.
 *
 * @param[in] header The address of the SDT entry to parse.
 * 
 * @returns The function will return an error if the entry cannot be parsed or
 * OS_NO_ERR in case of success.
 */
static OS_RETURN_E acpi_parse_dt(acpi_header_t* header)
{
    char sig_str[5];

    OS_RETURN_E err = OS_NO_ERR;

    if(header == NULL)
    {
        return OS_ERR_NULL_POINTER;
    }

    err = acpi_map_data(header, sizeof(acpi_header_t));
    if(err != OS_NO_ERR)
    {
        return err;
    }

    KERNEL_DEBUG(ACPI_DEBUG_ENABLED, "[ACPI] Parsing SDT at 0x%p", header);

    memcpy(sig_str, header->signature, 4);
    sig_str[4] = 0;

    KERNEL_DEBUG(ACPI_DEBUG_ENABLED, "[ACPI] Signature: %s", sig_str);

    if(*((uint32_t*)header->signature) == ACPI_FACP_SIG)
    {
        err = acpi_parse_fadt((acpi_fadt_t*)header);
        if(err != OS_NO_ERR)
        {
            KERNEL_ERROR("Failed to parse FADT [%d]\n", err);
        }
    }
    else if(*((uint32_t*)header->signature) == ACPI_APIC_SIG)
    {
        err = acpi_parse_apic((acpi_madt_t*)header);
        if(err == OS_NO_ERR)
        {
            madt = (acpi_madt_t *)header;
        }
        else
        {
            KERNEL_ERROR("Failed to parse MADT [%d]\n", err);
        }
    }
    else 
    {
        KERNEL_DEBUG(ACPI_DEBUG_ENABLED, "[ACPI] Not supported: %s", sig_str);
    }

    return err;
}

/**
 * @brief Parse the APIC RSDT table.
 * 
 * @details Parse the APIC RSDT table. The function will detect the read each 
 * entries of the RSDT and call the corresponding functions to parse the entries 
 * correctly.
 *
 * @param[in] rsdt_ptr The address of the RSDT entry to parse.
 * 
 * @returns The function will return an error if the entry cannot be parsed or
 * OS_NO_ERR in case of success.
 */
static OS_RETURN_E acpi_parse_rsdt(rsdt_descriptor_t* rsdt_ptr)
{
    uint32_t* range_begin;
    uint32_t* range_end;
    int8_t    sum;
    uint8_t   i;

    OS_RETURN_E err;

    if(rsdt_ptr == NULL)
    {
        return OS_ERR_NULL_POINTER;
    }

    err = acpi_map_data(rsdt_ptr, sizeof(rsdt_descriptor_t));
    if(err != OS_NO_ERR)
    {
        return err;
    }

    KERNEL_DEBUG(ACPI_DEBUG_ENABLED, "[ACPI] Parsing RSDT at 0x%p", rsdt_ptr);

    /* Verify checksum */
    sum = 0;
    for(i = 0; i < rsdt_ptr->header.length; ++i)
    {
        sum += ((uint8_t*)rsdt_ptr)[i];
    }

    if((sum & 0xFF) == 0)
    {
        if(*((uint32_t*)rsdt_ptr->header.signature) == ACPI_RSDT_SIG)
        {
            range_begin = (uint32_t*)(&rsdt_ptr->dt_pointers);
            range_end   = (uint32_t*)((uint8_t*)rsdt_ptr + 
                                      rsdt_ptr->header.length);

            err = acpi_map_data(range_begin, 
                                (uintptr_t)range_end - (uintptr_t)range_begin);
            if(err == OS_NO_ERR)
            {
                /* Parse each SDT of the RSDT */
                while(range_begin < range_end)
                {
                    uintptr_t address = *range_begin;
                    err = acpi_parse_dt((acpi_header_t*)address);

                    if(err != OS_NO_ERR)
                    {
                        KERNEL_ERROR("ACPI DT Parse error[%d]\n", err);
                    }
                    ++range_begin;
                }
            }
        }
        else 
        {
            KERNEL_ERROR("RSDT Signature comparison failed\n");
            err = OS_ERR_CHECKSUM_FAILED;
        }
    }
    else
    {
        KERNEL_ERROR("RSDT Checksum failed\n");
        err = OS_ERR_CHECKSUM_FAILED;
    }

    return err;
}

/* Parse the APIC XSDT table.
 * The function will detect the read each entries of the XSDT and call the
 * corresponding functions to parse the entries correctly.
 *
 * @param xsdt_ptr The address of the XSDT entry to parse.
 * @returns The function will return an error if the entry cannot be parsed or
 * OS_NO_ERR in case of success.
 */
static OS_RETURN_E acpi_parse_xsdt(xsdt_descriptor_t* xsdt_ptr)
{
    uint64_t* range_begin;
    uint64_t* range_end;
    int8_t    sum;
    uint8_t   i;

    OS_RETURN_E err;

    if(xsdt_ptr == NULL)
    {
        return OS_ERR_NULL_POINTER;
    }

    err = acpi_map_data(xsdt_ptr, sizeof(xsdt_descriptor_t));
    if(err != OS_NO_ERR)
    {
        return err;
    }

    KERNEL_DEBUG(ACPI_DEBUG_ENABLED, "[ACPI] Parsing XSDT at 0x%p", xsdt_ptr);

    /* Verify checksum */
    sum = 0;
    for(i = 0; i < xsdt_ptr->header.length; ++i)
    {
        sum += ((uint8_t*)xsdt_ptr)[i];
    }

    if((sum & 0xFF) == 0)
    {
        if(*((uint32_t*)xsdt_ptr->header.signature) != ACPI_XSDT_SIG)
        {
            range_begin = (uint64_t*)(&xsdt_ptr->dt_pointers);
            range_end   = (uint64_t*)((uint8_t*)xsdt_ptr + 
                                      xsdt_ptr->header.length);

            err = acpi_map_data(range_begin, 
                                (uintptr_t)range_end - (uintptr_t)range_begin);
            if(err == OS_NO_ERR)
            {
                /* Parse each SDT of the RSDT */
                while(range_begin < range_end)
                {
                    uintptr_t address = *range_begin;
                    err = acpi_parse_dt((acpi_header_t*)address);

                    if(err != OS_NO_ERR)
                    {
                        KERNEL_ERROR("ACPI DT Parse error[%d]\n", err);
                    }
                    ++range_begin;
                }
            }
        }
        else 
        {
            KERNEL_ERROR("XSDT Signature comparison failed\n");
            err = OS_ERR_CHECKSUM_FAILED;
        }
    }
    else
    {
        KERNEL_ERROR("XSDT Checksum failed\n");
        err = OS_ERR_CHECKSUM_FAILED;
    }

    return err;
}

/**
 * @brief Use the APIC RSDP to parse the ACPI infomation.
 * 
 * @details Use the APIC RSDP to parse the ACPI infomation. The function will 
 * detect the RSDT or XSDT pointed and parse them.
 *
 * @param[in] rsdp_desc The RSDP to walk.
 * 
 * @returns The function will return an error if the entry cannot be parsed or
 * OS_NO_ERR in case of success.
 */
static OS_RETURN_E acpi_parse_rsdp(rsdp_descriptor_t* rsdp_desc)
{
    uint8_t              sum;
    uint8_t              i;
    uint64_t             xsdt_addr;
    OS_RETURN_E          err;
    rsdp_descriptor_2_t* extended_rsdp;

    if(rsdp_desc == NULL)
    {
        return OS_ERR_NULL_POINTER;
    }

    err = OS_NO_ERR;

    KERNEL_DEBUG(ACPI_DEBUG_ENABLED, 
                 "[ACPI] Parsing RSDP at 0x%p", 
                 rsdp_desc);

    err = acpi_map_data(rsdp_desc, sizeof(rsdp_descriptor_t));
    if(err != OS_NO_ERR)
    {
        return err;
    }

    /* Verify checksum */
    sum = 0;
    for(i = 0; i < sizeof(rsdp_descriptor_t); ++i)
    {
        sum += ((uint8_t*)rsdp_desc)[i];
    }

    if((sum & 0xFF) == 0)
    {
        KERNEL_DEBUG(ACPI_DEBUG_ENABLED, 
                     "[ACPI] Revision %d detected", 
                     rsdp_desc->revision);

        /* ACPI version check */
        if(rsdp_desc->revision == 0)
        {
            err = acpi_parse_rsdt((rsdt_descriptor_t*)rsdp_desc->rsdt_address);          
        }
        else if(rsdp_desc->revision == 2)
        {
            extended_rsdp = (rsdp_descriptor_2_t*) rsdp_desc;
            sum = 0;

            for(i = 0; i < sizeof(rsdp_descriptor_2_t); ++i)
            {
                sum += ((uint8_t*)extended_rsdp)[i];
            }
            if(sum == 0)
            {
                xsdt_addr = extended_rsdp->xsdt_address;

                if(xsdt_addr)
                {
                    err = acpi_parse_xsdt((xsdt_descriptor_t*)(uintptr_t)
                                           xsdt_addr);
                }
                else
                {
                    err = acpi_parse_rsdt(
                                (rsdt_descriptor_t*)rsdp_desc->rsdt_address);
                }
            }
            else 
            {
                KERNEL_ERROR("Extended RSDP Checksum failed\n");
                err = OS_ERR_CHECKSUM_FAILED;
            }

        }
        else
        {
            KERNEL_ERROR("Unsupported ACPI version %d\n", rsdp_desc->revision);
            err = OS_ERR_ACPI_UNSUPPORTED;
        }
    }
    else
    {
        KERNEL_ERROR("RSDP Checksum failed\n");
        err = OS_ERR_CHECKSUM_FAILED;
    }

    return err;
}

OS_RETURN_E acpi_init(void)
{
    uint8_t*    range_begin;
    uint8_t*    range_end;
    OS_RETURN_E err;
    
    err = OS_NO_ERR;

    /* Init pointers */
    madt = NULL;
    dsdt = NULL;

    cpu_lapics = queue_create_queue(QUEUE_ALLOCATOR(kmalloc, kfree), &err);
    if(err != OS_NO_ERR)
    {
        return err;
    }
    io_apics = queue_create_queue(QUEUE_ALLOCATOR(kmalloc, kfree), &err);
    if(err != OS_NO_ERR)
    {
        return err;
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
            err = acpi_parse_rsdp((rsdp_descriptor_t*)range_begin);
            if(err != OS_NO_ERR)
            {                
                KERNEL_ERROR("Error while parsing RSDP: %d\n", err);
            }
            break;
        }

        range_begin += sizeof(uint64_t);
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

    if(madt == NULL)
    {
        return irq_number;
    }

    base  = (uint8_t*)(madt + 1);
    limit = ((uint8_t*) madt) + madt->header.length;

    /* Walk the table */
    while(base < limit)
    {
        apic_header_t* header = (apic_header_t*)base;

        /* Check for type */
        if(header->type == APIC_TYPE_INTERRUPT_OVERRIDE)
        {
            int_override = (apic_interrupt_override_t*)base;

            /* Return remaped IRQ number */
            if(int_override->source == irq_number)
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
    io_apic_t*    io_apic;
    queue_node_t* node;

    if(acpi_initialized != 1 || madt == NULL || io_apic_id >= io_apic_count)
    {
        return NULL;
    }

    node = io_apics->head;
    io_apic = NULL;
    while (node)
    {
        io_apic = (io_apic_t*)node->data;
        if(io_apic->apic_id == io_apic_id)
        {
            break;
        }
    }

    return (void*)io_apic;
}

void* acpi_get_lapic_addr(void)
{
    if(acpi_initialized != 1 || madt == NULL)
    {
        return NULL;
    }

    return (void*)madt->local_apic_addr;
}

OS_RETURN_E acpi_check_lapic_id(const uint32_t lapic_id)
{
    local_apic_t* lapic;
    queue_node_t* node;
    OS_RETURN_E   err;

    err = OS_ERR_NO_SUCH_LAPIC_ID;

    if(acpi_initialized != 1)
    {
        return OS_ACPI_NOT_INITIALIZED;
    }

    node = cpu_lapics->head;
    lapic = NULL;
    while (node)
    {
        lapic = (local_apic_t*)node->data;
        if(lapic->apic_id == lapic_id)
        {
            err = OS_NO_ERR;
            break;
        }
    }

    return err;
}

int32_t acpi_get_detected_cpu_count(void)
{
    return cpu_count;
}