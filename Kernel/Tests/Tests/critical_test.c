#include <config.h>

#include <interrupt/interrupts.h>
#include <io/kernel_output.h>
#include <cpu.h>
#include <time/time_management.h>
#include <Tests/test_bank.h>
#include <core/panic.h>
#include <interrupt_settings.h>
#include <lapic.h>
#include <rtc.h> 
#include <sync/critical.h>
#include <core/scheduler.h>
#include <lib/string.h>

#if CRITICAL_TEST == 1

static char value[61] = {0};
static uint8_t out = 0;
#if MAX_CPU_COUNT > 1
static spinlock_t lock = SPINLOCK_INIT_VALUE;
#endif
static void* print_th_pre(void*args)
{
    uint32_t i = 0;
    uint32_t word;
    char val;
    switch((int)args)
    {
        case 0:
            val = '-';
            break;
        case 1:
            val = '*';
            break;
        case 2:
            val = '.';
            break;
        default:
            val = '=';
    }
    #if MAX_CPU_COUNT > 1
 	ENTER_CRITICAL(word, &lock);
 	#else
 	ENTER_CRITICAL(word);
 	#endif
    for(i = 0; i < 100000000; ++i)
    {
        if(i % 5000000 == 0)
        {
            value[out++] = val;
            kernel_printf("%c",value[out-1]);
        }
    }
    #if MAX_CPU_COUNT > 1
 	EXIT_CRITICAL(word, &lock);
    #else
    EXIT_CRITICAL(word);
    #endif

    return NULL;
}

void critical_test(void)
{
    thread_t thread[3];
    OS_RETURN_E err;

    kernel_interrupt_restore(1);

    kernel_printf("[TESTMODE] Critical test starts\n");;

    for(int i = 0; i < 3; ++i)
    {
        err = sched_create_kernel_thread(&thread[i], 5, "test",
                                  0x1000, 0, print_th_pre, (void*)i);
        if(err != OS_NO_ERR)
        {
            kernel_error("Cannot create threads %d\n", err);
            kernel_panic(err);
        }
    }

    for(int i = 0; i < 3; ++i)
    {
        sched_wait_thread(thread[i], NULL, NULL);
    }
    kernel_printf("\n");
    if(strncmp(value, "--------------------********************....................", 60) != 0 &&
       strncmp(value, "--------------------....................********************", 60) != 0 &&
       strncmp(value, "********************--------------------....................", 60) != 0 &&
       strncmp(value, "....................--------------------********************", 60) != 0 &&
       strncmp(value, "********************....................--------------------", 60) != 0 &&
       strncmp(value, "....................********************--------------------", 60) != 0
    )
    {
        kernel_error("Scheduler thread critical tests error\n");
    }
    else
    {
        kernel_printf("[TESTMODE] Scheduler thread critical tests passed\n");
    }

    kernel_interrupt_disable();

    /* Kill QEMU */
    cpu_outw(0x2000, 0x604);    
    while(1)
    {
        __asm__ ("hlt");
    }
}
#else
void critical_test(void)
{

}
#endif