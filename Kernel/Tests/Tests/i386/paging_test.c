
#include <lib/stdio.h>
#include <lib/stdint.h>
#include <lib/stddef.h>
#include <io/kernel_output.h>
#include <Tests/test_bank.h>
#include <memory/paging.h>
#include <cpu.h>

#if PAGING_TEST == 1
volatile int display = 0;

void handler_err(const uintptr_t addr)
{
    if(!display)
    {
        display = 1;
        kernel_printf("[TESTMODE] Wrong Fault handler launched 0x%p\n", addr);
            /* Kill QEMU */
        cpu_outw(0x2000, 0x604);    
        while(1)
        {
            __asm__ ("hlt");
        }
    }    
}
void handler_good(const uintptr_t addr)
{
    if(!display)
    {
        display = 1;
        kernel_printf("[TESTMODE] Good Fault handler launched 0x%p\n", addr);
            /* Kill QEMU */
        cpu_outw(0x2000, 0x604);    
        while(1)
        {
            __asm__ ("hlt");
        }
    }    
}

void paging_test(void)
{
    OS_RETURN_E err;

    /* Try to add null pointer handler */
    err = paging_register_fault_handler(NULL, 0, 0);
    if(err != OS_ERR_NULL_POINTER)
    {
        kernel_error("0 | Wrong return value: %d\n", err);
    }

    /* Try to add end addres <= start address */
    err = paging_register_fault_handler(handler_err, 1900, 500);
    if(err != OS_ERR_UNAUTHORIZED_ACTION)
    {
        kernel_error("1 | Wrong return value: %d\n", err);
    }
    err = paging_register_fault_handler(handler_err, 1900, 1900);
    if(err != OS_ERR_UNAUTHORIZED_ACTION)
    {
        kernel_error("2 | Wrong return value: %d\n", err);
    }

    /* Add some ranges */
    err = paging_register_fault_handler(handler_err, 0x5000, 0x6000);
    if(err != OS_NO_ERR)
    {
        kernel_error("3 | Wrong return value: %d\n", err);

    }
    err = paging_register_fault_handler(handler_err, 0x6000, 0x6500);
    if(err != OS_NO_ERR)
    {
        kernel_error("4 | Wrong return value: %d\n", err);

    }
    err = paging_register_fault_handler(handler_err, 0x1000, 0x2000);
    if(err != OS_NO_ERR)
    {
        kernel_error("5 | Wrong return value: %d\n", err);

    }
    err = paging_register_fault_handler(handler_good, 0x3000, 0x4000);
    if(err != OS_NO_ERR)
    {
        kernel_error("6 | Wrong return value: %d\n", err);

    }

     /* Display list */
    const mem_handler_t* list = paging_get_handler_list();
    while(list)
    {
        kernel_printf("[TESTMODE] 0x%p -> 0x%p\n", list->start, list->end);
        list = list->next;
    }

    /* Add overlapping ranges */
    err = paging_register_fault_handler(handler_err, 0x4000, 0x5500);
    if(err != OS_ERR_HANDLER_ALREADY_EXISTS)
    {
        kernel_error("7 | Wrong return value: %d\n", err);

    }
    err = paging_register_fault_handler(handler_err, 0x6000, 0x6400);
    if(err != OS_ERR_HANDLER_ALREADY_EXISTS)
    {
        kernel_error("8 | Wrong return value: %d\n", err);

    }

     /* Display list */
    list = paging_get_handler_list();
    while(list)
    {
        kernel_printf("[TESTMODE] 0x%p -> 0x%p\n", list->start, list->end);
        list = list->next;
    }

   int* wrong_addr = (int*)0x3000;
   *wrong_addr = 0;

    kernel_printf("[TESTMODE] Test passed\n");
    /* Kill QEMU */
    cpu_outw(0x2000, 0x604);    
    while(1)
    {
        __asm__ ("hlt");
    }
}
#else
void paging_test(void)
{
}
#endif