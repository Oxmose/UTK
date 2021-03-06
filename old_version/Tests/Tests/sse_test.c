#include <Core/scheduler.h>
#include <Lib/stdio.h>
#include <Drivers/lapic.h>
#include <Sync/mutex.h>
#include <Sync/semaphore.h>
#include <Memory/kheap.h>
#include <Tests/test_bank.h>

#if SSE_TEST == 1
#include <Core/scheduler.h>
#include <Lib/stdio.h>
#include <Drivers/lapic.h>
#include <Sync/mutex.h>
#include <Sync/semaphore.h>
#include <Memory/kheap.h>

static uint8_t arrayTabF[256];
static uint8_t arrayTabT[256];

semaphore_t sem_sync;

void testsse(void)
{
    __asm__ __volatile__ (
        "movups (%0), %%xmm0\n\t" 
        "movntdq %%xmm0, (%1)\n\t"
    ::"r"(arrayTabF), "r"(arrayTabT) : "memory");
}

void* thread_2(void* args)
{
    int i;
    (void) args;

    for(i = 0; i < 3; ++i)
    {
        sem_pend(&sem_sync);

        testsse();

        printf("[TESTMODE] SSE Thread 2 (%d) passed\n", i);

        sem_post(&sem_sync);
        sched_sleep(100);
    }   
    sem_pend(&sem_sync);
    testsse();
    printf("[TESTMODE] SSE Thread 2 (%d) passed\n", i);
    testsse();
    printf("[TESTMODE] SSE Thread 2 (%d) passed\n", i + 1);
    return NULL;
}

void sse_test_entry(void)
{
    testsse();
    printf("[TESTMODE] SSE Thread 1 (1) passed\n");
    testsse();
    printf("[TESTMODE] SSE Thread 1 (2) passed\n");
    testsse();
    printf("[TESTMODE] SSE Thread 1 (3) passed\n");
    sem_post(&sem_sync);
    sched_sleep(100);
    sem_pend(&sem_sync);
    testsse();
    printf("[TESTMODE] SSE Thread 1 (4) passed\n");
    sem_post(&sem_sync);
    sched_sleep(100);
    sem_pend(&sem_sync);  
    testsse();
    printf("[TESTMODE] SSE Thread 1 (5) passed\n");
    testsse();
    printf("[TESTMODE] SSE Thread 1 (6) passed\n");
    sem_post(&sem_sync);
    sched_sleep(100);
    sem_pend(&sem_sync);
    testsse();
    printf("[TESTMODE] SSE Thread 1 (7) passed\n");
    sem_post(&sem_sync);
}

void sse_test(void)
{
    OS_RETURN_E err;
    static thread_t thread_sem1;
    if((err = sem_init(&sem_sync, 0)) != OS_NO_ERR)
    {
        kernel_error("Failed to init sem1\n");
        return;
    }

    if(sched_create_kernel_thread(&thread_sem1, 1, "thread1", 1024, 0, thread_2, NULL) != OS_NO_ERR)
    {
        kernel_error(" Error while creating the main thread!\n");
        return;
    }

    sse_test_entry();

    if((err = sched_wait_thread(thread_sem1, NULL, NULL)) != OS_NO_ERR)
    {
        kernel_error("Error while waiting thread! [%d]\n", err);
        return;
    }

    if((err = sem_destroy(&sem_sync)) != OS_NO_ERR)
    {
        kernel_error("Failed to destroy sem1\n");
        return;
    }
}

#else
void sse_test(void)
{

}
#endif