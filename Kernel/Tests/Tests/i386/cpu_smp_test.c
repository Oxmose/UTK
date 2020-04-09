
#include <lib/stdio.h>
#include <cpu.h>

#include <Tests/test_bank.h>

extern uint32_t main_core_id;

#if CPU_SMP_TEST  == 1
void cpu_smp_test(void)
{
    uint32_t cpu_id;

    cpu_id = cpu_get_id();

    if(cpu_id == main_core_id)
    {
        printf("[TESTMODE] All SMP CPU booted, test passed\n");
    }
    else 
    {
        printf("[TESTMODE] CPU %u booted\n", cpu_id);
        while(1)
        {
            cpu_hlt();
        }
    }

    /* Kill QEMU */
    cpu_outw(0x2000, 0x604);    
    while(1)
    {
        __asm__ ("hlt");
    }
}
#else 
void cpu_smp_test(void)
{
}
#endif