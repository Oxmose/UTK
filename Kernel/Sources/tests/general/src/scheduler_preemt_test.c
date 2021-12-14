#include <test_bank.h>

#if SCHEDULER_PREEMPT_TEST == 1

#include <kernel_output.h>
#include <scheduler.h>
#include <interrupts.h>
#include <string.h>

static char value[61] = {0};
static uint8_t out = 0;
static void* print_th_pre(void*args)
{
    uint32_t i = 0;
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
    for(i = 0; i < 100000000; ++i)
    {
        if(i % 5000000 == 0)
        {
            kernel_interrupt_disable();
            value[out++] = val;
            kernel_printf("%c",value[out-1]);
            kernel_interrupt_restore(1);
        }
    }
    return NULL;
}

void scheduler_preemt_test(void)
{
    kernel_thread_t* thread[4];
    OS_RETURN_E err;

    kernel_interrupt_restore(1);

    kernel_printf("[TESTMODE] Scheduler tests starts\n");;

    for(int i = 0; i < 3; ++i)
    {
        err = sched_create_kernel_thread(&thread[i], 5, "test",
                                  THREAD_TYPE_KERNEL, 0x1000, print_th_pre, (void*)i);
        if(err != OS_NO_ERR)
        {
            kernel_error("Cannot create threads %d\n", err);
            kill_qemu();
        }
    }

    for(int i = 0; i < 3; ++i)
    {
        sched_join_thread(thread[i], NULL, NULL);
    }
    kernel_printf("\n");
    if(strncmp(value, "--------------------********************....................", 60) == 0 ||
       strncmp(value, "--------------------....................********************", 60) == 0 ||
       strncmp(value, "********************--------------------....................", 60) == 0 ||
       strncmp(value, "....................--------------------********************", 60) == 0 ||
       strncmp(value, "********************....................--------------------", 60) == 0 ||
       strncmp(value, "....................********************--------------------", 60) == 0
    )
    {
        kernel_error("Scheduler thread premption tests error\n");
    }
    else
    {
        kernel_printf("[TESTMODE] Scheduler thread premption tests passed\n");
    }

    kernel_interrupt_disable();

    kill_qemu();
}
#else
void scheduler_preemt_test(void)
{

}
#endif