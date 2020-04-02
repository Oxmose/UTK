/* UTK configuration file */
#include <config.h>

/* Tests header file */
#if TEST_MODE_ENABLED
#include <Tests/test_bank.h>
#endif

void kernel_interrupt_handler(void)
{
    
}

void kernel_panic(unsigned int error)
{
    (void)error;
    while(1);
}