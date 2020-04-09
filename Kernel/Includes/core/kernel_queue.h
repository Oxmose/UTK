/*******************************************************************************
 * @file kernel_queue.h
 *
 * @see kernel_queue.c
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

#ifndef __CORE_KERNEL_QUEUE_H_
#define __CORE_KERNEL_QUEUE_H_

#include <lib/stddef.h>    /* Standard definitons */
#include <lib/stdint.h>    /* Generic int types */
#include <sync/critical.h> /* Critical sections */

/* UTK configuration file */
#include <config.h>

/*******************************************************************************
 * CONSTANTS
 ******************************************************************************/

/*******************************************************************************
 * STRUCTURES
 ******************************************************************************/

/** @brief Kernel queue node structure. */
struct kernel_queue_node
{
    /** @brief Next node in the queue. */
    struct kernel_queue_node* next;
    /** @brief Previous node in the queue. */
    struct kernel_queue_node* prev;

    /** @brief Tell if the node is present in a queue or stands alone. */
    uint16_t enlisted;

    /** @brief Node's priority, used when the queue is a priority queue. */
    uint32_t priority;

    /** @brief Node's data pointer. Store the address of the contained data. */
    void* data;
};

/**
 * @brief Defines kernel_queue_node_t type as a shorcut for struct
 * kernel_queue_node.
 */
typedef struct kernel_queue_node kernel_queue_node_t;

/** @brief Kernel queue structure. */
struct kernel_queue
{
    /** @brief Head of the queue. */
    struct kernel_queue_node* head;
    /** @brief Tail of the queue. */
    struct kernel_queue_node* tail;

    /** @brief Current queue's size. */
    uint32_t size;

#if MAX_CPU_COUNT > 1
    /** @brief Critical section spinlock. */
    spinlock_t lock;
#endif
};

/**
 * @brief Defines kernel_queue_t type as a shorcut for struct kernel_queue.
 */
typedef struct kernel_queue kernel_queue_t;

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
 * @param[out] error A pointer to the variable that contains the function
 * success state. May be NULL. The error values can be the following:
 * - OS_ERR_MALLOC if the kernel failed to allocate memory for the node.
 * - OS_NO_ERR if no error is encountered.
 *
 * @returns The node pointer is returned. In case of error NULL is returned.
 */
kernel_queue_node_t* kernel_queue_create_node(void* data, OS_RETURN_E *error);

/**
 * @brief Deletes a queue node.
 *
 * @details Deletes a node from the memory. The node should not be used in any
 * queue. If it is the case, the function will return an error.
 *
 * @param[in, out] node The node pointer of pointer to destroy.
 *
 * @return The success state or the error code.
 * - OS_NO_ERR is returned if no error is encountered.
 * - OS_ERR_NULL_POINTER is returned if the pointer to the node is NULL.
 * - OS_ERR_UNAUTHORIZED_ACTION is returned if the node is still used in a list.
 */
OS_RETURN_E kernel_queue_delete_node(kernel_queue_node_t** node);

/**
 * @brief Creates an empty queue ready to be used.
 *
 * @details Creates and initializes a new kernel queue. The returned queue is
 * ready to be used.
 *
 * @param[out] error A pointer to the variable that will contain the function
 * success state. May be NULL. The error values can be the following:
 * - OS_ERR_MALLOC if the kernel failed to allocate memory for the queue.
 * - OS_NO_ERR if no error is encountered.
 *
 * @returns The queue pointer is returned. In case of error NULL is returned.
 */
kernel_queue_t* kernel_queue_create_queue(OS_RETURN_E *error);

/**
 * @brief Deletes a previously created queue.
 *
 * @details Delete a queue from the memory. If the queue is not empty an error
 * is returned.
 *
 * @param[in, out] queue The queue pointer of pointer to destroy.
 *
 * @return The success state or the error code.
 * - OS_NO_ERR is returned if no error is encountered.
 * - OS_ERR_NULL_POINTER is returned if the pointer to the queue is NULL.
 * - OS_ERR_UNAUTHORIZED_ACTION is returned if the queue is not empty.
 */
OS_RETURN_E kernel_queue_delete_queue(kernel_queue_t** queue);

/**
 * @brief Enlists a node in the queue.
 *
 * @details Enlists a node in the queue given as parameter. The data will be
 * placed in the tail of the queue.
 *
 * @param[in] node A now node to add in the queue.
 * @param[in, out] queue The queue to manage.
 *
 * @return The success state or the error code.
 * - OS_NO_ERR is returned if no error is encountered.
 * - OS_ERR_NULL_POINTER is returned if the pointer to the queue or the node is
 *   NULL.
 */
OS_RETURN_E kernel_queue_push(kernel_queue_node_t* node,
                              kernel_queue_t* queue);

/**
 * @brief Enlists a node in the queue.
 *
 * @details Enlist a node in the queue given as parameter. The data will be
 * placed in the queue with regard to the priority argument.
 *
 * @param[in] node A now node to add in the queue.
 * @param[in, out] queue The queue to manage.
 * @param[in] priority The element priority.
 *
 * @return The success state or the error code.
 * - OS_NO_ERR is returned if no error is encountered.
 * - OS_ERR_NULL_POINTER is returned if the pointer to the queue or the node is
 *   NULL.
 */
OS_RETURN_E kernel_queue_push_prio(kernel_queue_node_t* node,
                                   kernel_queue_t* queue,
                                   const uint32_t priority);

/**
 * @brief Removes a node from a queue.
 *
 * @details Removes a node from the queue given as parameter. The retreived node
 * that is returned is the one placed in the head of the QUEUE.
 *
 * @param[in, out] queue The queue to manage.
 * @param[out] error A pointer to the variable that contains the function
 * success state. May be NULL. The error values can be the following:
 * - OS_ERR_NULL_POINTER is returned if the pointer to the queue or the node is
 *   NULL.
 * - OS_NO_ERR if no error is encountered.
 *
 * @return The data pointer placed in the head of the queue is returned.
 */
kernel_queue_node_t* kernel_queue_pop(kernel_queue_t* queue,
                                      OS_RETURN_E* error);

/**
 * @brief Finds a node containing the data given as parameter in the queue.
 *
 * @details Find a node containing the data given as parameter in the queue.
 * An error is set if not any node is found.
 *
 * @param[in] queue The queue to search the data in.
 * @param[in] data The data contained by the node to find.
 * @param[out] error A pointer to the variable that contains the function
 * successstate. May be NULL. The error values can be the following:
 * - OS_ERR_NULL_POINTER is returned if the pointer to the queue or the node is
 *   NULL.
 * - OS_ERR_NO_SUCH_ID is returned if the data could not be found in the queue.
 * - OS_NO_ERR if no error is encountered.
 *
 * @return The function returns a pointer to the node if found, NULL otherwise.
 */
kernel_queue_node_t* kernel_queue_find(kernel_queue_t* queue, void* data,
                                       OS_RETURN_E *error);

/**
 * @brief Removes a node from a queue.
 *
 * @details Removes a node from a queue given as parameter. If the node is not
 * found, nothing is done and an error is returned.
 *
 * @param[in, out] queue The queue containing the node.
 * @param[in] node The node to remove.
 *
 * @return The success state or the error code.
 * - OS_NO_ERR is returned if no error is encountered.
 * - OS_ERR_NULL_POINTER is returned if the pointer to the queue or the node is
 *   NULL.
 * - OS_ERR_NO_SUCH_ID is returned if the data could not be found in the queue.
 */
OS_RETURN_E kernel_queue_remove(kernel_queue_t* queue,
                                kernel_queue_node_t* node);

#endif /* #ifndef __CORE_KERNEL_QUEUE_H_ */