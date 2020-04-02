#define SCHEDULER_SW_INT_LINE 0x40


typedef struct apic_header
{
    unsigned char type;
    unsigned char  length;
} __attribute__((__packed__)) apic_header_t;


typedef struct local_apic
{
    apic_header_t   header;

    unsigned char          acpi_cpu_id;
    unsigned char          apic_id;
    unsigned int        flags;
} __attribute__((__packed__)) local_apic_t;

int acpi_check_lapic_id(int al);
const local_apic_t** acpi_get_cpu_lapics(void);

int lapic_get_id(void);

/** @brief Offset of the first line of an IRQ interrupt from PIC. */
#define INT_PIC_IRQ_OFFSET     0x30
/** @brief Master PIC spurious IRQ number. */
#define PIC_SPURIOUS_IRQ_MASTER 0x07
/** @brief Slave PIC spurious IRQ number. */
#define PIC_SPURIOUS_IRQ_SLAVE  0x0F