#include <interrupt/interrupts.h>
#include <io/kernel_output.h>
#include <Tests/test_bank.h>
#include <io/graphic.h>
#include <lib/string.h>
#include <core/scheduler.h>
#include <comm/mailbox.h>
#include <lib/stdio.h>
#include <cpu.h>

#if MAILBOX_TEST == 1
static mailbox_t mb1;
static mailbox_t mb2;
static thread_t thread1;
static thread_t thread2;

void* thread1_fn(void*args)
{
    (void)args;
    for(uint32_t i = 0; i < 2; ++i)
    {
        OS_RETURN_E err;

        printf("[1] Sleep\n");
        sched_sleep(200);
        err = mailbox_post(&mb1, (void*)i);
        if(err != OS_NO_ERR)
        {
            kernel_error("Error while posting the mailbox (1)[%d]\n", err);
            return NULL;
        }
        printf("[1] Post %u\n", i);
        err = mailbox_post(&mb1, (void*)i);
        if(err != OS_NO_ERR)
        {
            kernel_error("Error while posting the mailbox (1)[%d]\n", err);
            return NULL;
        }
        printf("[1] Post %u\n", i);
        printf("[1] Pend\n");
        uint32_t val = (uint32_t) mailbox_pend(&mb2, &err);
        if(err != OS_NO_ERR)
        {
            kernel_error("Error while pending the mailbox (1)[%d]\n", err);
            return NULL;
        }
        printf("[1] Received %u\n", val);

    }
    printf("THREAD1 return\n");
    return NULL;
}

void* thread2_fn(void*args)
{
    (void)args;
    for(uint32_t i = 0; i < 2; ++i)
    {
        OS_RETURN_E err;

        printf("[2] Pend\n");
        uint32_t val = (uint32_t) mailbox_pend(&mb1, &err);
        if(err != OS_NO_ERR)
        {
            kernel_error("Error while pending the mailbox (2)[%d]\n", err);
            return NULL;
        }
        printf("[2] Received %u\n", val);
        val = (uint32_t) mailbox_pend(&mb1, &err);
        if(err != OS_NO_ERR)
        {
            kernel_error("Error while pending the mailbox (2)[%d]\n", err);
            return NULL;
        }
        printf("[2] Received %u\n", val);

        printf("[2] Sleep\n");
        sched_sleep(200);
        err = mailbox_post(&mb2, (void*)i);
        if(err != OS_NO_ERR)
        {
            kernel_error("Error while posting the mailbox (2)[%d]\n", err);
            return NULL;
        }

        printf("[2] Post %u\n", i);
    }
    printf("THREAD2 return\n");
    return NULL;
}

void* thread3_fn(void*args)
{
    (void)args;
    for(uint32_t i = 0; i < 3; ++i)
    {
        OS_RETURN_E err;

        printf("[3] Pend\n");
        uint32_t val = (uint32_t) mailbox_pend(&mb1, &err);
        if(err != OS_NO_ERR)
        {
            if(err == OS_ERR_MAILBOX_NON_INITIALIZED)
            {
                printf("[TESTMODE] Thread 3 detected mailbox as non init\n");
            }
            else
            {
                kernel_error("Error while pending the mailbox (3)[%d]\n", err);
            }
            return NULL;
        }
        printf("[3] Received %u\n", val);
        val = (uint32_t) mailbox_pend(&mb1, &err);
        if(err != OS_NO_ERR)
        {
            kernel_error("Error while pending the mailbox (3)[%d]\n", err);
            return NULL;
        }
        printf("[3] Received %u\n", val);

        printf("[3] Sleep\n");
        sched_sleep(200);
        err = mailbox_post(&mb2, (void*)i);
        if(err != OS_NO_ERR)
        {
            kernel_error("Error while posting the mailbox (3)[%d]\n", err);
            return NULL;
        }

        printf("[3] Post %u\n", i);
    }

    printf("THREAD3 return\n");

    return NULL;
}

void mailbox_test(void)
{
    OS_RETURN_E err;

    /* Create */
    err = mailbox_init(&mb1);
    if(err != OS_NO_ERR)
    {
        kernel_error("Error while initializing the mailbox [%d]\n", err);

    }

    /* Delete */
    err = mailbox_destroy(&mb1);
    if(err != OS_NO_ERR)
    {
        kernel_error("Error while destroying the mailbox [%d]\n", err);

    }

    /* Create */
    err = mailbox_init(&mb1);
    if(err != OS_NO_ERR)
    {
        kernel_error("Error while initializing the mailbox [%d]\n", err);

    }
    err = mailbox_init(&mb2);
    if(err != OS_NO_ERR)
    {
        kernel_error("Error while initializing the mailbox [%d]\n", err);

    }

    if(sched_create_kernel_thread(&thread1, 1, "thread1", 1024, 0, thread1_fn, NULL) != OS_NO_ERR)
    {
        kernel_error("Error while creating the main thread!\n");

    }
    if(sched_create_kernel_thread(&thread2, 1, "thread2", 1024, 0, thread2_fn, NULL) != OS_NO_ERR)
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
    err = mailbox_destroy(&mb1);
    if(err != OS_NO_ERR)
    {
        kernel_error("Error while destroying the mailbox [%d]\n", err);

    }
    err = mailbox_destroy(&mb2);
    if(err != OS_NO_ERR)
    {
        kernel_error("Error while destroying the mailbox [%d]\n", err);

    }

    /* Create */
    err = mailbox_init(&mb1);
    if(err != OS_NO_ERR)
    {
        kernel_error("Error while initializing the mailbox [%d]\n", err);

    }
    err = mailbox_init(&mb2);
    if(err != OS_NO_ERR)
    {
        kernel_error("Error while initializing the mailbox [%d]\n", err);

    }

    /* Is empty tests */
    if(mailbox_isempty(&mb1, &err) != 1)
    {
        kernel_error("Error, the mauilbox should be empty[%d]\n", err);

    }

    if(sched_create_kernel_thread(&thread1, 1, "thread1", 1024, 0, thread1_fn, NULL) != OS_NO_ERR)
    {
        kernel_error("Error while creating the main thread!\n");

    }
    if(sched_create_kernel_thread(&thread2, 1, "thread2", 1024, 0, thread3_fn, NULL) != OS_NO_ERR)
    {
        kernel_error("Error while creating the main thread!\n");

    }

    if((err = sched_wait_thread(thread1, NULL, NULL)) != OS_NO_ERR)
    {
        kernel_error("Error while waiting thread! [%d]\n", err);

    }

    /* Delete while thread is waiting */
    err = mailbox_destroy(&mb1);
    if(err != OS_NO_ERR)
    {
        kernel_error("Error while destroying the mailbox [%d]\n", err);

    }
    err = mailbox_destroy(&mb2);
    if(err != OS_NO_ERR)
    {
        kernel_error("Error while destroying the mailbox [%d]\n", err);

    }

    if((err = sched_wait_thread(thread2, NULL, NULL)) != OS_NO_ERR)
    {
        kernel_error("Error while waiting thread! [%d]\n", err);

    }

    /* Create */
    err = mailbox_init(&mb1);
    if(err != OS_NO_ERR)
    {
        kernel_error("Error while initializing the mailbox [%d]\n", err);

    }

    err = mailbox_post(&mb1, (void*)3);
    if(err != OS_NO_ERR)
    {
        kernel_error("Error while posting the mailbox (3)[%d]\n", err);

    }

    /* Is empty tests */
    if(mailbox_isempty(&mb1, &err) != 0)
    {
        kernel_error("Error, the mauilbox should be empty[%d]\n", err);

    }

    /* Delete */
    err = mailbox_destroy(&mb1);
    if(err != OS_NO_ERR)
    {
        kernel_error("Error while destroying the mailbox [%d]\n", err);
    }

    printf("[TESTMODE] Mailbox test passed.\n");

    /* Kill QEMU */
    cpu_outw(0x2000, 0x604);    
    while(1)
    {
        __asm__ ("hlt");
    }
}
#else
void mailbox_test(void)
{

}
#endif