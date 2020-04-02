/* UTK configuration file */
#include <config.h>

/* Tests header file */
#if TEST_MODE_ENABLED
#include <Tests/test_bank.h>
#endif

void kernel_interrupt_handler(void)
{
    
}

void kernel_kickstart(void)
{
    #if TEST_MODE_ENABLED
    boot_test();
    #endif
    while(1);
}