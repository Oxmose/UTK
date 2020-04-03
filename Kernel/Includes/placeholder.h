


/* UTK configuration file */
#include <lib/stddef.h>
#include <cpu.h>
#include <cpu_structs.h>


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

void sched_set_thread_termination_cause(int value);
void sched_terminate_thread(void);