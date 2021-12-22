#include <test_bank.h>

#if KQUEUE_TEST  == 1
#include <kqueue.h>
#include <kernel_output.h>
#include <panic.h>
#include <stddef.h>
#include <kheap.h>

void kqueue_test(void)
{
    kqueue_node_t* nodes[40] = { NULL };
    kqueue_t*      queue = NULL;
    uint32_t   sorted[40];
    uint32_t   unsorted[10] = {0, 3, 5, 7, 4, 1, 8, 9, 6, 2};

    uint32_t test_count = 0;

    uint32_t old_size;

    int8_t j = -1;
    for(uint32_t i = 0; i < 40; ++i)
    {
        if(i % 4 == 0)
        {
            ++j;
        }
        sorted[i] = j;
    }

    /* Create node */
    nodes[0] = kqueue_create_node((void*) 0);
    if(nodes[0] == NULL)
    {
        kernel_error("TEST_KQUEUE 0\n");
        kill_qemu();
    }
    else
    {
        kernel_printf("[TESTMODE] Kernel Queue %d passed.\n", test_count++);
    }

    /* Delete node */
    kqueue_delete_node(&nodes[0]);
    if(nodes[0] != NULL)
    {
        kernel_error("TEST_KQUEUE 1\n");
        kill_qemu();
    }
    else
    {
        kernel_printf("[TESTMODE] Kernel Queue %d passed.\n", test_count++);
    }

    /* Create node */
    nodes[0] = kqueue_create_node((void*) 0);
    if(nodes[0] == NULL)
    {
        kernel_error("TEST_KQUEUE 2\n");
        kill_qemu();
    }
    else
    {
        kernel_printf("[TESTMODE] Kernel Queue %d passed.\n", test_count++);
    }

    /* Create queue */
    queue = kqueue_create_queue();
    if(queue == NULL)
    {
        kernel_error("TEST_KQUEUE 3\n");
        kill_qemu();
    }
    else
    {
        kernel_printf("[TESTMODE] Kernel Queue %d passed.\n", test_count++);
    }

    /* Delete queue */
    kqueue_delete_queue(&queue);
    if(queue != NULL)
    {
        kernel_error("TEST_KQUEUE 4\n");
        kill_qemu();
    }
    else
    {
        kernel_printf("[TESTMODE] Kernel Queue %d passed.\n", test_count++);
    }

    /* Create queue */
    queue = kqueue_create_queue();
    if(queue == NULL)
    {
        kernel_error("TEST_KQUEUE 5\n");
        kill_qemu();
    }
    else
    {
        kernel_printf("[TESTMODE] Kernel Queue %d passed.\n", test_count++);
    }

    /* Enqueue node */
    old_size = queue->size;
    kqueue_push(nodes[0], queue);
    if(old_size != queue->size - 1)
    {
        kernel_error("TEST_KQUEUE 6\n");
        kill_qemu();
    }
    else
    {
        kernel_printf("[TESTMODE] Kernel Queue %d passed.\n", test_count++);
    }

    /* Dequeue node */
    nodes[0] = kqueue_pop(queue);
    if(nodes[0] == NULL || old_size != queue->size)
    {
        kernel_error("TEST_KQUEUE 10\n");
        kill_qemu();
    }
    else
    {
        kernel_printf("[TESTMODE] Kernel Queue %d passed.\n", test_count++);
    }

    /* Create more nodes */
    for(uint8_t i = 0; i < 40; ++i)
    {
        nodes[i] = kqueue_create_node((void*) unsorted[i % 10]);
        if(nodes[i] == NULL)
        {
            kernel_error("TEST_KQUEUE 11\n");
            kill_qemu();
        }
        else
        {
            kernel_printf("[TESTMODE] Kernel Queue %d passed.\n", test_count);
        }

    }
    ++test_count;
    /* Enqueue nodes with prio */
    for(uint8_t i = 0; i < 40; ++i)
    {
        kqueue_push_prio(nodes[i], queue, (uintptr_t)nodes[i]->data);
        kernel_printf("[TESTMODE] Kernel Queue %d passed.\n", test_count);
    }
    ++test_count;

    /* Dequeue nodes and check order */
    for(uint8_t i = 0; i < 40; ++i)
    {
        nodes[i] = kqueue_pop(queue);
        if(nodes[i] == NULL)
        {
            kernel_error("TEST_KQUEUE 14\n");
            kill_qemu();
        }
        else
        {
            kernel_printf("[TESTMODE] Kernel Queue %d passed.\n", test_count);
        }
        if((uint32_t)nodes[i]->data != sorted[i])
        {
            kernel_error("TEST_KQUEUE 15\n");
            kill_qemu();
        }
        else
        {
            kernel_printf("[TESTMODE] Kernel Queue %d passed.\n", test_count+ 1);
        }
    }
    ++test_count;
    ++test_count;

    if(queue->size != 0)
    {
        kernel_error("TEST_KQUEUE 16\n");
        kill_qemu();
    }
    else
    {
        kernel_printf("[TESTMODE] Kernel Queue %d passed.\n", test_count++);
    }

    /* Delete nodes */
    for(uint8_t i = 0; i < 40; ++i)
    {
        kqueue_delete_node(&nodes[i]);
        if(nodes[i] != NULL)
        {
            kernel_error("TEST_KQUEUE 17\n");
            kill_qemu();
        }
        else
        {
            kernel_printf("[TESTMODE] Kernel Queue %d passed.\n", test_count);
        }
    }
    ++test_count;

    /* Create more nodes */
    for(uint8_t i = 0; i < 40; ++i)
    {
        nodes[i] = kqueue_create_node((void*) unsorted[i % 10]);
        if(nodes[i] == NULL)
        {
            kernel_error("TEST_KQUEUE 18\n");
            kill_qemu();
        }
        else
        {
            kernel_printf("[TESTMODE] Kernel Queue %d passed.\n", test_count);
        }
    }
    ++test_count;

    /* Enqueue without prio */
    for(uint8_t i = 0; i < 40; ++i)
    {
        kqueue_push(nodes[i], queue);
        kernel_printf("[TESTMODE] Kernel Queue %d passed.\n", test_count);
    }
    ++test_count;

    /* Find a present node */
    kqueue_node_t* find;
    find = kqueue_find(queue, (void*) 9);
    if(find == NULL || find->data != (void*) 9)
    {
        kernel_error("TEST_KQUEUE 20\n");
        kill_qemu();
    }
    else
    {
        kernel_printf("[TESTMODE] Kernel Queue %d passed.\n", test_count++);
    }

    /* Find a not present node */
    find = kqueue_find(queue, (void*) 42);
    if(find != NULL)
    {
        kernel_error("TEST_KQUEUE 21\n");
        kill_qemu();
    }
    else
    {
        kernel_printf("[TESTMODE] Kernel Queue %d passed.\n", test_count++);
    }

    /* Dequeue nodes and check "non order" */
    for(uint8_t i = 0; i < 40; ++i)
    {
        nodes[i] = kqueue_pop(queue);
        if(nodes[i] == NULL)
        {
            kernel_error("TEST_KQUEUE 22\n");
            kill_qemu();
        }
        else
        {
            kernel_printf("[TESTMODE] Kernel Queue %d passed.\n", test_count);
        }
        if((uint32_t)nodes[i]->data != unsorted[i%10])
        {

            kernel_error("TEST_KQUEUE 23 %d %d %d\n", (uint32_t)nodes[i]->data, unsorted[i%10], i);
            kill_qemu();
        }
        else
        {
            kernel_printf("[TESTMODE] Kernel Queue %d passed.\n", test_count + 1);
        }
    }
    ++test_count;
    ++test_count;
    if(queue->size != 0)
    {
        kernel_error("TEST_KQUEUE 24\n");
        kill_qemu();
    }
    else
    {
        kernel_printf("[TESTMODE] Kernel Queue %d passed.\n", test_count++);
    }

    /* Dequeue node on empty queue */
    find = kqueue_pop(queue);
    if(find != NULL)
    {
        kernel_error("TEST_KQUEUE 25\n");
        kill_qemu();
    }
    else
    {
        kernel_printf("[TESTMODE] Kernel Queue %d passed.\n", test_count++);
    }

    /* Delete queue */
    kqueue_delete_queue(&queue);
    if(queue != NULL)
    {
        kernel_error("TEST_KQUEUE 26\n");
        kill_qemu();
    }
    else
    {
        kernel_printf("[TESTMODE] Kernel Queue %d passed.\n", test_count++);
    }

    /* Delete nodes */
    for(uint8_t i = 0; i < 40; ++i)
    {
        kqueue_delete_node(&nodes[i]);
        if(nodes[i] != NULL)
        {
            kernel_error("TEST_KQUEUE 29\n");
            kill_qemu();
        }
        else
        {
            kernel_printf("[TESTMODE] Kernel Queue %d passed.\n", test_count);
        }
    }

    kernel_printf("[TESTMODE] Kernel queues tests passed\n");

    kill_qemu();
}
#else
void kqueue_test(void)
{

}
#endif