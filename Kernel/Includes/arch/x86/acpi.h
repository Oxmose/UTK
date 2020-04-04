/*******************************************************************************
 * @file acpi.h
 * 
 * @see acpi.c
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

#ifndef __X86_ACPI_H_
#define __X86_ACPI_H_

#include <lib/stdint.h> /* Generic int types */
#include <lib/stddef.h> /* Standard definitions */

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
 * STRUCTURES
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

/** @brief ACPI APIC descriptor header.
 * Please check the ACPI standard for more information.
 */
typedef struct apic_header
{
    uint8_t type;
    uint8_t length;
} __attribute__((__packed__)) apic_header_t;

/** @brief ACPI RSDP descriptor.
 * Please check the ACPI standard for more information.
 */
typedef struct io_apic
{
    apic_header_t   header;

    uint8_t         apic_id;

    uint8_t         reserved;

    uint32_t        io_apic_addr;
    uint32_t        global_system_interrupt_base;
} __attribute__((__packed__)) io_apic_t;

/** @brief ACPI LAPIC descriptor.
 * Please check the ACPI standard for more information.
 */
typedef struct local_apic
{
    apic_header_t   header;

    uint8_t         acpi_cpu_id;
    uint8_t         apic_id;
    uint32_t        flags;
} __attribute__((__packed__)) local_apic_t;

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

/*******************************************************************************
 * FUNCTIONS
 ******************************************************************************/

/**
 * @brief Initializes the ACPI parser.
 * 
 * @details Initializes all the ACPI structures. The function will search for 
 * the ACPI RSDP and then parse all the ACPI information. Each supported entry 
 * is stored for further use.
 *
 * @return The success state or the error code. 
 * - OS_NO_ERR is returned if no error is encountered. 
 * - OS_ERR_CHECKSUM_FAILED is returned if the ACPI is corrupted.
 * - OS_ERR_NULL_POINTER is returned if the ACPI contains errored memory 
 *   addresses.
 * - OS_ERR_ACPI_UNSUPPORTED is returned if the system's ACPI version is not 
 *   supported.
 */
OS_RETURN_E acpi_init(void);

/**
 * @brief Tells if an IO-APIC has been detected in the system.
 * 
 * @details Tells if an IO-APIC has been detected in the system. This function 
 * must be called after the init_acpi function.
 * 
 * @warning This function must be called after the init_acpi function.
 *
 * @return 1 if at least one IO-APIC have been detected, 0 otherwise. The
 * function will return -1 if the ACPI hass not been initialized before calling 
 * this function.
 */
int32_t acpi_get_io_apic_available(void);

/**
 * @brief Tells if a Local APIC has been detected in the system.
 * 
 * @details Tells if a Local APIC has been detected in the system. This function 
 * must be called after the init_acpi function.
 * 
 * @warning This function must be called after the init_acpi function.
 *
 * @return 1 if at least one Local APIC have been detected, 0 other wise. The
 * function will return -1 if the init_acpi function has not been called before
 * calling this function.
 */
int32_t acpi_get_lapic_available(void);

/**
 * @brief Checks if the IRQ has been remaped in the IO-APIC structure.
 * 
 * @details Checks if the IRQ has been remaped in the IO-APIC structure. This 
 * function must be called after the init_acpi function.
 * 
 * @warning This function must be called after the init_acpi function.
 *
 * @param[in] irq_number The initial IRQ number to check.
 *
 * @return The remapped IRQ number corresponding to the irq number given as
 * parameter. This function will return -1 if the init_acpi function has not 
 * been called before calling this function.
 */
int32_t acpi_get_remmaped_irq(const uint32_t irq_number);

/**
 * @brief Returns the Local APIC controller address of the current CPU.
 * 
 * @details Returns the Local APIC controller address. The Local APIC of the 
 * current CPU (the one that called the function) is used as reference to know
 * which local APIC to check. This function must be called after the init_acpi 
 * function.
 * 
 * @warning This function must be called after the init_acpi function.
 *
 * @return The Local APIC controller address. If the function has been called
 * before init_acpi, NULL is returned.
 */
void* acpi_get_lapic_addr(void);

/**
 * @brief Returns the IO-APIC controller address.
 * 
 * @details Returns the IO-APIC controller address. This function must be called 
 * after the init_acpi function.
 * 
 * @param[in] io_apic_id The ID of the IO APIC the user want the address of. The
 * id is the index of the IO APIC (starting from 0 to the maximal number of IO
 * APIC detected).
 *
 * @warning This function must be called after the init_acpi function.
 * 
 * @return The IO-APIC controller address. If the function has been called
 * before init_acpi, NULL is returned.
 */
const void* acpi_get_io_apic_address(const uint32_t io_apic_id);

/**
 * @brief Checks if the Local APIC id given as parameter exists in the system.
 * 
 * @details Checks if the Local APIC id given as parameter exists in the system. 
 * This function must be called after the init_acpi function.
 * 
 * @param[in] lapic_id The lapic ID to check the existence of.
 * 
 * @warning This function must be called after the init_acpi function.
 *
 * @return The success state or the error code. 
 * - OS_NO_ERR is returned if no error is encountered and the ID exists. 
 * - OS_ACPI_NOT_INITIALIZED is returned if the ACPI is not initialized.
 * - OS_ERR_NO_SUCH_LAPIC_ID is returned if the lapic ID does not exist in the
 *   system.
 */
OS_RETURN_E acpi_check_lapic_id(const uint32_t lapic_id);

/**
 * @brief Returns the number of CPU detected on the system. 
 * 
 * @details Returns the number of CPU detected on the system. This function must 
 * be called after the init_acpi function.
 * 
 * @warning This function must be called after the init_acpi function.
 *
 * @return The number of CPU detected in the system, -1 is returned on error.
 */
int32_t acpi_get_detected_cpu_count(void);

/**
 * @brief Returns a pointer to the array of CPU ids.
 * 
 * @details Returns a pointer to the array of CPU ids. The array might be wider 
 * than the number of CPU detected in the system. In that case, all data present
 * after the last detected CPU ID is not to be considered.
 * 
 * @warning This function must be called after the init_acpi function.
 * 
 * @return A pointer to the array of CPU ids.
 */
const uint32_t* acpi_get_cpu_ids(void);

/**
 * @brief Returns a pointer to the array of CPU lapics.
 * 
 * @details Returns a pointer to the array of CPU lapics. The array might be 
 * wider than the number of CPU detected in the system. In that case, all data 
 * present after the last detected CPU ID is not to be considered.
 * 
 * @warning This function must be called after the init_acpi function.
 * 
 * @return A pointer to the array of CPU lapics.
 */
const local_apic_t** acpi_get_cpu_lapics(void);

#endif /* #ifndef __X86_ACPI_H_ */