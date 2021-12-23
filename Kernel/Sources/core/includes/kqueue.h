/*******************************************************************************
 * @file kqueue.h
 *
 * @see kqueue.c
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

#ifndef __CORE_KQUEUE_H_
#define __CORE_KQUEUE_H_

/*******************************************************************************
 * INCLUDES
 ******************************************************************************/

#include <stddef.h>    /* Standard definitons */
#include <stdint.h>    /* Generic int types */

/*******************************************************************************
 * CONSTANTS
 ******************************************************************************/

/* None */

/*******************************************************************************
 * STRUCTURES AND TYPES
 ******************************************************************************/

/** @brief Queue node structure. */
typedef struct kqueue_node
{
    /** @brief Next node in the queue. */
    struct kqueue_node* next;
    /** @brief Previous node in the queue. */
    struct kqueue_node* prev;

    /** @brief Tell if the node is present in a queue or stands alone. */
    bool_t enlisted;

    /** @brief Node's priority, used when the queue is a priority queue. */
    uintptr_t priority;

    /** @brief Node's data pointer. Store the address of the contained data. */
    void* data;
} kqueue_node_t;

/** @brief Queue structure. */
typedef struct
{
    /** @brief Head of the queue. */
    struct kqueue_node* head;
    /** @brief Tail of the queue. */
    struct kqueue_node* tail;

    /** @brief Current queue's size. */
    size_t size;
} kqueue_t;

/*******************************************************************************
 * MACROS
 ******************************************************************************/

/* None */

/*******************************************************************************
 * GLOBAL VARIABLES
 ******************************************************************************/

/* Imported global variables */
/* None */

/* Exported global variables */
/* None */

/* Static global variables */
/* None */

/*******************************************************************************
 * FUNCTIONS
 ******************************************************************************/

/**
 * @brief Creates a new queue node.
 *
 * @details Creates a node ready to be inserted in a queue. The data can be
 * modified later by accessing the data field of the node structure.
 *
 * @warning A node should be only used in one queue at most.
 *
 * @param[in] data The pointer to the data to carry in the node.
 *
 * @returns The node pointer is returned.
 */
kqueue_node_t* kqueue_create_node(void* data);

/**
 * @brief Deletes a queue node.
 *
 * @details Deletes a node from the memory. The node should not be used in any
 * queue. If it is the case, the function will return an error.
 *
 * @param[in, out] node The node pointer of pointer to destroy.
 */
void kqueue_delete_node(kqueue_node_t** node);

/**
 * @brief Creates an empty queue ready to be used.
 *
 * @details Creates and initializes a new queue. The returned queue is
 * ready to be used.
 */
kqueue_t* kqueue_create_queue(void);

/**
 * @brief Deletes a previously created queue.
 *
 * @details Delete a queue from the memory. If the queue is not empty an error
 * is returned.
 *
 * @param[in, out] queue The queue pointer of pointer to destroy.
 */
void kqueue_delete_queue(kqueue_t** queue);

/**
 * @brief Enlists a node in the queue.
 *
 * @details Enlists a node in the queue given as parameter. The data will be
 * placed in the tail of the queue.
 *
 * @param[in] node A now node to add in the queue.
 * @param[in, out] queue The queue to manage.
 */
void kqueue_push(kqueue_node_t* node, kqueue_t* queue);

/**
 * @brief Enlists a node in the queue.
 *
 * @details Enlist a node in the queue given as parameter. The data will be
 * placed in the queue with regard to the priority argument.
 *
 * @param[in] node A now node to add in the queue.
 * @param[in, out] queue The queue to manage.
 * @param[in] priority The element priority.
 */
void kqueue_push_prio(kqueue_node_t* node,
                      kqueue_t* queue,
                      const uintptr_t priority);

/**
 * @brief Removes a node from a queue.
 *
 * @details Removes a node from the queue given as parameter. The retreived node
 * that is returned is the one placed in the head of the QUEUE.
 *
 * @param[in, out] queue The queue to manage.
 *
 * @return The data pointer placed in the head of the queue is returned.
 */
kqueue_node_t* kqueue_pop(kqueue_t* queue);

/**
 * @brief Finds a node containing the data given as parameter in the queue.
 *
 * @details Find a node containing the data given as parameter in the queue.
 * An error is set if not any node is found.
 *
 * @param[in] queue The queue to search the data in.
 * @param[in] data The data contained by the node to find.
 *
 * @return The function returns a pointer to the node if found, NULL otherwise.
 */
kqueue_node_t* kqueue_find(kqueue_t* queue, void* data);

/**
 * @brief Removes a node from a queue.
 *
 * @details Removes a node from a queue given as parameter. If the node is not
 * found, nothing is done and an error is returned.
 *
 * @param[in, out] queue The queue containing the node.
 * @param[in] node The node to remove.
 * @param[in] panic Tells if the function should generate a kernel panic of the
 * node is not found.
 */
void kqueue_remove(kqueue_t* queue, kqueue_node_t* node, const bool_t panic);


#endif /* #ifndef __CORE_KQUEUE_H_ */

/* EOF */