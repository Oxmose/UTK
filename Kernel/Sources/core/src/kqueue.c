/*******************************************************************************
 * @file kqueue.c
 *
 * @see kqueue.h
 *
 * @author Alexy Torres Aurora Dugo
 *
 * @date 03/03/2021
 *
 * @version 1.5
 *
 * @brief Kernel specific queue structures.
 *
 * @details Kernel specific queue structures. These queues are used as priority
 * queue or regular queues. A queue can virtually store every type of data and
 * is just a wrapper.
 *
 * @warning This implementation is not thread safe.
 *
 * @copyright Alexy Torres Aurora Dugo
 ******************************************************************************/

/*******************************************************************************
 * INCLUDES
 ******************************************************************************/

/* Included headers */
#include <stddef.h>        /* Standard definitions */
#include <stdint.h>        /* Generic int types */
#include <string.h>        /* String manipulation */
#include <panic.h>         /* Kernel panic */
#include <kernel_output.h> /* Kernel output methods */
#include <kheap.h>         /* Kernel heap */
#include <kernel_error.h>  /* Kernel errors definitions */

/* Configuration files */
#include <config.h>
#include <test_bank.h>

/* Header file */
#include <kqueue.h>

/*******************************************************************************
 * CONSTANTS
 ******************************************************************************/

/* None */

/*******************************************************************************
 * STRUCTURES AND TYPES
 ******************************************************************************/

/* None */

/*******************************************************************************
 * MACROS
 ******************************************************************************/

#define KQUEUE_ASSERT(COND, MSG, ERROR) {                    \
    if((COND) == FALSE)                                      \
    {                                                        \
        PANIC(ERROR, "KQUEUE", MSG, TRUE);                   \
    }                                                        \
}

/*******************************************************************************
 * GLOBAL VARIABLES
 ******************************************************************************/

/************************* Imported global variables **************************/
/* None */

/************************* Exported global variables **************************/
/* None */

/************************** Static global variables ***************************/
/* None */

/*******************************************************************************
 * STATIC FUNCTIONS DECLARATIONS
 ******************************************************************************/

/* None */

/*******************************************************************************
 * FUNCTIONS
 ******************************************************************************/

kqueue_node_t* kqueue_create_node(void* data)
{
    kqueue_node_t* new_node;

    /* Create new node */
    new_node = kmalloc(sizeof(kqueue_node_t));
    KQUEUE_ASSERT(new_node != NULL, "Could not allocate knode", OS_ERR_MALLOC);


    /* Init the structure */
    memset(new_node, 0, sizeof(kqueue_node_t));
    new_node->data = data;

    return new_node;
}

void kqueue_delete_node(kqueue_node_t** node)
{
    KQUEUE_ASSERT((node != NULL && *node != NULL),
                  "Tried to delete a NULL node",
                  OS_ERR_NULL_POINTER);

    KQUEUE_ASSERT((*node)->enlisted == FALSE,
                  "Tried to delete an enlisted node",
                  OS_ERR_UNAUTHORIZED_ACTION);

    kfree(*node);
    *node = NULL;
}

kqueue_t* kqueue_create_queue(void)
{
    kqueue_t* new_queue;

    /* Create new node */
    new_queue = kmalloc(sizeof(kqueue_t));
    KQUEUE_ASSERT(new_queue != NULL,
                  "Could not allocate kqueue",
                  OS_ERR_MALLOC);

    /* Init the structure */
    memset(new_queue, 0, sizeof(kqueue_t));

    return new_queue;
}

void kqueue_delete_queue(kqueue_t** queue)
{
    KQUEUE_ASSERT((queue != NULL && *queue != NULL),
                  "Tried to delete a NULL queue",
                  OS_ERR_NULL_POINTER);

    KQUEUE_ASSERT(((*queue)->head == NULL && (*queue)->tail == NULL),
                  "Tried to delete a non empty queue",
                  OS_ERR_UNAUTHORIZED_ACTION);

    kfree(*queue);
    *queue = NULL;
}

void kqueue_push(kqueue_node_t* node, kqueue_t* queue)
{
    KERNEL_DEBUG(KQUEUE_DEBUG_ENABLED,
                 "[KQUEUE] KQueue try push knode 0x%p in kqueue 0x%p",
                 node,
                 queue);

    KQUEUE_ASSERT((node != NULL && queue != NULL),
                  "Cannot push with NULL knode or NULL kqueue",
                  OS_ERR_NULL_POINTER);

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
        /* Just put on the head */
        node->next = queue->head;
        node->prev = NULL;
        queue->head->prev = node;
        queue->head = node;
    }

    ++queue->size;
    node->enlisted = TRUE;

    KQUEUE_ASSERT((node->next != node->prev ||
                   node->next == NULL ||
                   node->prev == NULL),
                  "Cycle detected in KQueue",
                  OS_ERR_NULL_POINTER);

    KERNEL_DEBUG(KQUEUE_DEBUG_ENABLED,
                 "[KQUEUE] KQueue pushed knode 0x%p in kqueue 0x%p",
                 node,
                 queue);
}


void kqueue_push_prio(kqueue_node_t* node,
                      kqueue_t* queue,
                      const uintptr_t priority)
{
    kqueue_node_t* cursor;

    KERNEL_DEBUG(KQUEUE_DEBUG_ENABLED,
                 "[KQUEUE] KQueue try push prio knode 0x%p in kqueue 0x%p",
                 node,
                 queue);

    KQUEUE_ASSERT((node != NULL && queue != NULL),
                  "Cannot push with NULL knode or NULL kqueue",
                  OS_ERR_NULL_POINTER);

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
    node->enlisted = TRUE;

    KQUEUE_ASSERT((node->next != node->prev ||
                   node->next == NULL ||
                   node->prev == NULL),
                  "Cycle detected in KQueue",
                  OS_ERR_NULL_POINTER);

    KERNEL_DEBUG(KQUEUE_DEBUG_ENABLED,
                 "[KQUEUE] KQueue pushed knode 0x%p in kqueue 0x%p",
                 node,
                 queue);
}

kqueue_node_t* kqueue_pop(kqueue_t* queue)
{
    kqueue_node_t* node;

    KERNEL_DEBUG(KQUEUE_DEBUG_ENABLED,
                 "[KQUEUE] KQueue try pop knode from kqueue 0x%p",
                 queue);

    KQUEUE_ASSERT(queue != NULL,
                  "Cannot pop NULL kqueue",
                  OS_ERR_NULL_POINTER);

    /* If this queue is empty */
    if(queue->head == NULL)
    {
        return NULL;
    }

    /* Dequeue the last item */
    node = queue->tail;

    KERNEL_DEBUG(KQUEUE_DEBUG_ENABLED,
                 "[KQUEUE] Pop knode 0x%p from kqueue 0x%p",
                 node,
                 queue);

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
    node->enlisted = FALSE;

    return node;
}

kqueue_node_t* kqueue_find(kqueue_t* queue, void* data)
{
    kqueue_node_t* node;

    KERNEL_DEBUG(KQUEUE_DEBUG_ENABLED,
                 "[KQUEUE] KQueue try find data 0x%p from kqueue 0x%p",
                 data,
                 queue);

    KQUEUE_ASSERT(queue != NULL,
                  "Cannot find in NULL kqueue",
                  OS_ERR_NULL_POINTER);

    /* Search for the data */
    node = queue->head;
    while(node != NULL && node->data != data)
    {
        node = node->next;
    }

    KERNEL_DEBUG(KQUEUE_DEBUG_ENABLED,
                 "[KQUEUE] KQueue found node 0x%p from kqueue 0x%p",
                 node,
                 queue);

    return node;
}

void kqueue_remove(kqueue_t* queue, kqueue_node_t* node, const bool_t panic)
{
    kqueue_node_t* cursor;

    KERNEL_DEBUG(KQUEUE_DEBUG_ENABLED,
                 "[KQUEUE] KQueue try renove knode 0x%p from kqueue 0x%p",
                 node,
                 queue);

    KQUEUE_ASSERT((node != NULL && queue != NULL),
                  "Cannot remove with NULL knode or NULL kqueue",
                  OS_ERR_NULL_POINTER);

    /* Search for node in the queue*/
    cursor = queue->head;
    while(cursor != NULL && cursor != node)
    {
        cursor = cursor->next;
    }

    KQUEUE_ASSERT((cursor != NULL || panic == FALSE),
                  "Could not find knode to remove",
                  OS_ERR_NO_SUCH_ID);

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

    node->enlisted = FALSE;

    KERNEL_DEBUG(KQUEUE_DEBUG_ENABLED,
                 "[KQUEUE] KQueue renoved knode 0x%p from kqueue 0x%p",
                 node,
                 queue);
}

/************************************ EOF *************************************/