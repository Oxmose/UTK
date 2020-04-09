#include <io/kernel_output.h>
#include <core/scheduler.h>
#include <interrupt/interrupts.h>
#include <Tests/test_bank.h>
#include <cpu.h>
#include <comm/queue.h>

#if USERQUEUE_TEST == 1
static queue_t q1;
static thread_t thread1;
static thread_t thread2;

void* th1_fn(void*args)
{
    (void)args;

    for(uint32_t i = 0; i < 20; ++i)
    {
        queue_post(&q1, (void*)i);
        //printf("[TESTMODE] [1] Post %u\n", i);
        sched_sleep(10);
    }

    kernel_printf("[TESTMODE]THREAD1 return\n");
    return NULL;
}

void* th2_fn(void*args)
{
    (void)args;

    sched_sleep(1000);
    for(uint32_t i = 0; i < 20; ++i)
    {
        uint32_t val = (uint32_t)queue_pend(&q1, NULL);
        //printf("[TESTMODE][2] Pend %u\n", val);
        if(val != i)
        {
            kernel_error("Error, wrong value poped\n");
        }
    }

    kernel_printf("[TESTMODE]THREAD2 return\n");
    return NULL;
}

void* th3_fn(void*args)
{
    (void)args;

    OS_RETURN_E err;

    for(uint32_t i = 0; i < 22; ++i)
    {
        uint32_t val = (uint32_t)queue_pend(&q1, &err);
        //printf("[TESTMODE][3] Pend %u [%d]\n", val, err);
        if(i < 20 && val != i)
        {
            kernel_error("Error, wrong value poped\n");
        }
    }

    kernel_printf("[TESTMODE]THREAD3 return\n");
    return NULL;
}

void userqueue_test(void)
{
    OS_RETURN_E err;

    /* Create */
    err = queue_init(&q1, 5);
    if(err != OS_NO_ERR)
    {
        kernel_error("Error while initializing the queue [%d]\n", err);

    }

    /* Delete */
    err = queue_destroy(&q1);
    if(err != OS_NO_ERR)
    {
        kernel_error("Error while destroying the queue [%d]\n", err);

    }

    /* Create */
    err = queue_init(&q1, 10);
    if(err != OS_NO_ERR)
    {
        kernel_error("Error while initializing the queue [%d]\n", err);

    }

    if(sched_create_kernel_thread(&thread1, 1, "thread1", 1024, 0, th1_fn, NULL) != OS_NO_ERR)
    {
        kernel_error("Error while creating the main thread!\n");

    }
    if(sched_create_kernel_thread(&thread2, 1, "thread2", 1024, 0, th2_fn, NULL) != OS_NO_ERR)
    {
        kernel_error("Error while creating the main thread!\n");

    }

    if((err = sched_wait_thread(thread1, NULL, NULL)) != OS_NO_ERR)
    {
        kernel_error("Error while waiting thread! [%d]\n", err);

    }
    if((err = sched_wait_thread(thread2, NULL, NULL)) != OS_NO_ERR)
    {
        kernel_error("Error while waiting thread! [%d]\n", err);

    }
    /* Delete */
    err = queue_destroy(&q1);
    if(err != OS_NO_ERR)
    {
        kernel_error("Error while destroying the queue [%d]\n", err);

    }
    /* Create */
    err = queue_init(&q1, 10);
    if(err != OS_NO_ERR)
    {
        kernel_error("Error while initializing the queue [%d]\n", err);

    }

    /* Is empty tests */
    if(queue_isempty(&q1, &err) != 1)
    {
        kernel_error("Error, the mauilbox should be empty[%d]\n", err);

    }

    if(sched_create_kernel_thread(&thread1, 1, "thread1", 1024, 0, th1_fn, NULL) != OS_NO_ERR)
    {
        kernel_error("Error while creating the main thread!\n");

    }
    if(sched_create_kernel_thread(&thread2, 1, "thread2", 1024, 0, th3_fn, NULL) != OS_NO_ERR)
    {
        kernel_error("Error while creating the main thread!\n");

    }

    if((err = sched_wait_thread(thread1, NULL, NULL)) != OS_NO_ERR)
    {
        kernel_error("Error while waiting thread! [%d]\n", err);

    }

    /* Delete while thread is waiting */
    err = queue_destroy(&q1);
    if(err != OS_NO_ERR)
    {
        kernel_error("Error while destroying the queue [%d]\n", err);

    }

    if((err = sched_wait_thread(thread2, NULL, NULL)) != OS_NO_ERR)
    {
        kernel_error("Error while waiting thread! [%d]\n", err);

    }

    /* Create */
    err = queue_init(&q1, 10);
    if(err != OS_NO_ERR)
    {
        kernel_error("Error while initializing the queue [%d]\n", err);

    }

    err = queue_post(&q1, (void*)3);
    if(err != OS_NO_ERR)
    {
        kernel_error("Error while posting the queue (3)[%d]\n", err);

    }

    err = queue_post(&q1, (void*)3);
    if(err != OS_NO_ERR)
    {
        kernel_error("Error while posting the queue (3)[%d]\n", err);

    }

    /* Is empty tests */
    if(queue_isempty(&q1, &err) != 0)
    {
        kernel_error("Error, the mauilbox should be empty[%d]\n", err);

    }

    if(queue_size(&q1, &err) != 2)
    {
        kernel_error("Error, the mauilbox should be empty[%d]\n", err);

    }

    /* Delete */
    err = queue_destroy(&q1);
    if(err != OS_NO_ERR)
    {
        kernel_error("Error while destroying the queue [%d]\n", err);

    }

    kernel_printf("[TESTMODE] Queue test passed.\n");

    /* Kill QEMU */
    cpu_outw(0x2000, 0x604);    
    while(1)
    {
        __asm__ ("hlt");
    }
}
#else
void userqueue_test(void)
{

}
#endif