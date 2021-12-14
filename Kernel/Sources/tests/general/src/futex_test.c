#include <test_bank.h>


#if FUTEX_TEST == 1

#include <kernel_output.h>
#include <scheduler.h>
#include <interrupts.h>
#include <time_management.h>
#include <futex.h>

uint32_t shared_data;
uint32_t shared_data2;

static void* print_th0(void*args)
{
    (void) args;

    futex_t params;

    uint64_t diff;

    
    params.addr  = &shared_data;
    params.error = OS_NO_ERR;
    params.val   = 4;

    kernel_printf("[TESTMODE] Thread 0 waits on futex\n");

    uint64_t i = time_get_current_uptime();
    futex_wait(SYSCALL_FUTEX_WAIT, (void*)&params);
    /* Check that we waited at least 2000ms */
    diff = time_get_current_uptime() - i;
    if(params.error != OS_NO_ERR || diff < 2000)
    {
        kernel_error("Futex was woken up too early\n");
        kill_qemu(); 
    }
    kernel_printf("[TESTMODE] Futex 1 passed, waited %d\n", diff);

    params.val   = 2;
    i = time_get_current_uptime();
    futex_wait(SYSCALL_FUTEX_WAIT, (void*)&params);
    /* Check that we waited at least 1000ms */
    diff = time_get_current_uptime() - i;
    if(params.error != OS_NO_ERR || diff < 1000)
    {
        kernel_error("Futex was woken up too early\n");
        kill_qemu(); 
    }
    kernel_printf("[TESTMODE] Futex 2 passed, waited %d\n", diff);

    return NULL;
}

static void* print_th1(void*args)
{
    (void) args;

    futex_t params;

    params.addr  = &shared_data2;
    params.error = OS_NO_ERR;
    params.val   = 4;

    futex_wait(SYSCALL_FUTEX_WAIT, (void*)&params);
    /* Check that we waited at least 2000ms */
    if(params.error != OS_NO_ERR)
    {
        kernel_error("Futex was woken up too early\n");
        kill_qemu(); 
    }
    else
    {
        kernel_printf("[TESTMODE] Futex th 1 - 0\n");
    }

    sched_sleep(200);

    shared_data2 = 4;
    futex_wake(SYSCALL_FUTEX_WAKE, (void*)&params);
    if(params.error != OS_NO_ERR)
    {
        kernel_error("Futex wake error\n");
        kill_qemu(); 
    }
    

    futex_wait(SYSCALL_FUTEX_WAIT, (void*)&params);
    /* Check that we waited at least 2000ms */
    if(params.error != OS_NO_ERR)
    {
        kernel_error("Futex was woken up too early\n");
        kill_qemu(); 
    }
    else
    {
        kernel_printf("[TESTMODE] Futex th 1 - 1\n");
    }

    sched_sleep(200);

    shared_data2 = 4;
    futex_wake(SYSCALL_FUTEX_WAKE, (void*)&params);
    if(params.error != OS_NO_ERR)
    {
        kernel_error("Futex wake error\n");
        kill_qemu(); 
    }

    futex_wait(SYSCALL_FUTEX_WAIT, (void*)&params);
    /* Check that we waited at least 2000ms */
    if(params.error != OS_NO_ERR)
    {
        kernel_error("Futex was woken up too early\n");
        kill_qemu(); 
    }
    else
    {
        kernel_printf("[TESTMODE] Futex th 1 - 2\n");
    }

    sched_sleep(200);

    shared_data2 = 4;
    futex_wake(SYSCALL_FUTEX_WAKE, (void*)&params);
    if(params.error != OS_NO_ERR)
    {
        kernel_error("Futex wake error\n");
        kill_qemu(); 
    }

    return NULL;
}

static void* print_th2(void*args)
{
    (void)args;
    futex_t params;

    params.addr  = &shared_data2;
    params.error = OS_NO_ERR;
    params.val   = 2;

    sched_sleep(1000);

    shared_data2 = 2;
    futex_wake(SYSCALL_FUTEX_WAKE, (void*)&params);
    if(params.error != OS_NO_ERR)
    {
        kernel_error("Futex wake error\n");
        kill_qemu(); 
    }

    futex_wait(SYSCALL_FUTEX_WAIT, (void*)&params);
    /* Check that we waited at least 2000ms */
    if(params.error != OS_NO_ERR)
    {
        kernel_error("Futex was woken up too early\n");
        kill_qemu(); 
    }
    else
    {
        kernel_printf("[TESTMODE] Futex th 2 - 0\n");
    }

    sched_sleep(1000);

    shared_data2 = 2;
    futex_wake(SYSCALL_FUTEX_WAKE, (void*)&params);
    if(params.error != OS_NO_ERR)
    {
        kernel_error("Futex wake error\n");
        kill_qemu(); 
    }

    futex_wait(SYSCALL_FUTEX_WAIT, (void*)&params);
    /* Check that we waited at least 2000ms */
    if(params.error != OS_NO_ERR)
    {
        kernel_error("Futex was woken up too early\n");
        kill_qemu(); 
    }
    else
    {
        kernel_printf("[TESTMODE] Futex th 2 - 1\n");
    }

    sched_sleep(1000);

    shared_data2 = 2;
    futex_wake(SYSCALL_FUTEX_WAKE, (void*)&params);
    if(params.error != OS_NO_ERR)
    {
        kernel_error("Futex wake error\n");
        kill_qemu(); 
    }

    futex_wait(SYSCALL_FUTEX_WAIT, (void*)&params);
    /* Check that we waited at least 2000ms */
    if(params.error != OS_NO_ERR)
    {
        kernel_error("Futex was woken up too early\n");
        kill_qemu(); 
    }
    else
    {
        kernel_printf("[TESTMODE] Futex th 2 - 2\n");
    }
    return NULL;
}

static uint32_t woke = 0;

static void* print_th3(void*args)
{
    (void)args;
    futex_t params;
    uint32_t int_value;

    params.addr  = &shared_data;
    params.error = OS_NO_ERR;
    params.val   = 2;

    futex_wait(SYSCALL_FUTEX_WAIT, (void*)&params);
    /* Check that we waited at least 2000ms */
    if(params.error != OS_NO_ERR)
    {
        kernel_error("Futex was woken up too early\n");
        kill_qemu(); 
    }
    else
    {
        ENTER_CRITICAL(int_value);
        kernel_printf("[TESTMODE] Futex th 3 - 0\n");
        EXIT_CRITICAL(int_value);
    }

    if(woke == 0)
    {
        shared_data = 2;
        woke = 1;

        futex_wait(SYSCALL_FUTEX_WAIT, (void*)&params);
        /* Check that we waited at least 2000ms */
        if(params.error != OS_NO_ERR)
        {
            kernel_error("Futex was woken up too early\n");
            kill_qemu(); 
        }
        else
        {
            ENTER_CRITICAL(int_value);
            kernel_printf("[TESTMODE] Futex th 3 - 0\n");
            EXIT_CRITICAL(int_value);
        }
    }

    return NULL;
}

void futex_test(void)
{
    kernel_thread_t* thread[3];
    OS_RETURN_E err;
    futex_t params;

    shared_data = 4;
    shared_data2 = 4;

    sched_sleep(200);

    kernel_printf("[TESTMODE] Futex tests starts\n");


    err = sched_create_kernel_thread(&thread[0], 0, "test",
                                    THREAD_TYPE_KERNEL, 0x1000, print_th0, (void*)0);
    if(err != OS_NO_ERR)
    {
        kernel_error("Cannot create threads %d\n", err);
        /* Kill QEMU */
        kill_qemu();    
    }

    sched_sleep(1100);

    shared_data = 4;

    params.addr  = &shared_data;
    params.error = OS_NO_ERR;
    params.val   = 1;

    futex_wake(SYSCALL_FUTEX_WAKE, (void*)&params);
    if(params.error != OS_NO_ERR)
    {
        kernel_error("Futex wake error %d\n", params.error);
        kill_qemu(); 
    }

    sched_sleep(1000);

    shared_data = 2;

    futex_wake(SYSCALL_FUTEX_WAKE, (void*)&params);
    if(params.error != OS_NO_ERR)
    {
        kernel_error("Futex wake error %d\n", params.error);
        kill_qemu(); 
    }

    sched_sleep(1100);

    shared_data = 4;

    futex_wake(SYSCALL_FUTEX_WAKE, (void*)&params);
    if(params.error != OS_NO_ERR)
    {
        kernel_error("Futex wake error\n");
        kill_qemu(); 
    }


    sched_join_thread(thread[0], NULL, NULL);

    kernel_printf("[TESTMODE] Futex tests 1 thread passed\n");
    

    err = sched_create_kernel_thread(&thread[1], 0, "test",
                                    THREAD_TYPE_KERNEL, 0x1000, print_th1, (void*)1);
    if(err != OS_NO_ERR)
    {
        kernel_error("Cannot create threads %d\n", err);
        /* Kill QEMU */
        kill_qemu();    
    }

    err = sched_create_kernel_thread(&thread[2], 0, "test",
                                    THREAD_TYPE_KERNEL, 0x1000, print_th2, (void*)2);
    if(err != OS_NO_ERR)
    {
        kernel_error("Cannot create threads %d\n", err);
        /* Kill QEMU */
        kill_qemu();    
    }

    sched_join_thread(thread[1], NULL, NULL);
    sched_join_thread(thread[2], NULL, NULL);

    kernel_printf("[TESTMODE] Futex tests passed 2\n");

    shared_data = 2;

    err = sched_create_kernel_thread(&thread[1], 0, "test",
                                    THREAD_TYPE_KERNEL, 0x1000, print_th3, (void*)1);
    if(err != OS_NO_ERR)
    {
        kernel_error("Cannot create threads %d\n", err);
        /* Kill QEMU */
        kill_qemu();    
    }

    err = sched_create_kernel_thread(&thread[2], 0, "test",
                                    THREAD_TYPE_KERNEL, 0x1000, print_th3, (void*)2);
    if(err != OS_NO_ERR)
    {
        kernel_error("Cannot create threads %d\n", err);
        /* Kill QEMU */
        kill_qemu();    
    }

    sched_sleep(1000);

    shared_data = 4;

    params.val = 1;

    futex_wake(SYSCALL_FUTEX_WAKE, (void*)&params);
    if(params.error != OS_NO_ERR)
    {
        kernel_error("Futex wake error\n");
        kill_qemu(); 
    }

    sched_sleep(2000);
    kernel_printf("[TESTMODE] Waking 2\n");

    params.val = 3;
    shared_data = 4;

    futex_wake(SYSCALL_FUTEX_WAKE, (void*)&params);
    if(params.error != OS_NO_ERR)
    {
        kernel_error("Futex wake error\n");
        kill_qemu(); 
    }

    sched_join_thread(thread[1], NULL, NULL);
    sched_join_thread(thread[2], NULL, NULL);

    kernel_interrupt_disable();

    kernel_printf("[TESTMODE] Futex tests passed\n");

    kill_qemu();  
}
#else
void futex_test(void)
{

}
#endif