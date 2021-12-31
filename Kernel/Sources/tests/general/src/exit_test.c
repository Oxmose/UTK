#include <test_bank.h>

#if EXIT_TEST  == 1
#include <kernel_output.h>
#include <stdlib.h>
#include <sys/process.h>
#include <scheduler.h>

void* new_th_routine(void* args);

void* new_th_routine(void* args)
{
    (void)args;

    sched_sleep(1000);
    exit(123);

    return NULL;
}

void exit_test(void)
{
    int32_t     pid;
    kernel_thread_t* thread;
    int32_t     status;
    int32_t     term_cause;
    OS_RETURN_E err;

    pid = fork();
    if(pid < 0)
    {
        kernel_error("[TESTMODE] Could not fork\n");
        kill_qemu();
    }

    if(pid)
    {
        pid = waitpid(pid, &status, &term_cause, &err);
        if(err != OS_NO_ERR)
        {
            kernel_error("[TESTMODE] Could not wait PID %d\n", err);
            kill_qemu();

        }
        kernel_printf("[TESTMODE] Process %d returned %d, %d\n", pid, status, err);
    }
    else
    {
        sched_sleep(1000);
        /* Here we cant return, this should be replaced in the future by the
         * exit syscall */
        exit(42);
    }

    kernel_printf("[TESTMODE] Testing exit in thread\n");

    pid = fork();
    if(pid < 0)
    {
        kernel_error("[TESTMODE] Could not fork\n");
        kill_qemu();
    }

    if(pid)
    {
        pid = waitpid(pid, &status, &term_cause, &err);
        if(err != OS_NO_ERR)
        {
            kernel_error("[TESTMODE] Could not wait PID %d\n", err);
            kill_qemu();

        }
        kernel_printf("[TESTMODE] Process %d returned %d, %d\n", pid, status, err);
    }
    else
    {
        sched_create_kernel_thread(&thread,
                                   1,
                                   "testth",
                                   THREAD_TYPE_KERNEL,
                                   0x1000,
                                   new_th_routine,
                                   NULL);
        while(1)
        {
            sched_sleep(1000);
        }
    }

    kernel_printf("[TESTMODE] Exit tests passed\n");
    kill_qemu();
}
#else
void exit_test(void)
{
}
#endif