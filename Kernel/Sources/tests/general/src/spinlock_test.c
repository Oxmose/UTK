
#include <test_bank.h>


#if SPINLOCK_TEST == 1

#include <atomic.h>
#include <kernel_output.h>
#include <scheduler.h>
#include <interrupts.h>

static kernel_thread_t* thread_mutex1;
static kernel_thread_t* thread_mutex2;

static spinlock_t lock = SPINLOCK_INIT_VALUE;

static volatile uint32_t lock_res;


void* spin_thread_1(void *args)
{
    for(int i = 0; i < 2000000; ++i)
    {
        SPINLOCK_LOCK(lock);

        uint32_t tmp = lock_res;
        for(volatile uint32_t k = 0; k < 200; ++k);

        lock_res = tmp + 1;

        SPINLOCK_UNLOCK(lock);
    }

    (void )args;
    return NULL;
}

void spinlock_test(void)
{
    OS_RETURN_E err;

    lock_res = 0;

    kernel_printf("[TESTMODE] Spinlock test start\n");

    if(sched_create_kernel_thread(&thread_mutex1, 1, "thread1", THREAD_TYPE_KERNEL, 0x1000, spin_thread_1, NULL) != OS_NO_ERR)
    {
        kernel_error(" Error while creating the main 1 thread!\n");
    }
    if(sched_create_kernel_thread(&thread_mutex2, 1, "thread1", THREAD_TYPE_KERNEL, 0x1000, spin_thread_1, NULL) != OS_NO_ERR)
    {
        kernel_error(" Error while creating the main 2 thread!\n");
    }

    if((err = sched_join_thread(thread_mutex1, NULL, NULL)) != OS_NO_ERR)
    {
        kernel_error("Error while waiting thread! [%d]\n", err);
    }
    if(sched_join_thread(thread_mutex2, NULL, NULL) != OS_NO_ERR)
    {
        kernel_error("Error while waiting thread! [%d]\n", err);
    }

    kernel_printf("[TESTMODE]Lock res = %u\n", lock_res);
    if(lock_res == 4000000)
    {
        kernel_printf("[TESTMODE] Spinlock test passed.\n");
    }

    /* Kill QEMU */
    kill_qemu();
}
#else
void spinlock_test(void)
{
}
#endif