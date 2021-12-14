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

#include <stdint.h>       /* Generic int types */
#include <vector.h>       /* Vector library */
#include <kernel_error.h> /* Kernel error codes */

/*******************************************************************************
 * CONSTANTS
 ******************************************************************************/

/* None */

/*******************************************************************************
 * STRUCTURES
 ******************************************************************************/

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
 * @warning This function should be called with interrupt disabled.
 */
void acpi_init(void);

/**
 * @brief Returns the number of IO-APIC detected in the system.
 * 
 * @details Returns the number of IO-APIC detected in the system. This function 
 * must be called after the init_acpi function.
 *
 * @return The number of IO-APIC detected in the system. The function will
 * return -1 if the ACPI hass not been initialized before calling this function.
 */
int32_t acpi_get_io_apic_count(void);

/**
 * @brief Returns the number of Local APIC detected in the system.
 * 
 * @details Returns the number of Local APIC detected in the system. This 
 * function must be called after the init_acpi function.
 *
 * @return The number of Local APIC detected in the system. The function will
 * return -1 if the ACPI hass not been initialized before calling this function.
 */
int32_t acpi_get_lapic_count(void);

/**
 * @brief Checks if the IRQ has been remaped in the IO-APIC structure.
 * 
 * @details Checks if the IRQ has been remaped in the IO-APIC structure. This 
 * function must be called after the init_acpi function.
 * 
 *
 * @param[in] irq_number The initial IRQ number to check.
 *
 * @return The remapped IRQ number corresponding to the irq number given as
 * parameter. This function will return -1 if the init_acpi function has not 
 * been called before calling this function or no IO apic is detected.
 */
int32_t acpi_get_remaped_irq(const uint32_t irq_number);

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
 * @return The IO-APIC controller address. If the function has been called
 * before init_acpi, NULL is returned.
 */
const void* acpi_get_io_apic_address(const uint32_t io_apic_id);

/**
 * @brief Returns the Local APIC controller address of the current CPU.
 * 
 * @details Returns the Local APIC controller address. The Local APIC of the 
 * current CPU (the one that called the function) is used as reference to know
 * which local APIC to check. This function must be called after the init_acpi 
 * function.
 * 
 * @return The Local APIC controller address. If the function has been called
 * before init_acpi, NULL is returned.
 */
void* acpi_get_lapic_addr(void);

/**
 * @brief Checks if the Local APIC id given as parameter exists in the system.
 * 
 * @details Checks if the Local APIC id given as parameter exists in the system. 
 * This function must be called after the init_acpi function.
 * 
 * @param[in] lapic_id The lapic ID to check the existence of.
 *
 * @return The success state or the error code. 
 * - OS_NO_ERR is returned if no error is encountered and the ID exists. 
 * - OS_ACPI_NOT_INITIALIZED is returned if the ACPI is not initialized.
 * - OS_ERR_NO_SUCH_LAPIC_ID is returned if the lapic ID does not exist in the
 *   system.
 */
OS_RETURN_E acpi_check_lapic_id(const uint32_t lapic_id);

/**
 * @brief Returns the list of IO apics registered.
 * 
 * @details Returns the list of IO apics registered. The number of IO APIC
 * detected might differ from the actual number of the user choose to restrict
 * the numer of detected IO APICS.
 *
 * @return Returns the list of IO apics registered.
 */
const vector_t* acpi_get_io_apics(void);

#endif /* #ifndef __X86_ACPI_H_ */