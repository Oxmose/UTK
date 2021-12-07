#include <test_bank.h>

#if FORK_TEST  == 1
#include <kernel_output.h>
#include <sys/process.h>

void* fork_test(void)
{
    int32_t     pid;
    int32_t     status;
    int32_t     term_cause;
    OS_RETURN_E err;

    pid = fork();
    if(pid < 0)
    {
        kernel_error("[TESTMODE] Could not fork\n");
        kill_qemu();
    }

    kernel_printf("[TESTMODE] Forked\n");

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
        /* Here we cant return, this should be replaced in the future by the
         * exit syscall */
        sched_terminate_self((void*)42);
    }

    kill_qemu();

    return 0;
}
#else 
void* fork_test(void)
{
    return 0;
}
#endif