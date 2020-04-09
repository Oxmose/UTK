#include <core/scheduler.h>
#include <sync/mutex.h>
#include <io/kernel_output.h>
#include <lib/stdio.h>
#include <Tests/test_bank.h>
#include <cpu.h>

#if MUTEX_TEST == 1
static thread_t thread_mutex1;
static thread_t thread_mutex2;
static thread_t thread_mutex3;

static mutex_t mutex1;
static mutex_t mutex2;

static volatile uint32_t lock_res;

void* test_inherit(void*args)
{
    int arg = (int)args;

    if(arg == 1)
    {
        printf("[TESTMODE]T1 sched_sleep\n");
        sched_sleep(500);
        printf("[TESTMODE]T1 Wake\n");

        for(volatile uint32_t i = 0; i < 1000000; ++i);

        printf("[TESTMODE]T1 Bye\n");
    }
    else if(arg == 2)
    {
        printf("[TESTMODE]T2 Wake\n");

        for(volatile uint32_t i = 0; i < 10000000; ++i);

        printf("[TESTMODE]T2 sched_sleep\n");
        sched_sleep(700);
        printf("[TESTMODE]T2 Wake\n");

        for(volatile uint32_t i = 0; i < 100000000; ++i);

        printf("[TESTMODE]T2 Bye\n");
    }
    else
    {
        printf("[TESTMODE]T3 Wake\n");
        if(mutex_pend(&mutex1))
        {
            printf("[TESTMODE]Failed to pend mutex1 1\n");
            return (void*)0;
        }

        for(volatile uint32_t i = 0; i < 100000000; ++i);

        printf("[TESTMODE]T3 End work\n");
        if(mutex_post(&mutex1))
        {
            printf("[TESTMODE]Failed to post mutex1 1\n");
            return NULL;
        }
        printf("[TESTMODE]T3 Bye\n");

    }
    return NULL;
}

void* test_rec(void* args)
{
    (void)args;
    if(mutex_pend(&mutex1))
    {
        printf("[TESTMODE]Failed to pend mutex1 1\n");
        return (void*)0;
    }

    if((int)args == 0)
    {
        if(mutex_pend(&mutex1) != OS_ERR_MUTEX_UNINITIALIZED)
        {
            printf("[TESTMODE]Failed to pend mutex1 1\n");
            return (void*)1;
        }
    }
    else
    {
        if(mutex_pend(&mutex1) != OS_NO_ERR)
        {
            printf("[TESTMODE]Failed to pend mutex1 1\n");
            return (void*)1;
        }
    }
    printf("\n[TESTMODE] (T R END) ");
    return (void*)0;
}


void *mutex_thread_1(void *args)
{
    for(int i = 0; i < 100000; ++i)
    {
        if(mutex_pend(&mutex1))
        {
            printf("[TESTMODE]Failed to pend mutex1 1\n");
            return NULL;
        }

        uint32_t tmp = lock_res;
        for(volatile uint32_t k = 0; k < 100; ++k);

        lock_res = tmp + 1;

        //if(i % 500 == 0)
        //    printf("[TESTMODE]T1\n");

        if(mutex_post(&mutex1))
        {
            printf("[TESTMODE]Failed to post mutex1 1\n");
            return NULL;
        }
    }
    printf(" (T1 END) ");
    (void )args;
    return NULL;
}
void *mutex_thread_2(void *args)
{
    for(int i = 0; i < 100000; ++i)
    {
        if(mutex_pend(&mutex1))
        {
            printf("[TESTMODE]Failed to pend mutex1 2\n");
            return NULL;
        }

        uint32_t tmp = lock_res;
        for(volatile uint32_t k = 0; k < 100; ++k);

        lock_res = tmp + 1;

        //if(i % 500 == 0)
        //    printf("[TESTMODE]T2\n");

        if(mutex_post(&mutex1))
        {
            printf("[TESTMODE]Failed to post mutex1 2\n");
            return NULL;
        }
    }
    printf(" (T2 END) ");
    (void )args;
    return NULL;
}

void *mutex_thread_3(void *args)
{
    //printf("[TESTMODE]T3\n");
    int32_t val;
    OS_RETURN_E err;
    if((err = mutex_try_pend(&mutex2, &val)) != OS_MUTEX_LOCKED || val != 0)
    {
        printf("[TESTMODE]Failed to trypend mutex2 3, val %d | %d\n", val, err);
        perror(err);
        return NULL;
    }
    if(mutex_pend(&mutex2) != OS_ERR_MUTEX_UNINITIALIZED)
    {
        printf("[TESTMODE]Failed to pend mutex2 3\n");
        return NULL;
    }
    else
    {
        printf("\n[TESTMODE] (T3 END) ");
        return NULL;
    }

    if(mutex_post(&mutex2))
    {
        printf("[TESTMODE]Failed to post mutex2 3\n");
        return NULL;
    }
    printf("\n[TESTMODE] (T3 END) ");
    (void )args;
    return NULL;
}

void mutex_test(void)
{
    OS_RETURN_E err;
    /* DIABLED PRIORITY INHERITANCE PROTOCOL */
    err = mutex_init(&mutex1, MUTEX_FLAG_NONE, MUTEX_PRIORITY_ELEVATION_NONE);
    if(err != OS_NO_ERR)
    {
        printf("[TESTMODE]Failed to init mutex1, %d\n", err);
        ;
    }
    if(mutex_init(&mutex2, MUTEX_FLAG_NONE, MUTEX_PRIORITY_ELEVATION_NONE) != OS_NO_ERR)
    {
        printf("[TESTMODE]Failed to init mutex2\n");
        return;
    }

    if(mutex_pend(&mutex2))
    {
        printf("[TESTMODE]Failed to pend mutex2\n");
        return;
    }

    lock_res = 0;

    if(sched_create_kernel_thread(&thread_mutex1, 1, "thread1", 1024, 0, mutex_thread_1, NULL) != OS_NO_ERR)
    {
        kernel_error(" Error while creating the main thread!\n");
        return;
    }
    if(sched_create_kernel_thread(&thread_mutex2, 1, "thread1", 1024, 0, mutex_thread_2, NULL) != OS_NO_ERR)
    {
        kernel_error(" Error while creating the main thread!\n");
        return;
    }
    if(sched_create_kernel_thread(&thread_mutex3, 1, "thread1", 1024, 0, mutex_thread_3, NULL) != OS_NO_ERR)
    {
        kernel_error(" Error while creating the main thread!\n");
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

    /* Test non recursive mutex */
    int ret;
    if((sched_create_kernel_thread(&thread_mutex1, 1, "thread1", 1024, 0, test_rec, (void*)0)) != OS_NO_ERR)
    {
        kernel_error(" Error while creating the main thread!\n");
        return;
    }

    sched_sleep(100);

    if((err = mutex_destroy(&mutex1)) != OS_NO_ERR)
    {
        kernel_error("Failed to destroy mutex1 %d\n", err);
        return;
    }

    if((err = sched_wait_thread(thread_mutex1, (void*)&ret, NULL)) != OS_NO_ERR)
    {
        kernel_error("Error while waiting thread! [%d]\n", err);
        return;
    }
    if(ret != 0)
    {
        return;
    }

    /* Test recursive mutex */
    if(mutex_init(&mutex1, MUTEX_FLAG_RECURSIVE, MUTEX_PRIORITY_ELEVATION_NONE) != OS_NO_ERR)
    {
        printf("[TESTMODE]Failed to init mutex1\n");
        return;
    }

    if((sched_create_kernel_thread(&thread_mutex1, 1, "thread1", 1024, 0, test_rec, (void*)1)) != OS_NO_ERR)
    {
        kernel_error(" Error while creating the main thread!\n");
        return;
    }


    if((err = sched_wait_thread(thread_mutex1, (void*)&ret, NULL)) != OS_NO_ERR)
    {
        kernel_error("Error while waiting thread! [%d]\n", err);
        return;
    }
    if(ret != 0)
    {
        return;
    }

    if((err = mutex_destroy(&mutex1)) != OS_NO_ERR)
    {
        kernel_error("Failed to destroy mutex1 %d\n", err);
        return;
    }

    if(mutex_destroy(&mutex2) != OS_NO_ERR)
    {
        kernel_error("Failed to destroy mutex2\n");
        return;
    }

    if(sched_wait_thread(thread_mutex3, NULL, NULL) != OS_NO_ERR)
    {
        printf("[TESTMODE]Error while waiting thread! [%d]\n", err);
        return;
    }

    /* ENABLE PRIORITY INHERITANCE PROTOCOL */
    if(mutex_init(&mutex1, MUTEX_FLAG_RECURSIVE, 5) != OS_NO_ERR)
    {
        printf("[TESTMODE]Failed to init mutex1\n");
        return;
    }

    printf("[TESTMODE]\n");

    if((sched_create_kernel_thread(&thread_mutex1, 1, "thread1", 1024, 0, test_inherit, (void*)1)) != OS_NO_ERR)
    {
        kernel_error(" Error while creating the main thread!\n");
        return;
    }

    if((sched_create_kernel_thread(&thread_mutex2, 5, "thread1", 1024, 0, test_inherit, (void*)2)) != OS_NO_ERR)
    {
        kernel_error(" Error while creating the main thread!\n");
        return;
    }

    if((sched_create_kernel_thread(&thread_mutex3, 10, "thread1", 1024, 0, test_inherit, (void*)3)) != OS_NO_ERR)
    {
        kernel_error(" Error while creating the main thread!\n");
        return;
    }

    if(sched_wait_thread(thread_mutex1, NULL, NULL) != OS_NO_ERR)
    {
        printf("[TESTMODE]Error while waiting thread! [%d]\n", err);
        return;
    }

    if(sched_wait_thread(thread_mutex2, NULL, NULL) != OS_NO_ERR)
    {
        printf("[TESTMODE]Error while waiting thread! [%d]\n", err);
        return;
    }

    if(sched_wait_thread(thread_mutex3, NULL, NULL) != OS_NO_ERR)
    {
        printf("[TESTMODE]Error while waiting thread! [%d]\n", err);
        return;
    }

    /* Test recusive mutex */

    printf("[TESTMODE]Lock res = %u\n", lock_res);
    if(lock_res == 200000)
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
void mutex_test(void)
{
}
#endif