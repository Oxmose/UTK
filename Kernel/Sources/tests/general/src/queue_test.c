#include <test_bank.h>

#if QUEUE_TEST  == 1
#include <queue.h>
#include <kernel_output.h>
#include <panic.h>
#include <stddef.h>
#include <kheap.h>

void queue_test(void)
{
    OS_RETURN_E error = OS_ERR_NULL_POINTER;
    queue_node_t* nodes[40] = { NULL };
    queue_t*      queue = NULL;
    uint32_t   sorted[40];
    uint32_t   unsorted[10] = {0, 3, 5, 7, 4, 1, 8, 9, 6, 2};

    uint32_t test_count = 0;

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
    nodes[0] = queue_create_node((void*) 0, QUEUE_ALLOCATOR(kmalloc, kfree), &error);
    if(nodes[0] == NULL || error != OS_NO_ERR)
    {
        kernel_error("TEST_KQUEUE 0\n");
        kernel_panic(error);
    }
    else 
    {
        kernel_printf("[TESTMODE] Kernel Queue %d passed.\n", test_count++);
    }   

    /* Delete node */
    error = queue_delete_node(&nodes[0]);
    if(nodes[0] != NULL || error != OS_NO_ERR)
    {
        kernel_error("TEST_KQUEUE 1\n");
        kernel_panic(error);
    }
    else 
    {
        kernel_printf("[TESTMODE] Kernel Queue %d passed.\n", test_count++);
    } 

    error = OS_ERR_NULL_POINTER;

    /* Create node */
    nodes[0] = queue_create_node((void*) 0, QUEUE_ALLOCATOR(kmalloc, kfree), &error);
    if(nodes[0] == NULL || error != OS_NO_ERR)
    {
        kernel_error("TEST_KQUEUE 2\n");
        kernel_panic(error);
    }
    else 
    {
        kernel_printf("[TESTMODE] Kernel Queue %d passed.\n", test_count++);
    } 

    error = OS_ERR_NULL_POINTER;

    /* Create queue */
    queue = queue_create_queue(QUEUE_ALLOCATOR(kmalloc, kfree), &error);
    if(queue == NULL || error != OS_NO_ERR)
    {
        kernel_error("TEST_KQUEUE 3\n");
        kernel_panic(error);
    }
    else 
    {
        kernel_printf("[TESTMODE] Kernel Queue %d passed.\n", test_count++);
    } 

    /* Delete queue */
    error = queue_delete_queue(&queue);
    if(queue != NULL || error != OS_NO_ERR)
    {
        kernel_error("TEST_KQUEUE 4\n");
        kernel_panic(error);
    }
    else 
    {
        kernel_printf("[TESTMODE] Kernel Queue %d passed.\n", test_count++);
    } 

    error = OS_ERR_NULL_POINTER;

    /* Create queue */
    queue = queue_create_queue(QUEUE_ALLOCATOR(kmalloc, kfree),&error);
    if(queue == NULL || error != OS_NO_ERR)
    {
        kernel_error("TEST_KQUEUE 5\n");
        kernel_panic(error);
    }
    else 
    {
        kernel_printf("[TESTMODE] Kernel Queue %d passed.\n", test_count++);
    } 

    /* Enqueue node */
    error = queue_push(nodes[0], queue);
    if(error != OS_NO_ERR)
    {
        kernel_error("TEST_KQUEUE 6\n");
        kernel_panic(error);
    }
    else 
    {
        kernel_printf("[TESTMODE] Kernel Queue %d passed.\n", test_count++);
    } 

    /* Delete node */
    error = queue_delete_node(&nodes[0]);
    if(nodes[0] == NULL || error != OS_ERR_UNAUTHORIZED_ACTION)
    {
        kernel_error("TEST_KQUEUE 7 %d %d\n", nodes[0], error);
        kernel_panic(error);
    }
    else 
    {
        kernel_printf("[TESTMODE] Kernel Queue %d passed.\n", test_count++);
    } 

    /* Enqueue NULL node */
    error = queue_push(NULL, queue);
    if(error != OS_ERR_NULL_POINTER)
    {
        kernel_error("TEST_KQUEUE 8\n");
        kernel_panic(error);
    }
    else 
    {
        kernel_printf("[TESTMODE] Kernel Queue %d passed.\n", test_count++);
    } 

    /* Delete queue */
    error = queue_delete_queue(&queue);
    if(queue == NULL || error != OS_ERR_UNAUTHORIZED_ACTION)
    {
        kernel_error("TEST_KQUEUE 9\n");
        kernel_panic(error);
    }
    else 
    {
        kernel_printf("[TESTMODE] Kernel Queue %d passed.\n", test_count++);
    } 

    error = OS_ERR_NULL_POINTER;

    /* Dequeue node */
    nodes[0] = queue_pop(queue, &error);
    if(nodes[0] == NULL || error != OS_NO_ERR)
    {
        kernel_error("TEST_KQUEUE 10\n");
        kernel_panic(error);
    }
    else 
    {
        kernel_printf("[TESTMODE] Kernel Queue %d passed.\n", test_count++);
    } 

    error = OS_ERR_NULL_POINTER;

    /* Create more nodes */
    for(uint8_t i = 0; i < 40; ++i)
    {
        nodes[i] = queue_create_node((void*) unsorted[i % 10], QUEUE_ALLOCATOR(kmalloc, kfree), &error);
        if(nodes[i] == NULL || error != OS_NO_ERR)
        {
            kernel_error("TEST_KQUEUE 11\n");
            kernel_panic(error);
        }
        else 
        {
            kernel_printf("[TESTMODE] Kernel Queue %d passed.\n", test_count);
        } 

        error = OS_ERR_NULL_POINTER;
    }
    ++test_count;
    /* Enqueue nodes with prio */
    for(uint8_t i = 0; i < 40; ++i)
    {
        error = queue_push_prio(nodes[i], queue, (uintptr_t)nodes[i]->data);
        if(error != OS_NO_ERR)
        {
            kernel_error("TEST_KQUEUE 12\n");
            kernel_panic(error);
        }
        else 
        {
            kernel_printf("[TESTMODE] Kernel Queue %d passed.\n", test_count);
        } 
    }
    ++test_count;

    error = OS_ERR_NULL_POINTER;

    /* Dequeue nodes and check order */
    for(uint8_t i = 0; i < 40; ++i)
    {
        nodes[i] = queue_pop(queue, &error);
        if(nodes[i] == NULL || error != OS_NO_ERR)
        {
            kernel_error("TEST_KQUEUE 14\n");
            kernel_panic(error);
        }
        else 
        {
            kernel_printf("[TESTMODE] Kernel Queue %d passed.\n", test_count);
        } 
        if((uint32_t)nodes[i]->data != sorted[i])
        {
            kernel_error("TEST_KQUEUE 15\n");
            kernel_panic(error);
        }
        else 
        {
            kernel_printf("[TESTMODE] Kernel Queue %d passed.\n", test_count+ 1);
        } 


        error = OS_ERR_NULL_POINTER;
    }
    ++test_count;
    ++test_count;

    if(queue->size != 0)
    {
        kernel_error("TEST_KQUEUE 16\n");
        kernel_panic(OS_ERR_UNAUTHORIZED_ACTION);
    }
    else 
    {
        kernel_printf("[TESTMODE] Kernel Queue %d passed.\n", test_count++);
    } 

    /* Delete nodes */
    for(uint8_t i = 0; i < 40; ++i)
    {
        error = queue_delete_node(&nodes[i]);
        if(nodes[i] != NULL || error != OS_NO_ERR)
        {
            kernel_error("TEST_KQUEUE 17\n");
            kernel_panic(error);
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
        nodes[i] = queue_create_node((void*) unsorted[i % 10], QUEUE_ALLOCATOR(kmalloc, kfree), &error);
        if(nodes[i] == NULL || error != OS_NO_ERR)
        {
            kernel_error("TEST_KQUEUE 18\n");
            kernel_panic(error);
        }
        else 
        {
            kernel_printf("[TESTMODE] Kernel Queue %d passed.\n", test_count);
        } 

        error = OS_ERR_NULL_POINTER;
    }
    ++test_count;

    /* Enqueue without prio */
    for(uint8_t i = 0; i < 40; ++i)
    {
        error = queue_push(nodes[i], queue);
        if(error != OS_NO_ERR)
        {
            kernel_error("TEST_KQUEUE 19\n");
            kernel_panic(error);
        }
        else 
        {
            kernel_printf("[TESTMODE] Kernel Queue %d passed.\n", test_count);
        } 
    }
    ++test_count;


    error = OS_ERR_NULL_POINTER;

    /* Find a present node */
    queue_node_t* find;
    find =  queue_find(queue, (void*) 9, &error);
    if(find == NULL || error != OS_NO_ERR || find->data != (void*) 9)
    {
        kernel_error("TEST_KQUEUE 20\n");
        kernel_panic(error);
    }
    else 
    {
        kernel_printf("[TESTMODE] Kernel Queue %d passed.\n", test_count++);
    } 

    error = OS_ERR_NULL_POINTER;

    /* Find a not present node */
    find =  queue_find(queue, (void*) 42, &error);
    if(find != NULL || error != OS_ERR_NO_SUCH_ID)
    {
        kernel_error("TEST_KQUEUE 21\n");
        kernel_panic(error);
    }
    else 
    {
        kernel_printf("[TESTMODE] Kernel Queue %d passed.\n", test_count++);
    } 

    error = OS_ERR_NULL_POINTER;

    /* Dequeue nodes and check "non order" */
    for(uint8_t i = 0; i < 40; ++i)
    {
        nodes[i] = queue_pop(queue, &error);
        if(nodes[i] == NULL || error != OS_NO_ERR)
        {
            kernel_error("TEST_KQUEUE 22\n");
            kernel_panic(error);
        }
        else 
        {
            kernel_printf("[TESTMODE] Kernel Queue %d passed.\n", test_count);
        } 
        if((uint32_t)nodes[i]->data != unsorted[i%10])
        {

            kernel_error("TEST_KQUEUE 23 %d %d %d\n", (uint32_t)nodes[i]->data, unsorted[i%10], i);
            kernel_panic(error);
        }
        else 
        {
            kernel_printf("[TESTMODE] Kernel Queue %d passed.\n", test_count + 1);
        } 
        error = OS_ERR_NULL_POINTER;
    }
    ++test_count;
    ++test_count;
    if(queue->size != 0)
    {
        kernel_error("TEST_KQUEUE 24\n");
        kernel_panic(OS_ERR_UNAUTHORIZED_ACTION);
    }
    else 
    {
        kernel_printf("[TESTMODE] Kernel Queue %d passed.\n", test_count++);
    } 

    /* Dequeue node on empty queue */
    find = queue_pop(queue, &error);
    if(find != NULL || error != OS_NO_ERR)
    {
        kernel_error("TEST_KQUEUE 25\n");
        kernel_panic(error);
    }
    else 
    {
        kernel_printf("[TESTMODE] Kernel Queue %d passed.\n", test_count++);
    } 

    /* Delete queue */
    error = queue_delete_queue(&queue);
    if(queue != NULL || error != OS_NO_ERR)
    {
        kernel_error("TEST_KQUEUE 26\n");
        kernel_panic(error);
    }
    else 
    {
        kernel_printf("[TESTMODE] Kernel Queue %d passed.\n", test_count++);
    } 

    /* Enqueue node on NULL queue */
    error = queue_push(nodes[0], queue);
    if(error != OS_ERR_NULL_POINTER)
    {
        kernel_error("TEST_KQUEUE 27\n");
        kernel_panic(error);
    }
    else 
    {
        kernel_printf("[TESTMODE] Kernel Queue %d passed.\n", test_count++);
    } 

    /* Dequeue node on NULL queue */
    find = queue_pop(queue, &error);
    if(find != NULL || error != OS_ERR_NULL_POINTER)
    {
        kernel_error("TEST_KQUEUE 28\n");
        kernel_panic(error);
    }
    else 
    {
        kernel_printf("[TESTMODE] Kernel Queue %d passed.\n", test_count++);
    } 

    /* Delete nodes */
    for(uint8_t i = 0; i < 40; ++i)
    {
        error = queue_delete_node(&nodes[i]);
        if(nodes[i] != NULL || error != OS_NO_ERR)
        {
            kernel_error("TEST_KQUEUE 29\n");
            kernel_panic(error);
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
void queue_test(void)
{

}
#endif