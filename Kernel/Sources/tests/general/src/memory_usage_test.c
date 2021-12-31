#include <test_bank.h>

#if MEMORY_USAGE_TEST  == 1
#include <kernel_output.h>
#include <stdlib.h>
#include <sys/process.h>
#include <memmgt.h>
#include <kheap.h>

void* throutine(void* args);
void* exit_routine(void* args);

void* throutine(void* args)
{
    (void)args;
    sched_sleep(500);
    return NULL;
}

void* exit_routine(void* args)
{
    (void)args;
    sched_sleep(500);
    exit(0);
    return NULL;
}

void memory_usage_test(void)
{
    int32_t     pid;
    int32_t     status;
    THREAD_TERMINATE_CAUSE_E     th_term_cause;
    int32_t     term_cause;
    uint32_t    page_free;
    uint32_t    kpage_free;
    uint32_t    frame_free;
    uint32_t    kheap_free;

    uint32_t    new_page_free;
    uint32_t    new_kpage_free;
    uint32_t    new_frame_free;
    uint32_t    new_kheap_free;

    void*       ret_val;

    kernel_thread_t* thread;

    OS_RETURN_E err;

    kernel_printf("[TESTMODE] Getting free pages\n");
    page_free = memory_get_free_pages();
    kernel_printf("[TESTMODE] Getting free kpages\n");
    kpage_free = memory_get_free_kpages();
    kernel_printf("[TESTMODE] Getting free frames\n");
    frame_free = memory_get_free_frames();
    kernel_printf("[TESTMODE] Getting free heap\n");
    kheap_free = kheap_get_free();

    err = sched_create_kernel_thread(&thread,
                                       42,
                                       "Name Test",
                                       THREAD_TYPE_KERNEL,
                                       0x1000,
                                       throutine,
                                       NULL);
    if(err != OS_NO_ERR)
    {
        kernel_error("Could not create thread %d\n", err);
        kill_qemu();
    }

    err = sched_join_thread(thread, &ret_val, &th_term_cause);
    if(err != OS_NO_ERR)
    {
        kernel_error("Could not join thread %d\n", err);
        kill_qemu();
    }

    new_page_free = memory_get_free_pages();
    new_kpage_free = memory_get_free_kpages();
    new_frame_free = memory_get_free_frames();
    new_kheap_free = kheap_get_free();

    kernel_printf("[TESTMODE] Page (%d), KPage (%d), Frame (%d), KHeap (%d)\n",
        page_free - new_page_free,
        kpage_free - new_kpage_free,
        frame_free - new_frame_free,
       (kheap_free > new_kheap_free ? kheap_free - new_kheap_free : 0));

    pid = fork();
    if(pid < 0)
    {
        kernel_error("[TESTMODE] Could not fork\n");
        kill_qemu();
    }

    if(pid)
    {
        new_page_free = memory_get_free_pages();
        new_kpage_free = memory_get_free_kpages();
        new_frame_free = memory_get_free_frames();
        new_kheap_free = kheap_get_free();

        kernel_printf("[TESTMODE] Page (%d), KPage (%d), Frame (%d), KHeap (%d)\n",
        page_free - new_page_free,
        kpage_free - new_kpage_free,
        frame_free - new_frame_free,
       (kheap_free > new_kheap_free ? kheap_free - new_kheap_free : 0));

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

    sched_sleep(500);

    new_page_free = memory_get_free_pages();
    new_kpage_free = memory_get_free_kpages();
    new_frame_free = memory_get_free_frames();
    new_kheap_free = kheap_get_free();

    kernel_printf("[TESTMODE] Page (%d), KPage (%d), Frame (%d), KHeap (%d)\n",
        page_free - new_page_free,
        kpage_free - new_kpage_free,
        frame_free - new_frame_free,
       (kheap_free > new_kheap_free ? kheap_free - new_kheap_free : 0));

    pid = fork();
    if(pid < 0)
    {
        kernel_error("[TESTMODE] Could not fork\n");
        kill_qemu();
    }

    if(pid)
    {
        sched_sleep(500);

        new_page_free = memory_get_free_pages();
        new_kpage_free = memory_get_free_kpages();
        new_frame_free = memory_get_free_frames();
        new_kheap_free = kheap_get_free();

        kernel_printf("[TESTMODE] Page (%d), KPage (%d), Frame (%d), KHeap (%d)\n",
        page_free - new_page_free,
        kpage_free - new_kpage_free,
        frame_free - new_frame_free,
       (kheap_free > new_kheap_free ? kheap_free - new_kheap_free : 0));

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

        sched_sleep(500);

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

            exit(22);
        }
        else
        {
            sched_sleep(1000);
            /* Here we cant return, this should be replaced in the future by the
            * exit syscall */
            exit(666);
        }
    }

    sched_sleep(500);
    sched_sleep(500);
    sched_sleep(500);
    new_page_free = memory_get_free_pages();
    new_kpage_free = memory_get_free_kpages();
    new_frame_free = memory_get_free_frames();
    new_kheap_free = kheap_get_free();

    kernel_printf("[TESTMODE] Page (%d), KPage (%d), Frame (%d), KHeap (%d)\n",
        page_free - new_page_free,
        kpage_free - new_kpage_free,
        frame_free - new_frame_free,
       (kheap_free > new_kheap_free ? kheap_free - new_kheap_free : 0));


    pid = fork();
    if(pid < 0)
    {
        kernel_error("[TESTMODE] Could not fork\n");
        kill_qemu();
    }

    if(pid)
    {
        sched_sleep(500);

        new_page_free = memory_get_free_pages();
        new_kpage_free = memory_get_free_kpages();
        new_frame_free = memory_get_free_frames();
        new_kheap_free = kheap_get_free();

        kernel_printf("[TESTMODE] Page (%d), KPage (%d), Frame (%d), KHeap (%d)\n",
        page_free - new_page_free,
        kpage_free - new_kpage_free,
        frame_free - new_frame_free,
       (kheap_free > new_kheap_free ? kheap_free - new_kheap_free : 0));

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

        sched_sleep(500);

        sched_create_kernel_thread(&thread,
                                   1,
                                   "testth",
                                   THREAD_TYPE_KERNEL,
                                   0x1000,
                                   exit_routine,
                                   NULL);
        while(1)
        {
            sched_sleep(1000);
        }
    }

    sched_sleep(500);
    sched_sleep(500);
    sched_sleep(500);
    new_page_free = memory_get_free_pages();
    new_kpage_free = memory_get_free_kpages();
    new_frame_free = memory_get_free_frames();
    new_kheap_free = kheap_get_free();

    kernel_printf("[TESTMODE] Page (%d), KPage (%d), Frame (%d), KHeap (%d)\n",
        page_free - new_page_free,
        kpage_free - new_kpage_free,
        frame_free - new_frame_free,
       (kheap_free > new_kheap_free ? kheap_free - new_kheap_free : 0));

    kill_qemu();
}
#else
void memory_usage_test(void)
{
}
#endif