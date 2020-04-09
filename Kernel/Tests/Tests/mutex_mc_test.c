#include <core/scheduler.h>
#include <sync/mutex.h>
#include <io/kernel_output.h>
#include <lib/stdio.h>
#include <Tests/test_bank.h>
#include <cpu.h>

#if MUTEX_MC_TEST == 1
static thread_t thread_mutex1;
static thread_t thread_mutex2;

static mutex_t mutex1;

static volatile uint32_t lock_res;


void *mutex_thread_1(void *args)
{
    for(int i = 0; i < 200000; ++i)
    {
        if(mutex_pend(&mutex1))
        {
            printf("[TESTMODE]Failed to pend mutex1 1\n");
            return NULL;
        }

        uint32_t tmp = lock_res;
        for(volatile uint32_t k = 0; k < 200; ++k);

        lock_res = tmp + 1;

        if(mutex_post(&mutex1))
        {
            printf("[TESTMODE]Failed to post mutex1 1\n");
            return NULL;
        }
    }
    if(mutex_pend(&mutex1))
    {
        printf("[TESTMODE]Failed to pend mutex1 1\n");
        return NULL;
    }
    printf("(T1 END)\n");
    if(mutex_post(&mutex1))
    {
        printf("[TESTMODE]Failed to post mutex1 1\n");
        return NULL;
    }
    (void )args;
    return NULL;
}
void *mutex_thread_2(void *args)
{
    for(int i = 0; i < 200000; ++i)
    {
        if(mutex_pend(&mutex1))
        {
            printf("[TESTMODE]Failed to pend mutex1 2\n");
            return NULL;
        }

        uint32_t tmp = lock_res;
        for(volatile uint32_t k = 0; k < 200; ++k);

        lock_res = tmp + 1;


        if(mutex_post(&mutex1))
        {
            printf("[TESTMODE]Failed to post mutex1 2\n");
            return NULL;
        }
    }
    if(mutex_pend(&mutex1))
    {
        printf("[TESTMODE]Failed to pend mutex1 2\n");
        return NULL;
    }
    printf("(T2 END)\n");

    if(mutex_post(&mutex1))
    {
        printf("[TESTMODE]Failed to post mutex1 2\n");
        return NULL;
    }
    (void )args;
    return NULL;
}

void mutex_mc_test(void)
{
    OS_RETURN_E err;

    err = mutex_init(&mutex1, MUTEX_FLAG_NONE, MUTEX_PRIORITY_ELEVATION_NONE);
    if(err != OS_NO_ERR)
    {
        printf("[TESTMODE]Failed to init mutex1, %d\n", err);
    }

    lock_res = 0;

    if(sched_create_kernel_thread(&thread_mutex1, 1, "thread1", 0x1000, 0, mutex_thread_1, NULL) != OS_NO_ERR)
    {
        kernel_error(" Error while creating the main 1 thread!\n");
        return;
    }
    if(sched_create_kernel_thread(&thread_mutex2, 1, "thread1", 0x1000, 1, mutex_thread_2, NULL) != OS_NO_ERR)
    {
        kernel_error(" Error while creating the main 2 thread!\n");
        return;
    }

    if((err = sched_wait_thread(thread_mutex1, NULL, NULL)) != OS_NO_ERR)
    {
        kernel_error("Error while waiting thread! [%d]\n", err);
        return;
    }
    if(sched_wait_thread(thread_mutex2, NULL, NULL) != OS_NO_ERR)
    {
        kernel_error("Error while waiting thread! [%d]\n", err);
        return;
    }

    printf("[TESTMODE]Lock res = %u\n", lock_res);
    if(lock_res == 400000)
    {
        printf("[TESTMODE] Mutex test passed.\n");
    }

    /* Kill QEMU */
    cpu_outw(0x2000, 0x604);    
    while(1)
    {
        __asm__ ("hlt");
    }
}
#else
void mutex_mc_test(void)
{
}
#endif