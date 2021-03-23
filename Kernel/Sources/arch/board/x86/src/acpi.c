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
#include <memmgt.h>        /* Memory management */
#include <arch_memmgt.h>   /* Memory information */
#include <panic.h>         /* Kernel panic */
#include <queue.h>         /* Queue library */
#include <kheap.h>         /* Kernel heap */
#include <lapic.h>         /* LAPIC driver */

/* UTK configuration file */
#include <config.h>

/* Tests header file */
#ifdef TEST_MODE_ENABLED
#include <test_bank.h>
#endif

/* Header file */
#include <acpi.h>

/** @brief ACPI mapping tree node. */
struct acpi_page_tree
{
    /** @brief Left node. */
    struct acpi_page_tree* left;
    /** @brief Right node. */
    struct acpi_page_tree* right;

    /** @brief Address storedi n this node. */
    uintptr_t address;
};

/** @brief Short name for struct acpi_page_tree. */
typedef struct acpi_page_tree acpi_page_tree_t;

/*******************************************************************************
 * GLOBAL VARIABLES
 ******************************************************************************/

/* CPU informations */
/** @brief Stores the number of detected CPU. */
static uint32_t cpu_count;

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

/** @brief The ACPI mapping tree. */
static acpi_page_tree_t* acpi_mapping;

/*******************************************************************************
 * FUNCTIONS
 ******************************************************************************/

/**
 * @brief Search a page in the ACPI mapped tree.
 * 
 * @details Search a page in the ACPI mapped tree. The function return 0 if the
 * page was not found, otherwise 1 is returned.
 * 
 * @param[in] addr The address of the mapped page to search.
 * @param[in] node The starting point node to walk the tree.
 * 
 * @return The function return 0 if the page was not found, otherwise 1 is 
 * returned.
 */
static uint8_t walk_acpi_tree(acpi_page_tree_t* node, const uintptr_t addr)
{
    if(node == NULL)
    {
        return 0;
    }
    else if(node->address == addr)
    {
        return 1;
    }
    else if(addr > node->address)
    {
        return walk_acpi_tree(node->right, addr);
    }
    else
    {
        return walk_acpi_tree(node->left, addr);
    }
}

/**
 * @brief Search a page in the ACPI mapped tree.
 * 
 * @details Search a page in the ACPI mapped tree. The function return 0 if the
 * page was not found, otherwise 1 is returned.
 * 
 * @param[in] addr The address of the mapped page to search.
 * 
 * @return The function return 0 if the page was not found, otherwise 1 is 
 * returned.
 */
static uint8_t is_page_mapped(const uintptr_t addr)
{
    return walk_acpi_tree(acpi_mapping, addr);
}

/**
 * @brief Adds a mapped page node to the ACPI page tree.
 * 
 * @details Adds a mapped page node to the ACPI page tree.
 * 
 * @param[in] node The starting point node to walk the tree.
 * @param[in] addr The address of the mapped page to add.
 */
static void add_acpi_tree(acpi_page_tree_t* node, const uintptr_t addr)
{
    if(node == NULL)
    {
        KERNEL_ERROR("Adding to a NULL node\n");
        KERNEL_PANIC(OS_ERR_NULL_POINTER);
    }
    else if(node->address == addr)
    {
        KERNEL_ERROR("Adding an already existing node\n");
        KERNEL_PANIC(OS_ERR_NULL_POINTER);
    }
    else if(addr > node->address)
    {
        if(node->right == NULL)
        {
            node->right = kmalloc(sizeof(acpi_page_tree_t));
            if(node->right == NULL)
            {
                KERNEL_ERROR("Could not allocate ACPI mapping tree\n");
                KERNEL_PANIC(OS_ERR_MALLOC);
            }
            node->right->right = NULL;
            node->right->left  = NULL;
            node->right->address = addr;
            return;
        }
        else
        {
            add_acpi_tree(node->right, addr);
        }
    }
    else
    {
        if(node->left == NULL)
        {
            node->left = kmalloc(sizeof(acpi_page_tree_t));
            if(node->left == NULL)
            {
                KERNEL_ERROR("Could not allocate ACPI mapping tree\n");
                KERNEL_PANIC(OS_ERR_MALLOC);
            }
            node->left->right = NULL;
            node->left->left  = NULL;
            node->left->address = addr;
            return;
        }
        else
        {
            add_acpi_tree(node->left, addr);
        }
    }
}

/**
 * @brief Adds a mapped page to the ACPI page tree.
 * 
 * @details Adds a mapped page to the ACPI page tree.
 * 
 * @param[in] addr The address of the mapped page to add.
 */
static void add_mapped_page(uintptr_t addr)
{
    if(acpi_mapping == NULL)
    {
        acpi_mapping = kmalloc(sizeof(acpi_page_tree_t));
        if(acpi_mapping == NULL)
        {
            KERNEL_ERROR("Could not allocate ACPI mapping tree\n");
            KERNEL_PANIC(OS_ERR_MALLOC);
        }
        acpi_mapping->right = NULL;
        acpi_mapping->left  = NULL;
        acpi_mapping->address = addr;
    }
    else 
    {
        add_acpi_tree(acpi_mapping, addr);
    }
}

/**
 * @brief Map ACPI memory.
 *
 * @details Map ACPI memory.
 *
 * @param[in] start_addr The address of the MADT entry to parse.
 * @param[in] size The size to map.
 *
 */
static void acpi_map_data(const void* start_addr, size_t size)
{
    uintptr_t   addr_align;
    OS_RETURN_E err;

    /* Align address and size */
    addr_align = (uintptr_t)start_addr & PAGE_ALIGN_MASK;
    size       = size + (uintptr_t)start_addr - addr_align;

    KERNEL_DEBUG(ACPI_DEBUG_ENABLED, 
                         "[ACPI] Mapping request: 0x%p (%d)", 
                         addr_align, size);

    /* Search for mapping for each pages */
    while(size)
    {
        /* Try to map, if already mapped skip */
        if(is_page_mapped(addr_align) == 0)
        {
            KERNEL_DEBUG(ACPI_DEBUG_ENABLED, 
                         "[ACPI] Mapping: 0x%p", 
                         addr_align);
            err = memory_declare_hw(addr_align, KERNEL_PAGE_SIZE);
            if(err != OS_NO_ERR)
            {
                KERNEL_ERROR("Could not declare ACPI region\n");
                KERNEL_PANIC(err)
            }
            memory_mmap_direct((void*)addr_align, 
                                (void*)addr_align, 
                                KERNEL_PAGE_SIZE, 
                                1, 
                                0,
                                1);
            
            add_mapped_page(addr_align);
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
 */
static void acpi_parse_apic(acpi_madt_t* madt_ptr)
{
    uint8_t        type;
    int32_t        sum;
    uint32_t       i;
    uintptr_t      madt_entry;
    uintptr_t      madt_limit;
    apic_header_t* header;
    queue_node_t*  new_node;
    OS_RETURN_E    err;

    cpu_count = 0;
    io_apic_count = 0;

    if(madt_ptr == NULL)
    {
        KERNEL_ERROR("Tried to parse a NULL MADT\n");
        KERNEL_PANIC(OS_ERR_NULL_POINTER);
    }

    acpi_map_data(madt_ptr, sizeof(acpi_madt_t));

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
            madt_entry = (uintptr_t)(madt_ptr + 1);
            madt_limit = ((uintptr_t)madt_ptr) + madt_ptr->header.length;

            acpi_map_data((void*)madt_entry, madt_limit - madt_entry);
          
            while (madt_entry < madt_limit)
            {
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
                            (void*)madt_entry, 
                            QUEUE_ALLOCATOR(kmalloc, kfree), 
                            &err);
                        if(err != OS_NO_ERR)
                        {
                            KERNEL_ERROR(
                            "Could not allocate node for new lapic %d\n",
                            err);
                            continue;
                        }
                        err = queue_push(new_node, cpu_lapics);
                        if(err != OS_NO_ERR)
                        {
                            queue_delete_node(&new_node);
                            KERNEL_ERROR(
                            "Could not enqueue node for new lapic %d\n",
                            err);
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
                            (void*)madt_entry, 
                            QUEUE_ALLOCATOR(kmalloc, kfree), 
                            &err);
                        if(err != OS_NO_ERR)
                        {
                            KERNEL_ERROR(
                            "Could not allocate node for new IO APIC %d\n",
                            err);
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
        else
        {
            KERNEL_ERROR("MADT Signature comparison failed\n");
            KERNEL_PANIC(OS_ERR_CHECKSUM_FAILED);
        }
    }
    else
    {
        KERNEL_ERROR("MADT Checksum failed\n");
        KERNEL_PANIC(OS_ERR_CHECKSUM_FAILED);
    }
}

/**
 * @brief Parse the APIC DSDT table.
 * 
 * @details The function will save the DSDT table address in for further use.
 *
 * @param dsdt_ptr[in] The address of the DSDT entry to parse.
 */
static void acpi_parse_dsdt(acpi_dsdt_t* dsdt_ptr)
{
    int32_t  sum;
    uint32_t i;

    if(dsdt_ptr == NULL)
    {
        KERNEL_ERROR("Tried to parse a NULL DSDT\n");
        KERNEL_PANIC(OS_ERR_NULL_POINTER);
    }

    acpi_map_data(dsdt_ptr, sizeof(acpi_dsdt_t));

    KERNEL_DEBUG(ACPI_DEBUG_ENABLED, "[ACPI] Parsing DSDT at 0x%p", dsdt_ptr);

    acpi_map_data(dsdt_ptr, dsdt_ptr->header.length);

    /* Verify checksum */
    sum = 0;

    for(i = 0; i < dsdt_ptr->header.length; ++i)
    {
        sum += ((uint8_t*)dsdt_ptr)[i];
    }
    if((sum & 0xFF) != 0)
    {
        KERNEL_ERROR("DSDT Checksum failed\n");
        KERNEL_PANIC(OS_ERR_CHECKSUM_FAILED);
    }

    if(*((uint32_t*)dsdt_ptr->header.signature) != ACPI_DSDT_SIG)
    {
        KERNEL_ERROR("DSDT Signature comparison failed\n");
        KERNEL_PANIC(OS_ERR_CHECKSUM_FAILED);
    }
}

/**
 * @brief Parse the APIC FADT table.
 * 
 * @details Parse the APIC FADT table. The function will save the FADT table 
 * address in for further use. Then the FACS and DSDT addresses are extracted 
 * and both tables are parsed.
 *
 * @param[in] fadt_ptr The address of the FADT entry to parse.
 */
static void acpi_parse_fadt(acpi_fadt_t* fadt_ptr)
{
    int32_t  sum;
    uint32_t i;

    if(fadt_ptr == NULL)
    {
        KERNEL_ERROR("Tried to parse a NULL FADT\n");
        KERNEL_PANIC(OS_ERR_NULL_POINTER);
    }

    acpi_map_data(fadt_ptr, sizeof(acpi_fadt_t));

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
            acpi_parse_dsdt((acpi_dsdt_t*)fadt_ptr->dsdt);
        }
        else
        {
            KERNEL_ERROR("FADT Signature comparison failed\n");
            KERNEL_PANIC(OS_ERR_CHECKSUM_FAILED);
        }
    }
    else 
    {
        KERNEL_ERROR("FADT Checksum failed\n");
        KERNEL_PANIC(OS_ERR_CHECKSUM_FAILED);
    }
}

/**
 * @brief Parse the APIC SDT table.
 * 
 * @details Parse the APIC SDT table. The function will detect the SDT given as 
 * parameter thanks to the information contained in the header. Then, if the 
 * entry is correctly detected and supported, the parsing function corresponding 
 * will be called.
 *
 * @param[in] header The address of the SDT entry to parse..
 */
static void acpi_parse_dt(acpi_header_t* header)
{
    char sig_str[5];

    if(header == NULL)
    {
        KERNEL_ERROR("Tried to parse a NULL DT\n");
        KERNEL_PANIC(OS_ERR_NULL_POINTER);
    }

    acpi_map_data(header, sizeof(acpi_header_t));

    KERNEL_DEBUG(ACPI_DEBUG_ENABLED, "[ACPI] Parsing SDT at 0x%p", header);

    memcpy(sig_str, header->signature, 4);
    sig_str[4] = 0;

    KERNEL_DEBUG(ACPI_DEBUG_ENABLED, "[ACPI] Signature: %s", sig_str);

    if(*((uint32_t*)header->signature) == ACPI_FACP_SIG)
    {
        acpi_parse_fadt((acpi_fadt_t*)header);
    }
    else if(*((uint32_t*)header->signature) == ACPI_APIC_SIG)
    {
        acpi_parse_apic((acpi_madt_t*)header);
        madt = (acpi_madt_t *)header;
    }
    else 
    {
        KERNEL_DEBUG(ACPI_DEBUG_ENABLED, "[ACPI] Not supported: %s", sig_str);
    }
}

/**
 * @brief Parse the APIC RSDT table.
 * 
 * @details Parse the APIC RSDT table. The function will detect the read each 
 * entries of the RSDT and call the corresponding functions to parse the entries 
 * correctly.
 *
 * @param[in] rsdt_ptr The address of the RSDT entry to parse.
 */
static void acpi_parse_rsdt(rsdt_descriptor_t* rsdt_ptr)
{
    uintptr_t      range_begin;
    uintptr_t      range_end;
    acpi_header_t* address;
    int8_t         sum;
    uint8_t        i;

    if(rsdt_ptr == NULL)
    {
        KERNEL_ERROR("Tried to parse a NULL RSDT\n");
        KERNEL_PANIC(OS_ERR_NULL_POINTER);
    }

    acpi_map_data(rsdt_ptr, sizeof(rsdt_descriptor_t));

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
            range_begin = (uintptr_t)(&rsdt_ptr->dt_pointers);
            range_end   = ((uintptr_t)rsdt_ptr + rsdt_ptr->header.length);

            acpi_map_data((void*)range_begin, range_end - range_begin);

            /* Parse each SDT of the RSDT */
            while(range_begin < range_end)
            {
                address = (acpi_header_t*)(*(uintptr_t*)range_begin);
                acpi_parse_dt(address);
                range_begin += sizeof(uintptr_t);
            }
        }
        else 
        {
            KERNEL_ERROR("RSDT Signature comparison failed\n");
            KERNEL_PANIC(OS_ERR_CHECKSUM_FAILED);
        }
    }
    else
    {
        KERNEL_ERROR("RSDT Checksum failed\n");
        KERNEL_PANIC(OS_ERR_CHECKSUM_FAILED);
    }
}

/**
 * @brief Parse the APIC XSDT table.
 * @details The function will detect the read each entries of the XSDT and call 
 * the corresponding functions to parse the entries correctly.
 *
 * @param xsdt_ptr[in] The address of the XSDT entry to parse.
 */
static void acpi_parse_xsdt(xsdt_descriptor_t* xsdt_ptr)
{
    uintptr_t      range_begin;
    uintptr_t      range_end;
    acpi_header_t* address;
    int8_t         sum;
    uint8_t        i;

    if(xsdt_ptr == NULL)
    {
        KERNEL_ERROR("Tried to parse a NULL XSDT\n");
        KERNEL_PANIC(OS_ERR_NULL_POINTER);
    }

    acpi_map_data(xsdt_ptr, sizeof(xsdt_descriptor_t));

    KERNEL_DEBUG(ACPI_DEBUG_ENABLED, "[ACPI] Parsing XSDT at 0x%p", xsdt_ptr);

    /* Verify checksum */
    sum = 0;
    for(i = 0; i < xsdt_ptr->header.length; ++i)
    {
        sum += ((uint8_t*)xsdt_ptr)[i];
    }

    if((sum & 0xFF) == 0)
    {
        if(*((uint32_t*)xsdt_ptr->header.signature) == ACPI_XSDT_SIG)
        {
            range_begin = (uintptr_t)(&xsdt_ptr->dt_pointers);
            range_end   = ((uintptr_t)xsdt_ptr + xsdt_ptr->header.length);

            acpi_map_data((void*)range_begin, range_end - range_begin);
            
            /* Parse each SDT of the RSDT */
            while(range_begin < range_end)
            {
                address = (acpi_header_t*)(*(uintptr_t*)range_begin);
                acpi_parse_dt((acpi_header_t*)address);
                range_begin += sizeof(uintptr_t);
            }
        }
        else 
        {
            KERNEL_ERROR("XSDT Signature comparison failed\n");
            KERNEL_PANIC(OS_ERR_CHECKSUM_FAILED);
        }
    }
    else
    {
        KERNEL_ERROR("XSDT Checksum failed\n");
        KERNEL_PANIC(OS_ERR_CHECKSUM_FAILED);
    }
}

/**
 * @brief Use the APIC RSDP to parse the ACPI infomation.
 * 
 * @details Use the APIC RSDP to parse the ACPI infomation. The function will 
 * detect the RSDT or XSDT pointed and parse them.
 *
 * @param[in] rsdp_desc The RSDP to walk.
 */
static void acpi_parse_rsdp(rsdp_descriptor_t* rsdp_desc)
{
    uint8_t              sum;
    uint8_t              i;
    uintptr_t            xsdt_addr;
    rsdp_descriptor_2_t* extended_rsdp;

    if(rsdp_desc == NULL)
    {
        KERNEL_ERROR("Tried to parse a NULL RSDP\n");
        KERNEL_PANIC(OS_ERR_NULL_POINTER);
    }

    KERNEL_DEBUG(ACPI_DEBUG_ENABLED, "[ACPI] Parsing RSDP at 0x%p", rsdp_desc);

    acpi_map_data(rsdp_desc, sizeof(rsdp_descriptor_t));

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
            acpi_parse_rsdt((rsdt_descriptor_t*)rsdp_desc->rsdt_address);          
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
                    acpi_parse_xsdt((xsdt_descriptor_t*)xsdt_addr);
                }
                else
                {
                    acpi_parse_rsdt((rsdt_descriptor_t*)
                                    rsdp_desc->rsdt_address);
                }
            }
            else 
            {
                KERNEL_ERROR("Extended RSDP Checksum failed\n");
                KERNEL_PANIC(OS_ERR_CHECKSUM_FAILED);
            }

        }
        else
        {
            KERNEL_ERROR("Unsupported ACPI version %d\n", rsdp_desc->revision);
            KERNEL_PANIC(OS_ERR_ACPI_UNSUPPORTED);
        }
    }
    else
    {
        KERNEL_ERROR("RSDP Checksum failed\n");
        KERNEL_PANIC(OS_ERR_CHECKSUM_FAILED);
    }
}

OS_RETURN_E acpi_init(void)
{
    uint8_t*    range_begin;
    uint8_t*    range_end;
    uint64_t    signature;
    OS_RETURN_E err;
    
    err = OS_NO_ERR;

    /* Init pointers */
    madt = NULL;
    dsdt = NULL;

    acpi_mapping = NULL;

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
    acpi_map_data(range_begin, range_end - range_begin);

    /* Search for ACPI table */
    while (range_begin < range_end)
    {
        signature = *(uint64_t*)range_begin;

        /* Checking the RSDP signature */
        if(signature == ACPI_RSDP_SIG)
        {
            KERNEL_DEBUG(ACPI_DEBUG_ENABLED, 
                         "[ACPI] RSDP found at 0x%p", 
                         range_begin);

            /* Parse RSDP */
            acpi_parse_rsdp((rsdp_descriptor_t*)range_begin);
            break;
        }

        range_begin += sizeof(uintptr_t);
    }

#ifdef TEST_MODE_ENABLED
    acpi_test();
#endif

    acpi_initialized = 1;
    
    return err;
}

int32_t acpi_get_io_apic_count(void)
{
    if(acpi_initialized != 1)
    {
        return -1;
    }

    return io_apic_count;
}

int32_t acpi_get_lapic_count(void)
{
    if(acpi_initialized != 1)
    {
        return -1;
    }

    return cpu_count;
}

int32_t acpi_get_remaped_irq(const uint32_t irq_number)
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

const queue_t* acpi_get_io_apics(void)
{
    return io_apics;
}

int32_t get_cpu_count(void)
{
    if(cpu_count == 0 || acpi_initialized == 0)
    {
        return 1;
    }
    return cpu_count;
}

int32_t cpu_get_id(void)
{   
    uint32_t i;
    uint32_t lapic_id;
    queue_node_t* node;
    local_apic_t* lapic;

    /* If lapic is not activated but we only use one CPU */
    if(cpu_count == 0 || acpi_initialized == 0)
    {
        return 0;
    }    

    node = cpu_lapics->tail;
    i = 0;
    lapic_id = lapic_get_id();
    while(node != NULL)
    {
        lapic = (local_apic_t*)node->data;
        if(lapic->apic_id == lapic_id)
        {
            return i;
        }
        node = node->prev;
        ++i;
    }

    return 0;
}