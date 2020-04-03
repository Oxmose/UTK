/* UTK configuration file */
#include <config.h>
#include <cpu.h>
#include <cpu_structs.h>
#include <lib/stddef.h>
#include <placeholder.h>
/* Tests header file */
#if TEST_MODE_ENABLED
#include <Tests/test_bank.h>
#endif

local_apic_t current;

const local_apic_t* place0[2] = {
    &current,
    &current
};

void kernel_panic(unsigned int error)
{
    (void)error;
    while(1);
}

void panic(void)
{
    while(1);
}

int acpi_check_lapic_id(int al)
{
    (void) al;
    return 0;
}

const local_apic_t** acpi_get_cpu_lapics(void)
{
    return place0;
}

int lapic_get_id(void)
{
    return 0;
}

void sched_set_thread_termination_cause(int value)
{
    (void) value;
}

void sched_terminate_thread(void)
{

}