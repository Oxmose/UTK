/*******************************************************************************
 * @file kernel_queue.c
 *
 * @see kernel_queue.h
 *
 * @author Alexy Torres Aurora Dugo
 *
 * @date 09/03/2018
 *
 * @version 1.5
 *
 * @brief Kernel's queue structures.
 *
 * @details Kernel's queue structures. These queues are used by the kernel as
 * priority queue or regular queues. A kernel queue can virtually store every
 * type of data and is just a wrapper.
 *
 * @copyright Alexy Torres Aurora Dugo
 ******************************************************************************/

#include <lib/stddef.h>       /* Standard definitions */
#include <lib/stdint.h>       /* Generic int types */
#include <lib/string.h>       /* String manipulation */
#include <memory/kheap.h>     /* Kernel heap */
#include <core/panic.h>       /* Kernel panic */
#include <sync/critical.h>    /* Critical sections */
#include <io/kernel_output.h> /* Kernel output methods */

/* UTK configuration file */
#include <config.h>

/* Header file */
#include <core/kernel_queue.h>

/*******************************************************************************
 * GLOBAL VARIABLES
 ******************************************************************************/

/*******************************************************************************
 * FUNCTIONS
 ******************************************************************************/

kernel_queue_node_t* kernel_queue_create_node(void* data, OS_RETURN_E *error)
{
    kernel_queue_node_t* new_node;

    /* Create new node */
    new_node = kmalloc(sizeof(kernel_queue_node_t));

    if(new_node == NULL)
    {
        if(error != NULL)
        {
            *error = OS_ERR_MALLOC;
        }
        return NULL;
    }
     /* Init the structure */
    memset(new_node, 0, sizeof(kernel_queue_node_t));
    new_node->data = data;
    if(error != NULL)
    {
        *error = OS_NO_ERR;
    }
    return new_node;
}

OS_RETURN_E kernel_queue_delete_node(kernel_queue_node_t** node)
{
    if(node == NULL || *node == NULL)
    {
        return OS_ERR_NULL_POINTER;
    }

    /* Checkqueuechaining */
    if((*node)->enlisted != 0)
    {
        return OS_ERR_UNAUTHORIZED_ACTION;
    }

    kfree(*node);

    *node = NULL;

    return OS_NO_ERR;
}

kernel_queue_t* kernel_queue_create_queue(OS_RETURN_E *error)
{
    kernel_queue_t* newqueue;

    /* Create new node */
    newqueue = kmalloc(sizeof(kernel_queue_t));
    if(newqueue == NULL)
    {
        if(error != NULL)
        {
            *error = OS_ERR_MALLOC;
        }
        return NULL;
    }

    /* Init the structure */
    memset(newqueue, 0, sizeof(kernel_queue_t));

#if MAX_CPU_COUNT > 1
    INIT_SPINLOCK(&newqueue->lock);
#endif

    if(error != NULL)
    {
        *error = OS_NO_ERR;
    }

    return newqueue;
}

OS_RETURN_E kernel_queue_delete_queue(kernel_queue_t** queue)
{
    if(queue == NULL || *queue == NULL)
    {
        return OS_ERR_NULL_POINTER;
    }

    /* Check queue chaining */
    if((*queue)->head != NULL || (*queue)->tail != NULL)
    {
        return OS_ERR_UNAUTHORIZED_ACTION;
    }

    kfree(*queue);

    *queue = NULL;

    return OS_NO_ERR;
}

OS_RETURN_E kernel_queue_push(kernel_queue_node_t* node,
                              kernel_queue_t* queue)
{
    uint32_t word;

#if QUEUE_KERNEL_DEBUG == 1
    kernel_serial_debug("Enqueue 0x%p in queue 0x%p\n",
                        node,
                        queue);
#endif

    if(node == NULL || queue == NULL)
    {
        return OS_ERR_NULL_POINTER;
    }

#if MAX_CPU_COUNT > 1
    ENTER_CRITICAL(word, &queue->lock);
#else
    ENTER_CRITICAL(word);
#endif

    /* If this queue is empty */
    if(queue->head == NULL)
    {
        /* Set the first item */
        queue->head = node;
        queue->tail = node;
        node->next = NULL;
        node->prev = NULL;
    }
    else
    {
        /* Just put on the tail */
        node->next = queue->head;
        node->prev = NULL;
        queue->head->prev = node;
        queue->head = node;
    }

    ++queue->size;
    node->enlisted = 1;

#if QUEUE_KERNEL_DEBUG == 1
kernel_serial_debug("Enqueue kernel element 0x%p in queue 0x%p\n",
                    node, queue);
#endif

#if QUEUE_KERNEL_DEBUG == 1
kernel_serial_debug("Enqueue element 0x%p 0x%p 0x%p 0x%p 0x%p\n",
                    node->next, node->prev, node->enlisted, node->priority, node->data);
#endif

    if(node->next != NULL && node->prev != NULL && node->next == node->prev)
    {
#if MAX_CPU_COUNT > 1
        EXIT_CRITICAL(word, &queue->lock);
#else
        EXIT_CRITICAL(word);
#endif

        kernel_panic(OS_ERR_UNAUTHORIZED_ACTION);
    }

#if MAX_CPU_COUNT > 1
    EXIT_CRITICAL(word, &queue->lock);
#else
    EXIT_CRITICAL(word);
#endif

    return OS_NO_ERR;
}


OS_RETURN_E kernel_queue_push_prio(kernel_queue_node_t* node,
                                   kernel_queue_t* queue,
                                   const uint32_t priority)
{
    kernel_queue_node_t* cursor;
    uint32_t             word;

#if QUEUE_KERNEL_DEBUG == 1
    kernel_serial_debug("Enqueue 0x%p in queue 0x%p\n",
                        node,
                        queue);
#endif

    if(node == NULL || queue == NULL)
    {
#if QUEUE_KERNEL_DEBUG == 1
        kernel_serial_debug("Enqueue NULL\n");
#endif   
        return OS_ERR_NULL_POINTER;
    }

#if MAX_CPU_COUNT > 1
    ENTER_CRITICAL(word, &queue->lock);
#else
    ENTER_CRITICAL(word);
#endif

    node->priority = priority;

    /* If this queue is empty */
    if(queue->head == NULL)
    {
        /* Set the first item */
        queue->head = node;
        queue->tail = node;
        node->next = NULL;
        node->prev = NULL;
    }
    else
    {
        cursor = queue->head;
        while(cursor != NULL && cursor->priority > priority)
        {
            cursor = cursor->next;
        }

        if(cursor != NULL)
        {
            node->next = cursor;
            node->prev = cursor->prev;
            cursor->prev = node;
            if(node->prev != NULL)
            {
                node->prev->next = node;
            }
            else
            {
                queue->head = node;
            }
        }
        else
        {
            /* Just put on the tail */
            node->prev = queue->tail;
            node->next = NULL;
            queue->tail->next = node;
            queue->tail = node;
        }
    }
    ++queue->size;
    node->enlisted = 1;

#if QUEUE_KERNEL_DEBUG == 1
kernel_serial_debug("Enqueue kernel element 0x%p in queue 0x%p\n",
                    node, queue);
#endif

#if QUEUE_KERNEL_DEBUG == 1
kernel_serial_debug("Enqueue element 0x%p 0x%p 0x%p 0x%p 0x%p\n",
                    node->next, node->prev, node->enlisted, node->priority, node->data);
#endif

    if(node->next != NULL && node->prev != NULL && node->next == node->prev)
    {
#if MAX_CPU_COUNT > 1
        EXIT_CRITICAL(word, &queue->lock);
#else
        EXIT_CRITICAL(word);
#endif

        kernel_panic(OS_ERR_UNAUTHORIZED_ACTION);
    }

#if MAX_CPU_COUNT > 1
    EXIT_CRITICAL(word, &queue->lock);
#else
    EXIT_CRITICAL(word);
#endif
    return OS_NO_ERR;
}

kernel_queue_node_t* kernel_queue_pop(kernel_queue_t* queue,
                                      OS_RETURN_E* error)
{
    kernel_queue_node_t* node;
    uint32_t             word;

#if QUEUE_KERNEL_DEBUG == 1
    kernel_serial_debug("Dequeue kernel element in queue 0x%p\n",
                        queue);
#endif

    if(queue == NULL)
    {
        if(error != NULL)
        {
            *error = OS_ERR_NULL_POINTER;
        }
        return NULL;
    }

    if(error != NULL)
    {
        *error = OS_NO_ERR;
    }

#if MAX_CPU_COUNT > 1
    ENTER_CRITICAL(word, &queue->lock);
#else
    ENTER_CRITICAL(word);
#endif

    /* If this queue is empty */
    if(queue->head == NULL)
    {

#if MAX_CPU_COUNT > 1
        EXIT_CRITICAL(word, &queue->lock);
#else
        EXIT_CRITICAL(word);
#endif

        return NULL;
    }

    /* Dequeue the last item */
    node = queue->tail;

#if QUEUE_KERNEL_DEBUG == 1
kernel_serial_debug("Dequeue kernel element 0x%p in queue 0x%p\n",
                    node, queue);
#endif

#if QUEUE_KERNEL_DEBUG == 1
kernel_serial_debug("Element 0x%p 0x%p 0x%p 0x%p 0x%p\n",
                    node->next, node->prev, node->enlisted, node->priority, node->data);
#endif

    if(node->prev != NULL)
    {
        node->prev->next = NULL;
        queue->tail = node->prev;
    }
    else
    {
        queue->head = NULL;
        queue->tail = NULL;
    }

    --queue->size;

    node->next = NULL;
    node->prev = NULL;
    node->enlisted = 0;

    if(error != NULL)
    {
        *error = OS_NO_ERR;
    }

#if MAX_CPU_COUNT > 1
    EXIT_CRITICAL(word, &queue->lock);
#else
    EXIT_CRITICAL(word);
#endif

    return node;
}

kernel_queue_node_t* kernel_queue_find(kernel_queue_t* queue, void* data,
                                       OS_RETURN_E *error)
{
    kernel_queue_node_t* node;
    uint32_t             word;

#if QUEUE_KERNEL_DEBUG == 1
    kernel_serial_debug("Find kernel data 0x%p in queue 0x%p\n",
                        data,
                        queue);
#endif

    /* If this queue is empty */
    if(queue == NULL)
    {
        if(error != NULL)
        {
            *error = OS_ERR_NULL_POINTER;
        }
        return NULL;
    }

#if MAX_CPU_COUNT > 1
    ENTER_CRITICAL(word, &queue->lock);
#else
    ENTER_CRITICAL(word);
#endif

    /* Search for the data */
    node = queue->head;
    while(node != NULL && node->data != data)
    {
        node = node->next;
    }

#if MAX_CPU_COUNT > 1
    EXIT_CRITICAL(word, &queue->lock);
#else
    EXIT_CRITICAL(word);
#endif

    /* No such data */
    if(node == NULL)
    {
        if(error != NULL)
        {
            *error = OS_ERR_NO_SUCH_ID;
        }
        return NULL;
    }

    if(error != NULL)
    {
        *error = OS_NO_ERR;
    }

    return node;
}

OS_RETURN_E kernel_queue_remove(kernel_queue_t* queue,
                               kernel_queue_node_t* node)
{
    kernel_queue_node_t* cursor;
    uint32_t             word;

    if(queue == NULL || node == NULL)
    {
        return OS_ERR_NULL_POINTER;
    }

#if QUEUE_KERNEL_DEBUG == 1
    kernel_serial_debug("Remove node kernel node 0x%p in queue0x%p\n",
                        node,
                        queue);
#endif

#if MAX_CPU_COUNT > 1
    ENTER_CRITICAL(word, &queue->lock);
#else
    ENTER_CRITICAL(word);
#endif

    /* Search for node in the queue*/
    cursor = queue->head;
    while(cursor != NULL && cursor != node)
    {
        cursor = cursor->next;
    }

    if(cursor == NULL)
    {
#if MAX_CPU_COUNT > 1
        EXIT_CRITICAL(word, &queue->lock);
#else
        EXIT_CRITICAL(word);
#endif
        return OS_ERR_NO_SUCH_ID;
    }

    /* Manage link */
    if(cursor->prev != NULL && cursor->next != NULL)
    {
        cursor->prev->next = cursor->next;
        cursor->next->prev = cursor->prev;
    }
    else if(cursor->prev == NULL && cursor->next != NULL)
    {
        queue->head = cursor->next;
        cursor->next->prev = NULL;
    }
    else if(cursor->prev != NULL && cursor->next == NULL)
    {
        queue->tail = cursor->prev;
        cursor->prev->next = NULL;
    }
    else
    {
        queue->head = NULL;
        queue->tail = NULL;
    }

    node->next = NULL;
    node->prev = NULL;

    node->enlisted = 0;

#if MAX_CPU_COUNT > 1
    EXIT_CRITICAL(word, &queue->lock);
#else
    EXIT_CRITICAL(word);
#endif

    return OS_NO_ERR;
}