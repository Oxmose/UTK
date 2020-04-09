#include <Tests/test_bank.h>
#include <core/kernel_queue.h>
#include <io/kernel_output.h>
#include <core/panic.h>
#include <lib/stddef.h>
#include <cpu.h>

#if KERNEL_QUEUE_TEST == 1
void kernel_queue_test(void)
{
    OS_RETURN_E error = OS_ERR_NULL_POINTER;
    kernel_queue_node_t* nodes[40] = { NULL };
    kernel_queue_t*      queue = NULL;
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
    nodes[0] = kernel_queue_create_node((void*) 0, &error);
    if(nodes[0] == NULL || error != OS_NO_ERR)
    {
        kernel_error("TEST_KQUEUE 0\n");
    }
    else 
    {
        kernel_printf("[TESTMODE] Kernel Queue %d passed.\n", test_count++);
    }   

    /* Delete node */
    error = kernel_queue_delete_node(&nodes[0]);
    if(nodes[0] != NULL || error != OS_NO_ERR)
    {
        kernel_error("TEST_KQUEUE 1\n");
    }
    else 
    {
        kernel_printf("[TESTMODE] Kernel Queue %d passed.\n", test_count++);
    } 

    error = OS_ERR_NULL_POINTER;

    /* Create node */
    nodes[0] = kernel_queue_create_node((void*) 0, &error);
    if(nodes[0] == NULL || error != OS_NO_ERR)
    {
        kernel_error("TEST_KQUEUE 2\n");
    }
    else 
    {
        kernel_printf("[TESTMODE] Kernel Queue %d passed.\n", test_count++);
    } 

    error = OS_ERR_NULL_POINTER;

    /* Create queue */
    queue = kernel_queue_create_queue(&error);
    if(queue == NULL || error != OS_NO_ERR)
    {
        kernel_error("TEST_KQUEUE 3\n");
    }
    else 
    {
        kernel_printf("[TESTMODE] Kernel Queue %d passed.\n", test_count++);
    } 

    /* Delete queue */
    error = kernel_queue_delete_queue(&queue);
    if(queue != NULL || error != OS_NO_ERR)
    {
        kernel_error("TEST_KQUEUE 4\n");
    }
    else 
    {
        kernel_printf("[TESTMODE] Kernel Queue %d passed.\n", test_count++);
    } 

    error = OS_ERR_NULL_POINTER;

    /* Create queue */
    queue = kernel_queue_create_queue(&error);
    if(queue == NULL || error != OS_NO_ERR)
    {
        kernel_error("TEST_KQUEUE 5\n");
    }
    else 
    {
        kernel_printf("[TESTMODE] Kernel Queue %d passed.\n", test_count++);
    } 

    /* Enqueue node */
    error = kernel_queue_push(nodes[0], queue);
    if(error != OS_NO_ERR)
    {
        kernel_error("TEST_KQUEUE 6\n");
    }
    else 
    {
        kernel_printf("[TESTMODE] Kernel Queue %d passed.\n", test_count++);
    } 

    /* Delete node */
    error = kernel_queue_delete_node(&nodes[0]);
    if(nodes[0] == NULL || error != OS_ERR_UNAUTHORIZED_ACTION)
    {
        kernel_error("TEST_KQUEUE 7 %d %d\n", nodes[0], error);
    }
    else 
    {
        kernel_printf("[TESTMODE] Kernel Queue %d passed.\n", test_count++);
    } 

    /* Enqueue NULL node */
    error = kernel_queue_push(NULL, queue);
    if(error != OS_ERR_NULL_POINTER)
    {
        kernel_error("TEST_KQUEUE 8\n");
    }
    else 
    {
        kernel_printf("[TESTMODE] Kernel Queue %d passed.\n", test_count++);
    } 

    /* Delete queue */
    error = kernel_queue_delete_queue(&queue);
    if(queue == NULL || error != OS_ERR_UNAUTHORIZED_ACTION)
    {
        kernel_error("TEST_KQUEUE 9\n");
    }
    else 
    {
        kernel_printf("[TESTMODE] Kernel Queue %d passed.\n", test_count++);
    } 

    error = OS_ERR_NULL_POINTER;

    /* Dequeue node */
    nodes[0] = kernel_queue_pop(queue, &error);
    if(nodes[0] == NULL || error != OS_NO_ERR)
    {
        kernel_error("TEST_KQUEUE 10\n");
    }
    else 
    {
        kernel_printf("[TESTMODE] Kernel Queue %d passed.\n", test_count++);
    } 

    error = OS_ERR_NULL_POINTER;

    /* Create more nodes */
    for(uint8_t i = 0; i < 40; ++i)
    {
        nodes[i] = kernel_queue_create_node((void*) unsorted[i % 10], &error);
        if(nodes[i] == NULL || error != OS_NO_ERR)
        {
            kernel_error("TEST_KQUEUE 11\n");
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
        error = kernel_queue_push_prio(nodes[i], queue, (uint32_t)nodes[i]->data);
        if(error != OS_NO_ERR)
        {
            kernel_error("TEST_KQUEUE 12\n");
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
        nodes[i] = kernel_queue_pop(queue, &error);
        if(nodes[i] == NULL || error != OS_NO_ERR)
        {
            kernel_error("TEST_KQUEUE 14\n");
        }
        else 
        {
            kernel_printf("[TESTMODE] Kernel Queue %d passed.\n", test_count);
        } 
        if((uint32_t)nodes[i]->data != sorted[i])
        {
            kernel_error("TEST_KQUEUE 15\n");
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
    }
    else 
    {
        kernel_printf("[TESTMODE] Kernel Queue %d passed.\n", test_count++);
    } 

    /* Delete nodes */
    for(uint8_t i = 0; i < 40; ++i)
    {
        error = kernel_queue_delete_node(&nodes[i]);
        if(nodes[i] != NULL || error != OS_NO_ERR)
        {
            kernel_error("TEST_KQUEUE 17\n");
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
        nodes[i] = kernel_queue_create_node((void*) unsorted[i % 10], &error);
        if(nodes[i] == NULL || error != OS_NO_ERR)
        {
            kernel_error("TEST_KQUEUE 18\n");
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
        error = kernel_queue_push(nodes[i], queue);
        if(error != OS_NO_ERR)
        {
            kernel_error("TEST_KQUEUE 19\n");
        }
        else 
        {
            kernel_printf("[TESTMODE] Kernel Queue %d passed.\n", test_count);
        } 
    }
    ++test_count;


    error = OS_ERR_NULL_POINTER;

    /* Find a present node */
    kernel_queue_node_t* find;
    find =  kernel_queue_find(queue, (void*) 9, &error);
    if(find == NULL || error != OS_NO_ERR || find->data != (void*) 9)
    {
        kernel_error("TEST_KQUEUE 20\n");
    }
    else 
    {
        kernel_printf("[TESTMODE] Kernel Queue %d passed.\n", test_count++);
    } 

    error = OS_ERR_NULL_POINTER;

    /* Find a not present node */
    find =  kernel_queue_find(queue, (void*) 42, &error);
    if(find != NULL || error != OS_ERR_NO_SUCH_ID)
    {
        kernel_error("TEST_KQUEUE 21\n");
    }
    else 
    {
        kernel_printf("[TESTMODE] Kernel Queue %d passed.\n", test_count++);
    } 

    error = OS_ERR_NULL_POINTER;

    /* Dequeue nodes and check "non order" */
    for(uint8_t i = 0; i < 40; ++i)
    {
        nodes[i] = kernel_queue_pop(queue, &error);
        if(nodes[i] == NULL || error != OS_NO_ERR)
        {
            kernel_error("TEST_KQUEUE 22\n");
        }
        else 
        {
            kernel_printf("[TESTMODE] Kernel Queue %d passed.\n", test_count);
        } 
        if((uint32_t)nodes[i]->data != unsorted[i%10])
        {

            kernel_error("TEST_KQUEUE 23 %d %d %d\n", (uint32_t)nodes[i]->data, unsorted[i%10], i);
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
    }
    else 
    {
        kernel_printf("[TESTMODE] Kernel Queue %d passed.\n", test_count++);
    } 

    /* Dequeue node on empty queue */
    find = kernel_queue_pop(queue, &error);
    if(find != NULL || error != OS_NO_ERR)
    {
        kernel_error("TEST_KQUEUE 25\n");
    }
    else 
    {
        kernel_printf("[TESTMODE] Kernel Queue %d passed.\n", test_count++);
    } 

    /* Delete queue */
    error = kernel_queue_delete_queue(&queue);
    if(queue != NULL || error != OS_NO_ERR)
    {
        kernel_error("TEST_KQUEUE 26\n");
    }
    else 
    {
        kernel_printf("[TESTMODE] Kernel Queue %d passed.\n", test_count++);
    } 

    /* Enqueue node on NULL queue */
    error = kernel_queue_push(nodes[0], queue);
    if(error != OS_ERR_NULL_POINTER)
    {
        kernel_error("TEST_KQUEUE 27\n");
    }
    else 
    {
        kernel_printf("[TESTMODE] Kernel Queue %d passed.\n", test_count++);
    } 

    /* Dequeue node on NULL queue */
    find = kernel_queue_pop(queue, &error);
    if(find != NULL || error != OS_ERR_NULL_POINTER)
    {
        kernel_error("TEST_KQUEUE 28\n");
    }
    else 
    {
        kernel_printf("[TESTMODE] Kernel Queue %d passed.\n", test_count++);
    } 

    /* Delete nodes */
    for(uint8_t i = 0; i < 40; ++i)
    {
        error = kernel_queue_delete_node(&nodes[i]);
        if(nodes[i] != NULL || error != OS_NO_ERR)
        {
            kernel_error("TEST_KQUEUE 29\n");
        }
        else 
        {
            kernel_printf("[TESTMODE] Kernel Queue %d passed.\n", test_count);
        } 
    }

    kernel_printf("[TESTMODE] Kernel queues tests passed\n");

    /* Kill QEMU */
    cpu_outw(0x2000, 0x604);    
    while(1)
    {
        __asm__ ("hlt");
    }
}
#else 
void kernel_queue_test(void)
{

}
#endif