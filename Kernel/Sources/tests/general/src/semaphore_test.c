

#include <test_bank.h>

#if SEMAPHORE_TEST == 1

#include <kernel_output.h>
#include <scheduler.h>
#include <interrupts.h>
#include <semaphore.h>
#include <atomic.h>

static kernel_thread_t* thread_sem1;
static kernel_thread_t* thread_sem2;
static kernel_thread_t* thread_sem3;
static kernel_thread_t* thread_sem4;
static kernel_thread_t* thread_sem5;

static semaphore_t sem1;
static semaphore_t sem2;
static semaphore_t sem3;
static semaphore_t sem4;
static semaphore_t sem_end;

static volatile uint32_t lock_res;

void *sem_thread_1(void *args);
void *sem_thread_2(void *args);
void *sem_thread_3(void *args);
void *sem_thread_4(void *args);
void *sem_thread_5(void *args);

void *sem_thread_1(void *args)
{
    for(int i = 0; i < 3; ++i)
    {
        if(sem_pend(&sem1))
        {
            kernel_printf("Failed to pend sem1\n");
            (void )args;
            return NULL;
        }
        kernel_printf("\n[TESTMODE] (T1) ");
        ++lock_res;
        sched_sleep(500);
        if(sem_post(&sem2))
        {
            kernel_printf("Failed to post sem2\n");
            (void )args;
            return NULL;
        }
    }
    kernel_printf(" (T1 END) ");

    return NULL;
}
void *sem_thread_2(void *args)
{
    for(int i = 0; i < 3; ++i)
    {
        if(sem_pend(&sem2))
        {
            kernel_printf("Failed to pend sem2\n");
            (void )args;
            return NULL;
        }
        kernel_printf(" (T2) ");
        ++lock_res;
        sched_sleep(300);
        if(sem_post(&sem3))
        {
            kernel_printf("Failed to post sem3\n");
            (void )args;
            return NULL;
        }
    }
    kernel_printf(" (T2 END) ");

    return NULL;
}

void *sem_thread_3(void *args)
{
    for(int i = 0; i < 3; ++i)
    {
        if(sem_pend(&sem3))
        {
            kernel_printf("Failed to pend sem3\n");
            (void )args;
            return NULL;
        }
        kernel_printf(" (T3) ");
        ++lock_res;
        if(sem_post(&sem1))
        {
            kernel_printf("Failed to post sem1\n");
            (void )args;
            return NULL;
        }
    }
    if(sem_post(&sem_end))
    {
        kernel_printf("Failed to post sem_end\n");
        (void )args;
            return NULL;
    }
    kernel_printf(" (T3 END) ");

    return NULL;
}

void *sem_thread_4(void *args)
{
    int32_t val;
    if(sem_trypend(&sem4, &val) != OS_ERR_UNAUTHORIZED_ACTION)
    {
        kernel_printf("Failed to trypend sem4\n");
        (void )args;
            return NULL;
    }
    if(val == -1)
    {
        if(sem_post(&sem1))
        {
            kernel_printf("Failed to post sem1\n");
            (void )args;
            return NULL;
        }
    }
    else
    {
        kernel_printf("Failed to trypend sem4, wrong value\n");
        lock_res = 535;
        (void )args;
            return NULL;
    }
    for(int i = 0; i < 3; ++i)
    {
        OS_RETURN_E err;
        err = sem_pend(&sem4);
        if(err != OS_ERR_NOT_INITIALIZED)
        {
            kernel_printf("Failed to pend sem4,%d\n", i);
            lock_res = -3;

            (void )args;
            return NULL;
        }
    }
    kernel_printf(" (T4 END) ");

    return NULL;
}

void *sem_thread_5(void *args)
{
    for(int i = 0; i < 3; ++i)
    {
        OS_RETURN_E err;
        err = sem_pend(&sem4);
        if(err != OS_ERR_NOT_INITIALIZED)
        {
            kernel_printf("Failed to pend sem4,%d\n", i);
            lock_res = -3;

            (void )args;
            return NULL;
        }
    }
    kernel_printf(" (T5 END) ");

    return NULL;
}


void semaphore_test(void)
{
    OS_RETURN_E err;

    kernel_printf("[TESTMODE] Semaphore test start\n");

    if((err = sem_init(&sem1, 0)) != OS_NO_ERR)
    {
        kernel_error("Failed to init sem1\n");

    }
    if(sem_init(&sem2, 0) != OS_NO_ERR)
    {
        kernel_error("Failed to init sem2\n");

    }
    if(sem_init(&sem3, 0) != OS_NO_ERR)
    {
        kernel_error("Failed to init sem3\n");

    }
    if(sem_init(&sem4, -1) != OS_NO_ERR)
    {
        kernel_error("Failed to init sem4\n");

    }
    if(sem_init(&sem_end, 0) != OS_NO_ERR)
    {
        kernel_error("Failed to init sem_end\n");

    }

    lock_res = 0;

    if(sched_create_kernel_thread(&thread_sem1, 1, "thread1", THREAD_TYPE_KERNEL, 0x1000, sem_thread_1, NULL) != OS_NO_ERR)
    {
        kernel_error(" Error while creating the main thread!\n");

    }
    if(sched_create_kernel_thread(&thread_sem2, 2, "thread1", THREAD_TYPE_KERNEL, 0x1000, sem_thread_2, NULL) != OS_NO_ERR)
    {
        kernel_error(" Error while creating the main thread!\n");

    }
    if(sched_create_kernel_thread(&thread_sem3, 3, "thread1", THREAD_TYPE_KERNEL, 0x1000, sem_thread_3, NULL) != OS_NO_ERR)
    {
        kernel_error(" Error while creating the main thread!\n");

    }
    if(sched_create_kernel_thread(&thread_sem4, 4, "thread1", THREAD_TYPE_KERNEL, 0x1000, sem_thread_4, NULL) != OS_NO_ERR)
    {
        kernel_error(" Error while creating the main thread!\n");

    }
    if(sched_create_kernel_thread(&thread_sem5, 5, "thread1", THREAD_TYPE_KERNEL, 0x1000, sem_thread_5, NULL) != OS_NO_ERR)
    {
        kernel_error(" Error while creating the main thread!\n");

    }

    if(sem_pend(&sem_end) != OS_NO_ERR)
    {
        kernel_error("Failed to pend sem_end\n");

    }

    if(sem_destroy(&sem1) != OS_NO_ERR)
    {
        kernel_error("Failed to destroy sem1\n");

    }
    if(sem_destroy(&sem2) != OS_NO_ERR)
    {
        kernel_error("Failed to destroy sem2\n");

    }
    if(sem_destroy(&sem3) != OS_NO_ERR)
    {
        kernel_error("Failed to destroy sem3\n");

    }
    if(sem_destroy(&sem4) != OS_NO_ERR)
    {
        kernel_error("Failed to destroy sem4\n");

    }
    if(sem_destroy(&sem_end) != OS_NO_ERR)
    {
        kernel_error("Failed to destroy sem_end\n");

    }

    if((err = sched_join_thread(thread_sem1, NULL, NULL)) != OS_NO_ERR)
    {
        kernel_error("Error while waiting thread! [%d]\n", err);

    }
    if(sched_join_thread(thread_sem2, NULL, NULL) != OS_NO_ERR)
    {
        kernel_error("Error while waiting thread! [%d]\n", err);

    }
    if(sched_join_thread(thread_sem3, NULL, NULL) != OS_NO_ERR)
    {
        kernel_error("Error while waiting thread! [%d]\n", err);

    }
    if(sched_join_thread(thread_sem4, NULL, NULL) != OS_NO_ERR)
    {
        kernel_error("Error while waiting thread! [%d]\n", err);

    }
     if(sched_join_thread(thread_sem5, NULL, NULL) != OS_NO_ERR)
    {
        kernel_error("Error while waiting thread! [%d]\n", err);

    }

    kernel_printf("\n");

    if(lock_res != 9)
    {
        kernel_error("Test failed\n");
    }
    else
    {
        kernel_printf("[TESTMODE] Semaphore test passed\n");
    }

    /* Kill QEMU */
    kill_qemu();
}
#else
void semaphore_test(void)
{

}
#endif