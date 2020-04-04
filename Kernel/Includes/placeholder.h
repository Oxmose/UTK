


/* UTK configuration file */
#include <lib/stddef.h>
#include <cpu.h>
#include <cpu_structs.h>
#include <acpi.h>

int lapic_get_id(void);

void sched_set_thread_termination_cause(int value);
void sched_terminate_thread(void);

OS_RETURN_E lapic_set_int_eoi(int irq);