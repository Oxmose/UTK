/*******************************************************************************
 * @file queue.c
 *
 * @see queue.h
 *
 * @author Alexy Torres Aurora Dugo
 *
 * @date 03/03/2021
 *
 * @version 1.5
 *
 * @brief Queue structures.
 *
 * @details Queue structures. These queues are used as priority queue or regular 
 * queues. A queue can virtually store every type of data and is just a wrapper.
 * This queue library is thread safe.
 *
 * @copyright Alexy Torres Aurora Dugo
 ******************************************************************************/

#include <stddef.h>        /* Standard definitions */
#include <stdint.h>        /* Generic int types */
#include <string.h>        /* String manipulation */
#include <panic.h>         /* Kernel panic */
#include <critical.h>      /* Critical sections */
#include <kernel_output.h> /* Kernel output methods */

/* UTK configuration file */
#include <config.h>

/* Header file */
#include <queue.h>

/*******************************************************************************
 * GLOBAL VARIABLES
 ******************************************************************************/

/*******************************************************************************
 * FUNCTIONS
 ******************************************************************************/

queue_node_t* queue_create_node(void* data, 
                                queue_alloc_t allocator,
                                OS_RETURN_E *error)
{
    queue_node_t* new_node;

    /* Create new node */
    new_node = allocator.malloc(sizeof(queue_node_t));

    if(new_node == NULL)
    {
        if(error != NULL)
        {
            *error = OS_ERR_MALLOC;
        }
        return NULL;
    }
     /* Init the structure */
    memset(new_node, 0, sizeof(queue_node_t));
    new_node->allocator = allocator;
    new_node->data = data;
    if(error != NULL)
    {
        *error = OS_NO_ERR;
    }
    return new_node;
}

OS_RETURN_E queue_delete_node(queue_node_t** node)
{
    if(node == NULL || *node == NULL)
    {
        return OS_ERR_NULL_POINTER;
    }

    /* Check queue chaining */
    if((*node)->enlisted != 0)
    {
        return OS_ERR_UNAUTHORIZED_ACTION;
    }

    (*node)->allocator.free(*node);
    *node = NULL;

    return OS_NO_ERR;
}

queue_t* queue_create_queue(queue_alloc_t allocator, OS_RETURN_E *error)
{
    queue_t* newqueue;

    /* Create new node */
    newqueue = allocator.malloc(sizeof(queue_t));
    if(newqueue == NULL)
    {
        if(error != NULL)
        {
            *error = OS_ERR_MALLOC;
        }
        return NULL;
    }

    /* Init the structure */
    memset(newqueue, 0, sizeof(queue_t));
    newqueue->allocator = allocator;

    if(error != NULL)
    {
        *error = OS_NO_ERR;
    }

    return newqueue;
}

OS_RETURN_E queue_delete_queue(queue_t** queue)
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

    (*queue)->allocator.free(*queue);
    *queue = NULL;

    return OS_NO_ERR;
}

OS_RETURN_E queue_push(queue_node_t* node, queue_t* queue)
{
    uint32_t word;

    KERNEL_DEBUG("[QUEUE] queue 0x%p in queue 0x%p\n", node, queue);

    if(node == NULL || queue == NULL)
    {
        return OS_ERR_NULL_POINTER;
    }

    ENTER_CRITICAL(word);

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

    KERNEL_DEBUG("[QUEUE] Enqueue element 0x%p in queue 0x%p\n", node, queue);
    KERNEL_DEBUG("[QUEUE] Enqueue element 0x%p 0x%p 0x%p 0x%p 0x%p\n",
            node->next, node->prev, node->enlisted, node->priority, node->data);


    if(node->next != NULL && node->prev != NULL && node->next == node->prev)
    {
        EXIT_CRITICAL(word);
        return OS_ERR_UNAUTHORIZED_ACTION;
    }

    EXIT_CRITICAL(word);

    return OS_NO_ERR;
}


OS_RETURN_E queue_push_prio(queue_node_t* node, 
                            queue_t* queue, 
                            const uintptr_t priority)
{
    queue_node_t* cursor;
    uint32_t      word;

    KERNEL_DEBUG("[QUEUE] Enqueue 0x%p in queue 0x%p\n", node, queue);

    if(node == NULL || queue == NULL)
    {
        KERNEL_DEBUG("[QUEUE] Enqueue NULL\n");
        return OS_ERR_NULL_POINTER;
    }

    ENTER_CRITICAL(word);

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

    KERNEL_DEBUG("[QUEUE] Enqueue element 0x%p in queue 0x%p\n", node, queue);
    KERNEL_DEBUG("[QUEUE] Enqueue element 0x%p 0x%p 0x%p 0x%p 0x%p\n",
            node->next, node->prev, node->enlisted, node->priority, node->data);

    if(node->next != NULL && node->prev != NULL && node->next == node->prev)
    {
        EXIT_CRITICAL(word);
        return OS_ERR_UNAUTHORIZED_ACTION;
    }

    EXIT_CRITICAL(word);

    return OS_NO_ERR;
}

queue_node_t* queue_pop(queue_t* queue, OS_RETURN_E* error)
{
    queue_node_t* node;
    uint32_t      word;

    KERNEL_DEBUG("[QUEUE] Dequeue element in queue 0x%p\n", queue);

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

    ENTER_CRITICAL(word);

    /* If this queue is empty */
    if(queue->head == NULL)
    {
        EXIT_CRITICAL(word);
        return NULL;
    }

    /* Dequeue the last item */
    node = queue->tail;

    KERNEL_DEBUG("[QUEUE] Dequeue element 0x%p in queue 0x%p\n", node, queue);
    KERNEL_DEBUG("[QUEUE] Element 0x%p 0x%p 0x%p 0x%p 0x%p\n",
            node->next, node->prev, node->enlisted, node->priority, node->data);

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

    EXIT_CRITICAL(word);
    return node;
}

queue_node_t* queue_find(queue_t* queue, void* data, OS_RETURN_E *error)
{
    queue_node_t* node;
    uint32_t      word;

    KERNEL_DEBUG("[QUEUE] Find data 0x%p in queue 0x%p\n", data, queue);

    /* If this queue is empty */
    if(queue == NULL)
    {
        if(error != NULL)
        {
            *error = OS_ERR_NULL_POINTER;
        }
        return NULL;
    }

    ENTER_CRITICAL(word);

    /* Search for the data */
    node = queue->head;
    while(node != NULL && node->data != data)
    {
        node = node->next;
    }

    EXIT_CRITICAL(word);

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

OS_RETURN_E queue_remove(queue_t* queue, queue_node_t* node)
{
    queue_node_t* cursor;
    uint32_t      word;

    if(queue == NULL || node == NULL)
    {
        return OS_ERR_NULL_POINTER;
    }

    KERNEL_DEBUG("[QUEUE] Remove node node 0x%p in queue 0x%p\n", node, queue);

    ENTER_CRITICAL(word);

    /* Search for node in the queue*/
    cursor = queue->head;
    while(cursor != NULL && cursor != node)
    {
        cursor = cursor->next;
    }

    if(cursor == NULL)
    {
        EXIT_CRITICAL(word);
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

    EXIT_CRITICAL(word);

    return OS_NO_ERR;
}