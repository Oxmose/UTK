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

/*******************************************************************************
 * INCLUDES
 ******************************************************************************/

/* Included headers */
#include <stdint.h>        /* Generic int types */
#include <string.h>        /* String manipualtion */
#include <kernel_output.h> /* Kernel output methods */
#include <memmgt.h>        /* Memory management */
#include <panic.h>         /* Kernel panic */
#include <vector.h>        /* Vector library */
#include <kheap.h>         /* Kernel heap */
#include <kernel_error.h>  /* Kernel error codes */
#include <arch_memmgt.h>   /* Atcihtecture memory management */
#include <lapic.h>         /* LAPIC driver */
#include <stddef.h>        /* Standard definitions */
#include <bsp_api.h>       /* BSP API */
#include <cpu_api.h>       /* CPU API */

/* Configuration files */
#include <config.h>
#include <test_bank.h>

/* Header file */
#include <acpi.h>

/*******************************************************************************
 * CONSTANTS
 ******************************************************************************/

/* APIC structure types */
/** @brief APIC type: local APIC. */
#define APIC_TYPE_LOCAL_APIC         0x0
/** @brief APIC type: IO APIC. */
#define APIC_TYPE_IO_APIC            0x1
/** @brief APIC type: interrupt override. */
#define APIC_TYPE_INTERRUPT_OVERRIDE 0x2
/** @brief APIC type: NMI. */
#define APIC_TYPE_NMI                0x4

/* ACPI SIGNATURE */
/** @brief ACPI memory signature: RSDP. */
#define ACPI_RSDP_SIG 0x2052545020445352
/** @brief ACPI memory signature: RSDT. */
#define ACPI_RSDT_SIG 0x54445352
/** @brief ACPI memory signature: XSDT. */
#define ACPI_XSDT_SIG 0x54445358
/** @brief ACPI memory signature: FACP. */
#define ACPI_FACP_SIG 0x50434146
/** @brief ACPI memory signature: FACS. */
#define ACPI_FACS_SIG 0x53434146
/** @brief ACPI memory signature: APIC. */
#define ACPI_APIC_SIG 0x43495041
/** @brief ACPI memory signature: DSDT. */
#define ACPI_DSDT_SIG 0x54445344

/** @brief Maximal number od IO-APICs supported by the kernel. */
#define MAX_IO_APIC_COUNT 1

/*******************************************************************************
 * STRUCTURES AND TYPES
 ******************************************************************************/

/** @brief ACPI structure header.
 * Please check the ACPI standard for more information.
 */
typedef struct acpi_header
{
    char        signature[4];
    uint32_t    length;
    uint8_t     revision;
    uint8_t     checksum;

    char        oem[6];
    char        oem_table_id[8];
    uint32_t    oem_revision;

    uint32_t    creator_id;
    uint32_t    creator_revision;
} __attribute__((__packed__)) acpi_header_t;

/** @brief ACPI RSDP descriptor.
 * Please check the ACPI standard for more information.
 */
typedef struct rsdp_descriptor
{
    char            signature[8];
    uint8_t         checksum;
    char            oemid[6];
    uint8_t         revision;
    uint32_t        rsdt_address;
} __attribute__ ((packed)) rsdp_descriptor_t;

/** @brief ACPI extended RSDP descriptor.
 * Please check the ACPI standard for more information.
 */
typedef struct rsdp_descriptor_2
{
    rsdp_descriptor_t first_part;

    uint32_t length;
    uint64_t xsdt_address;
    uint8_t  extended_checksum;
    uint8_t  reserved[3];
} __attribute__ ((packed)) rsdp_descriptor_2_t;

/** @brief ACPI RSDT descriptor.
 * Please check the ACPI standard for more information.
 */
typedef struct rsdt_descriptor
{
    acpi_header_t header;
    uint32_t      *dt_pointers;
} __attribute__ ((packed)) rsdt_descriptor_t;

/** @brief ACPI XSDT descriptor.
 * Please check the ACPI standard for more information.
 */
typedef struct xsdt_descriptor
{
    acpi_header_t header;
    uint64_t      *dt_pointers;
} __attribute__ ((packed)) xsdt_descriptor_t;

/** @brief ACPI address descriptor.
 * Please check the ACPI standard for more information.
 */
typedef struct generic_address
{
    uint8_t   address_space;

    uint8_t   bit_width;
    uint8_t   bit_offset;

    uint8_t   access_size;

    uint64_t  address;
} __attribute__((__packed__)) generic_address_t;

/** @brief ACPI FADT descriptor.
 * Please check the ACPI standard for more information.
 */
typedef struct acpi_fadt
{
    acpi_header_t      header;

    uint32_t            firmware_control;
    uint32_t            dsdt;

    uint8_t             reserved0;

    uint8_t             preferred_pm_profile;
    uint16_t            sci_interrupt;
    uint32_t            smi_command_port;

    uint8_t             acpi_enable;
    uint8_t             acpi_disable;

    uint8_t             S4BIOS_req;
    uint8_t             PSTATE_control;

    uint32_t            PM1_a_event_block;
    uint32_t            PM1_b_event_block;

    uint32_t            PM1_a_control_block;
    uint32_t            PM1_b_control_block;

    uint32_t            PM2_control_block;

    uint32_t            PM_timer_block;

    uint32_t            GPE0_block;
    uint32_t            GPE1_block;

    uint8_t             PM1_event_length;

    uint8_t             PM1_control_length;
    uint8_t             PM2_control_length;

    uint8_t             PM_timer_length;

    uint8_t             GPE0_length;
    uint8_t             GPE1_length;

    uint8_t             GPE1_base;

    uint8_t             C_state_control;

    uint16_t            worst_C2_latency;
    uint16_t            worst_C3_latency;

    uint16_t            flush_size;
    uint16_t            flush_stride;

    uint8_t             duty_offset;
    uint8_t             duty_width;

    uint8_t             day_alarm;
    uint8_t             month_alarm;

    uint8_t             century;

    uint16_t            boot_architecture_flags;

    uint8_t             reserved1;

    uint32_t            flags;

    generic_address_t   reset_reg;

    uint8_t             reset_value;

    uint8_t             reserved2[3];

    uint64_t            X_firmware_control;
    uint64_t            X_dsdt;

    generic_address_t   X_PM1_a_event_block;
    generic_address_t   X_PM1_b_event_block;

    generic_address_t   X_PM1_a_control_block;
    generic_address_t   X_PM1_b_control_block;

    generic_address_t   X_PM2_control_block;

    generic_address_t   X_PM_timer_block;

    generic_address_t   X_GPE0_block;
    generic_address_t   X_GPE1_block;

} __attribute__((__packed__)) acpi_fadt_t;

/** @brief ACPI FACS descriptor.
 * Please check the ACPI standard for more information.
 */
typedef struct acpi_facs
{
    acpi_header_t      header;

}  __attribute__((__packed__)) acpi_facs_t;

/** @brief ACPI DSDT descriptor.
 * Please check the ACPI standard for more information.
 */
typedef struct acpi_dsdt
{
    acpi_header_t      header;

}  __attribute__((__packed__)) acpi_dsdt_t;

/** @brief ACPI MADT descriptor.
 * Please check the ACPI standard for more information.
 */
typedef struct acpi_madt
{
    acpi_header_t   header;

    uint32_t            local_apic_addr;
    uint32_t            flags;
} __attribute__((__packed__)) acpi_madt_t;

/** @brief ACPI Interrupt override descriptor.
 * Please check the ACPI standard for more information.
 */
typedef struct apic_interrupt_override
{
    apic_header_t   header;

    uint8_t         bus;
    uint8_t         source;
    uint32_t        interrupt;
    uint16_t        flags;
} __attribute__((__packed__)) apic_interrupt_override_t;

/** @brief ACPI NMI descriptor.
 * Please check the ACPI standard for more information.
 */
typedef struct local_apic_nmi
{
    uint8_t         processors;
    uint16_t        flags;
    uint8_t         lint_id;
} __attribute__((__packed__)) local_apic_nmi_t;

/** @brief ACPI mapping tree node. */
typedef struct acpi_page_tree
{
    /** @brief Left node. */
    struct acpi_page_tree* left;
    /** @brief Right node. */
    struct acpi_page_tree* right;

    /** @brief Address stored in this node. */
    uintptr_t address;
} acpi_page_tree_t;

/*******************************************************************************
 * MACROS
 ******************************************************************************/

/**
 * @brief Assert macro used by the ACPI to ensure correctness of execution.
 *
 * @details Assert macro used by the ACPI to ensure correctness of execution.
 * Due to the critical nature of the ACPI, any error generates a kernel panic.
 *
 * @param[in] COND The condition that should be true.
 * @param[in] MSG The message to display in case of kernel panic.
 * @param[in] ERROR The error code to use in case of kernel panic.
 */
#define ACPI_ASSERT(COND, MSG, ERROR) {                     \
    if((COND) == FALSE)                                     \
    {                                                       \
        PANIC(ERROR, "ACPI", MSG, TRUE);                    \
    }                                                       \
}

/*******************************************************************************
 * GLOBAL VARIABLES
 ******************************************************************************/

/************************* Imported global variables **************************/
/* None */

/************************* Exported global variables **************************/
/* None */

/************************** Static global variables ***************************/

/** @brief Stores the number of detected CPU. */
static uint32_t cpu_count;

/** @brief Stores the detected CPUs' lapics. */
static vector_t* cpu_lapics;

/** @brief Stores the number of detected IO APIC. */
static uint32_t io_apic_count;

/** @brief Stores the detected IO APICs' information table. */
static vector_t* io_apics;

/** @brief Stores the MADT descriptor's address in memory. */
static acpi_madt_t* madt;

/** @brief Stores the DSDT descriptor's address in memory. */
static acpi_dsdt_t* dsdt;

/** @brief Stores the ACPI initialization state. */
static bool_t acpi_initialized = FALSE;

/** @brief The ACPI mapping tree. */
static acpi_page_tree_t* acpi_mapping;

/*******************************************************************************
 * STATIC FUNCTIONS DECLARATIONS
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
 * @return The function return FALSE if the page was not found, otherwise TRUE
 * is returned.
 */
static bool_t walk_acpi_tree(acpi_page_tree_t* node, const uintptr_t addr);

/**
 * @brief Search a page in the ACPI mapped tree.
 *
 * @details Search a page in the ACPI mapped tree. The function return 0 if the
 * page was not found, otherwise 1 is returned.
 *
 * @param[in] addr The address of the mapped page to search.
 *
 * @return The function return FALSE if the page was not found, otherwise TRUE
 * is returned.
 */
static bool_t is_page_mapped(const uintptr_t addr);

/**
 * @brief Adds a mapped page node to the ACPI page tree.
 *
 * @details Adds a mapped page node to the ACPI page tree.
 *
 * @param[in] node The starting point node to walk the tree.
 * @param[in] addr The address of the mapped page to add.
 */
static void add_acpi_tree(acpi_page_tree_t* node, const uintptr_t addr);

/**
 * @brief Adds a mapped page to the ACPI page tree.
 *
 * @details Adds a mapped page to the ACPI page tree.
 *
 * @param[in] addr The address of the mapped page to add.
 */
static void add_mapped_page(uintptr_t addr);

/**
 * @brief Map ACPI memory.
 *
 * @details Map ACPI memory.
 *
 * @param[in] start_addr The address of the MADT entry to parse.
 * @param[in] size The size to map.
 *
 */
static void acpi_map_data(const void* start_addr, size_t size);

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
static void acpi_parse_apic(acpi_madt_t* madt_ptr);

/**
 * @brief Parse the APIC DSDT table.
 *
 * @details The function will save the DSDT table address in for further use.
 *
 * @param[in] dsdt_ptr The address of the DSDT entry to parse.
 */
static void acpi_parse_dsdt(acpi_dsdt_t* dsdt_ptr);

/**
 * @brief Parse the APIC FADT table.
 *
 * @details Parse the APIC FADT table. The function will save the FADT table
 * address in for further use. Then the FACS and DSDT addresses are extracted
 * and both tables are parsed.
 *
 * @param[in] fadt_ptr The address of the FADT entry to parse.
 */
static void acpi_parse_fadt(acpi_fadt_t* fadt_ptr);

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
static void acpi_parse_dt(acpi_header_t* header);

/**
 * @brief Parse the APIC RSDT table.
 *
 * @details Parse the APIC RSDT table. The function will detect the read each
 * entries of the RSDT and call the corresponding functions to parse the entries
 * correctly.
 *
 * @param[in] rsdt_ptr The address of the RSDT entry to parse.
 */
static void acpi_parse_rsdt(rsdt_descriptor_t* rsdt_ptr);

/**
 * @brief Parse the APIC XSDT table.
 * @details The function will detect the read each entries of the XSDT and call
 * the corresponding functions to parse the entries correctly.
 *
 * @param[in] xsdt_ptr The address of the XSDT entry to parse.
 */
static void acpi_parse_xsdt(xsdt_descriptor_t* xsdt_ptr);

/**
 * @brief Use the APIC RSDP to parse the ACPI infomation.
 *
 * @details Use the APIC RSDP to parse the ACPI infomation. The function will
 * detect the RSDT or XSDT pointed and parse them.
 *
 * @param[in] rsdp_desc The RSDP to walk.
 */
static void acpi_parse_rsdp(rsdp_descriptor_t* rsdp_desc);

/*******************************************************************************
 * FUNCTIONS
 ******************************************************************************/

static bool_t walk_acpi_tree(acpi_page_tree_t* node, const uintptr_t addr)
{
    if(node == NULL)
    {
        return FALSE;
    }
    else if(node->address == addr)
    {
        return TRUE;
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

static bool_t is_page_mapped(const uintptr_t addr)
{
    return walk_acpi_tree(acpi_mapping, addr);
}

static void add_acpi_tree(acpi_page_tree_t* node, const uintptr_t addr)
{
    ACPI_ASSERT(node != NULL,
                "Cannot add NULL node to ACPI tree",
                OS_ERR_NULL_POINTER);

    ACPI_ASSERT(node->address != addr,
                "Cannot add an already existing node to ACPI tree",
                OS_ERR_UNAUTHORIZED_ACTION);

    /* Add on top of tree */
    if(addr > node->address)
    {
        if(node->right == NULL)
        {
            node->right = kmalloc(sizeof(acpi_page_tree_t));

            ACPI_ASSERT(node->right != NULL,
                        "Could not allocate ACPI node",
                        OS_ERR_MALLOC);

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

            ACPI_ASSERT(node->left != NULL,
                        "Could not allocate ACPI node",
                        OS_ERR_MALLOC);

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

static void add_mapped_page(uintptr_t addr)
{
    if(acpi_mapping == NULL)
    {
        acpi_mapping = kmalloc(sizeof(acpi_page_tree_t));

        ACPI_ASSERT(acpi_mapping != NULL,
                    "Could not allocate ACPI mapping tree",
                    OS_ERR_MALLOC);

        acpi_mapping->right = NULL;
        acpi_mapping->left  = NULL;
        acpi_mapping->address = addr;
    }
    else
    {
        add_acpi_tree(acpi_mapping, addr);
    }
}

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

            ACPI_ASSERT(err == OS_NO_ERR,
                        "Could not declare ACPI region",
                        err);

            memory_mmap_direct((void*)addr_align,
                                (void*)addr_align,
                                KERNEL_PAGE_SIZE,
                                1,
                                0,
                                0,
                                1,
                                NULL);

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

static void acpi_parse_apic(acpi_madt_t* madt_ptr)
{
    uint8_t        type;
    int32_t        sum;
    uint32_t       i;
    uintptr_t      madt_entry;
    uintptr_t      madt_limit;
    apic_header_t* header;
    OS_RETURN_E    err;

    cpu_count = 0;
    io_apic_count = 0;

    ACPI_ASSERT(madt_ptr != NULL,
                "Tried to parse a NULL MADT",
                OS_ERR_NULL_POINTER);

    acpi_map_data(madt_ptr, sizeof(acpi_madt_t));

    KERNEL_DEBUG(ACPI_DEBUG_ENABLED, "[ACPI] Parsing MADT at 0x%p", madt_ptr);

    /* Verify checksum */
    sum = 0;
    for(i = 0; i < madt_ptr->header.length; ++i)
    {
        sum += ((uint8_t*)madt_ptr)[i];
    }

    ACPI_ASSERT((sum & 0xFF) == 0,
                "MADT checksum failed",
                OS_ERR_CHECKSUM_FAILED);

    ACPI_ASSERT(*((uint32_t*)madt_ptr->header.signature) == ACPI_APIC_SIG,
                "Invalid MADT signature",
                OS_ERR_WRONG_SIGNATURE);


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
                err = vector_push(cpu_lapics, (void*)madt_entry);
                ACPI_ASSERT(err == OS_NO_ERR,
                            "Could not allocate node for new lapic",
                            err);

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
                err = vector_push(io_apics, (void*)madt_entry);
                ACPI_ASSERT(err == OS_NO_ERR,
                            "Could not allocate node for new IO-APIC",
                            err);

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

static void acpi_parse_dsdt(acpi_dsdt_t* dsdt_ptr)
{
    int32_t  sum;
    uint32_t i;

    ACPI_ASSERT(dsdt_ptr != NULL,
                "Tried to parse a NULL DSDT",
                OS_ERR_NULL_POINTER);

    acpi_map_data(dsdt_ptr, sizeof(acpi_dsdt_t));

    KERNEL_DEBUG(ACPI_DEBUG_ENABLED, "[ACPI] Parsing DSDT at 0x%p", dsdt_ptr);

    acpi_map_data(dsdt_ptr, dsdt_ptr->header.length);

    /* Verify checksum */
    sum = 0;

    for(i = 0; i < dsdt_ptr->header.length; ++i)
    {
        sum += ((uint8_t*)dsdt_ptr)[i];
    }

    ACPI_ASSERT((sum & 0xFF) == 0,
                "DSDT Checksum failed",
                OS_ERR_CHECKSUM_FAILED);

    ACPI_ASSERT(*((uint32_t*)dsdt_ptr->header.signature) == ACPI_DSDT_SIG,
                "Wrong DSDT Signature",
                OS_ERR_WRONG_SIGNATURE);
}

static void acpi_parse_fadt(acpi_fadt_t* fadt_ptr)
{
    int32_t  sum;
    uint32_t i;

    ACPI_ASSERT(fadt_ptr != NULL,
                "Tried to parse a NULL FADT",
                OS_ERR_NULL_POINTER);

    acpi_map_data(fadt_ptr, sizeof(acpi_fadt_t));

    KERNEL_DEBUG(ACPI_DEBUG_ENABLED,"[ACPI] Parsing FADT at 0x%p", fadt_ptr);

    /* Verify checksum */
    sum = 0;
    for(i = 0; i < fadt_ptr->header.length; ++i)
    {
        sum += ((uint8_t*)fadt_ptr)[i];
    }

    ACPI_ASSERT((sum & 0xFF) == 0,
                "FADT Checksum failed",
                OS_ERR_CHECKSUM_FAILED);

    ACPI_ASSERT(*((uint32_t*)fadt_ptr->header.signature) == ACPI_FACP_SIG,
                "FADT Signature comparison failed",
                OS_ERR_WRONG_SIGNATURE);

    /* Parse DSDT */
    acpi_parse_dsdt((acpi_dsdt_t*)fadt_ptr->dsdt);
}

static void acpi_parse_dt(acpi_header_t* header)
{
    char sig_str[5];

    ACPI_ASSERT(header != NULL,
                "Tried to parse a NULL DT",
                OS_ERR_NULL_POINTER);

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

static void acpi_parse_rsdt(rsdt_descriptor_t* rsdt_ptr)
{
    uintptr_t      range_begin;
    uintptr_t      range_end;
    acpi_header_t* address;
    int8_t         sum;
    uint8_t        i;

    ACPI_ASSERT(rsdt_ptr != NULL,
                "Tried to parse a NULL RSDT",
                OS_ERR_NULL_POINTER);

    acpi_map_data(rsdt_ptr, sizeof(rsdt_descriptor_t));

    KERNEL_DEBUG(ACPI_DEBUG_ENABLED, "[ACPI] Parsing RSDT at 0x%p", rsdt_ptr);

    /* Verify checksum */
    sum = 0;
    for(i = 0; i < rsdt_ptr->header.length; ++i)
    {
        sum += ((uint8_t*)rsdt_ptr)[i];
    }

    ACPI_ASSERT((sum & 0xFF) == 0,
                "RSDT Checksum failed",
                OS_ERR_CHECKSUM_FAILED);

    ACPI_ASSERT(*((uint32_t*)rsdt_ptr->header.signature) == ACPI_RSDT_SIG,
                "Wrong RSDT Signature",
                OS_ERR_WRONG_SIGNATURE);

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

static void acpi_parse_xsdt(xsdt_descriptor_t* xsdt_ptr)
{
    uintptr_t      range_begin;
    uintptr_t      range_end;
    acpi_header_t* address;
    int8_t         sum;
    uint8_t        i;

    ACPI_ASSERT(xsdt_ptr != NULL,
                "Tried to parse a NULL XSDT",
                OS_ERR_NULL_POINTER);

    acpi_map_data(xsdt_ptr, sizeof(xsdt_descriptor_t));

    KERNEL_DEBUG(ACPI_DEBUG_ENABLED, "[ACPI] Parsing XSDT at 0x%p", xsdt_ptr);

    /* Verify checksum */
    sum = 0;
    for(i = 0; i < xsdt_ptr->header.length; ++i)
    {
        sum += ((uint8_t*)xsdt_ptr)[i];
    }

    ACPI_ASSERT((sum & 0xFF) == 0,
                "XSDT Checksum failed",
                OS_ERR_CHECKSUM_FAILED);

    ACPI_ASSERT(*((uint32_t*)xsdt_ptr->header.signature) == ACPI_XSDT_SIG,
                "Wrong XSDT Signature",
                OS_ERR_WRONG_SIGNATURE);

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

static void acpi_parse_rsdp(rsdp_descriptor_t* rsdp_desc)
{
    uint8_t              sum;
    uint8_t              i;
    uintptr_t            xsdt_addr;
    rsdp_descriptor_2_t* extended_rsdp;

    ACPI_ASSERT(rsdp_desc != NULL,
                "Tried to parse a NULL RSDP",
                OS_ERR_NULL_POINTER);

    KERNEL_DEBUG(ACPI_DEBUG_ENABLED, "[ACPI] Parsing RSDP at 0x%p", rsdp_desc);

    acpi_map_data(rsdp_desc, sizeof(rsdp_descriptor_t));

    /* Verify checksum */
    sum = 0;
    for(i = 0; i < sizeof(rsdp_descriptor_t); ++i)
    {
        sum += ((uint8_t*)rsdp_desc)[i];
    }

    ACPI_ASSERT((sum & 0xFF) == 0,
                "RSDP Checksum failed",
                OS_ERR_CHECKSUM_FAILED);

    KERNEL_DEBUG(ACPI_DEBUG_ENABLED,
                    "[ACPI] Revision %d detected",
                    rsdp_desc->revision);

    ACPI_ASSERT((rsdp_desc->revision == 0 || rsdp_desc->revision == 2),
                "Unsupported ACPI version",
                OS_ERR_NOT_SUPPORTED);

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

    }
}

void acpi_init(void)
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


    cpu_lapics = vector_create(VECTOR_ALLOCATOR(kmalloc, kfree), NULL, 0, &err);
    ACPI_ASSERT((cpu_lapics != NULL && err == OS_NO_ERR),
                "Could not create LAPIC vector",
                err);

    io_apics = vector_create(VECTOR_ALLOCATOR(kmalloc, kfree), NULL, 0, &err);
    ACPI_ASSERT((io_apics != NULL && err == OS_NO_ERR),
                "Could not create IO-APIC vector",
                err);

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

    KERNEL_TEST_POINT(acpi_test)

    /* Ensure as little space as possible is used */
    err = vector_shrink_to_fit(io_apics);
    ACPI_ASSERT(err == OS_NO_ERR,
                "Could not shrink IO-APIC structures",
                err);

    err = vector_shrink_to_fit(cpu_lapics);
    ACPI_ASSERT(err == OS_NO_ERR,
                "Could not shrink LAPIC structures",
                err);

    acpi_initialized = TRUE;
}

int32_t acpi_get_io_apic_count(void)
{
    if(acpi_initialized != TRUE)
    {
        return -1;
    }

    return io_apic_count;
}

int32_t acpi_get_lapic_count(void)
{
    if(acpi_initialized != TRUE)
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

    if(acpi_initialized != TRUE)
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
    size_t i;

    if(acpi_initialized != TRUE || madt == NULL || io_apic_id >= io_apic_count)
    {
        return NULL;
    }

    for(i = 0; i < io_apics->size; ++i)
    {
        if(((io_apic_t*)io_apics->array[i])->apic_id == io_apic_id)
        {
            return io_apics->array[i];
        }
    }

    return NULL;
}

void* acpi_get_lapic_addr(void)
{
    if(acpi_initialized != TRUE || madt == NULL)
    {
        return NULL;
    }

    return (void*)madt->local_apic_addr;
}

OS_RETURN_E acpi_check_lapic_id(const uint32_t lapic_id)
{
    size_t i;

    if(acpi_initialized != TRUE)
    {
        return OS_ERR_NOT_INITIALIZED;
    }

    for(i = 0; i < cpu_lapics->size; ++i)
    {
        if(((local_apic_t*)cpu_lapics->array[i])->apic_id == lapic_id)
        {
            return OS_NO_ERR;
        }
    }

    return OS_ERR_NO_SUCH_ID;
}


int32_t get_cpu_count(void)
{
    if(cpu_count == 0 || acpi_initialized == FALSE)
    {
        return 1;
    }
    return cpu_count;
}

int32_t cpu_get_id(void)
{
    uint32_t lapic_id;
    size_t   i;

    /* If lapic is not activated but we only use one CPU */
    if(cpu_count == 0 || acpi_initialized == FALSE)
    {
        return 0;
    }

    lapic_id = lapic_get_id();

    for(i = 0; i < cpu_lapics->size; ++i)
    {
        if(((local_apic_t*)cpu_lapics->array[i])->apic_id == lapic_id)
        {
            return i;
        }
    }

    return 0;
}

const vector_t* acpi_get_io_apics(void)
{
    return io_apics;
}

/************************************ EOF *************************************/