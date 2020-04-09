/*******************************************************************************
 * @file queue.h
 *
 * @see queue.c
 *
 * @author Alexy Torres Aurora Dugo
 *
 * @date 16/12/2017
 *
 * @version 3.0
 *
 * @brief Queue communication and synchronization primitive.
 *
 * @details Queue used to send multiple messages between threads. The queues
 * will block the threads when either full (on a sending thread) or empty (on a
 * receiving thread). The synchronization method used is the semaphore.
 *
 * @warning Queues can only be used when the current system is running and
 * the scheduler initialized.
 *
 * @copyright Alexy Torres Aurora Dugo
 ******************************************************************************/

#ifndef __COMM_QUEUE_H_
#define __COMM_QUEUE_H_

#include <lib/stddef.h>     /* Standard definitions */
#include <lib/stdint.h>     /* Generic int types */
#include <sync/semaphore.h> /* Semaphores */
#include <sync/critical.h>  /* Critical sections */

/*******************************************************************************
 * CONSTANTS
 ******************************************************************************/

/*******************************************************************************
 * STRUCTURES
 ******************************************************************************/

/** @brief Queue definition structure */
struct queue
{
    /** @brief Queue's data container. */
    void** container;

    /** @brief Queue's current size. */
    uint32_t size;
    /** @brief Queue's maximal size. */
    uint32_t max_size;
    /** @brief Current top (head) index in the queue's circular array. */
    uint32_t index_top;
    /** @brief Current bottom (tail) index in the queue's circular array. */
    uint32_t index_bot;

    /** @brief Queue's initialization sate. */
    int32_t init;

    /** @brief Queue's reader synchronization semaphore. */
    semaphore_t queue_sem_read;
    /** @brief Queue's writer synchronization semaphore. */
    semaphore_t queue_sem_write;

#if MAX_CPU_COUNT > 1
    /** @brief Critical section spinlock. */
    spinlock_t lock;
#endif
};

/**
 * @brief Defines queue_t type as a shorcut for struct queue.
 */
typedef struct queue queue_t;

/*******************************************************************************
 * FUNCTIONS
 ******************************************************************************/

/**
 * @brief Initialize the queue given as parameter.
 *
 * @details Initialize the queue given as parameter. The function will set the
 * queue structure and init the queue as empty. See system returns type for
 * error handling.
 *
 * @param[out] queue A pointer to the queue to initialize. If NULL, the
 * function will immediatly return with the according error code.
 *
 * @return The success state or the error code.
 * - OS_NO_ERR is returned if no error is encountered.
 * - OS_ERR_NULL_POINTER is returned if the pointer to the queue to
 *   initialize is NULL.
 */
OS_RETURN_E queue_init(queue_t *queue, const uint32_t size);

/**
 * @brief Pends on the queue given as parameter.
 *
 * @details Pends on the queue given as parameter. This function will block
 * the calling thread if the queue is empty. See system returns type for error
 * handling.
 *
 * @param[in] queue A pointer to the queue to pend. If NULL, the function
 * will immediatly return and set error with the according error code.
 * @param[out] error A pointer to the variable that contains the function
 * success state. May be NULL.
 *
 * @return The success state or the error code.
 * - OS_NO_ERR is returned if no error is encountered.
 * - OS_ERR_NULL_POINTER is returned if the pointer to the queue to
 *   initialize is NULL.
 * - OS_ERR_QUEUE_NON_INITIALIZED is returned if the queue used is not
 *   initialized.
 */
void* queue_pend(queue_t *queue, OS_RETURN_E *error);

/**
 * @brief Posts on the queue given as parameter.
 *
 * @details Posts on the queue given as parameter. This function will block
 * the calling thread if the queue is full. See system returns type for error
 * handling.
 *
 * @param[in] queue A pointer to the queue to post. If NULL, the function
 * will immediatly return with the according error code.
 * @param[in] element A pointer to the element to store in the queue. Only the
 * pointer is stored in the queue, meaning the content of the pointed address
 * might change.
 *
 * @return The success state or the error code.
 * - OS_NO_ERR is returned if no error is encountered.
 * - OS_ERR_NULL_POINTER is returned if the pointer to the queue to
 *   initialize is NULL.
 * - OS_ERR_QUEUE_NON_INITIALIZED is returned if the queue used is not
 *   initialized.
 */
OS_RETURN_E queue_post(queue_t *queue, void *element);

/**
 * @brief Destroys the queue given as parameter.
 *
 * @details Destroys the queue given as parameter. The function will set the
 * queue structure to uninitialized and destroy the queue. See system
 * returns type for error handling.
 *
 * @param[in, out] queue A pointer to the queue to destroy. If NULL, the
 * function will immediatly return with the according error code.
 *
 * @return The success state or the error code.
 * - OS_NO_ERR is returned if no error is encountered.
 * - OS_ERR_NULL_POINTER is returned if the pointer to the queue to
 *   initialize is NULL.
 * - OS_ERR_QUEUE_NON_INITIALIZED is returned if the queue used is not
 *   initialized.
 */
OS_RETURN_E queue_destroy(queue_t *queue);

/**
 * @brief Returns the queue emptyness status.
 *
 * @details Returns the queue emptyness status. The function will return -1 in
 * case of error and the error pointer given as parameter will be set
 * accordingly. Returns values are 1 if the queue is empty and 0 otherwise.
 *
 * @param[in] queue A pointer to the queue to test. If NULL, the function
 * will immediatly return and set error with the according error code.
 * @param[out] error A pointer to the variable that contains the function
 * success state. May be NULL. The error values can be the following:
 * - OS_NO_ERR is returned if no error is encountered.
 * - OS_ERR_NULL_POINTER is returned if the pointer to the queue to
 *   initialize is NULL.
 * - OS_ERR_QUEUE_NON_INITIALIZED is returned if the queue used is not
 *   initialized.
 *
 * @return 1 if the queue is empty and 0 otherwise.
 */
int32_t queue_isempty(queue_t *queue, OS_RETURN_E *error);

/**
 * @brief Returns the size of the queue.
 *
 * @details Returns the queue size. The size is the number of elements currently
 * contained in the queue. *
 *
 * @param[in] queue A pointer to the queue to test. If NULL, the function
 * will immediatly return and set error with the according error code.
 * @param[out] error A pointer to the variable that contains the function success
 * state. May be NULL. The error values can be the following:
 * - OS_NO_ERR is returned if no error is encountered.
 * - OS_ERR_NULL_POINTER is returned if the pointer to the queue to
 *   initialize is NULL.
 * - OS_ERR_QUEUE_NON_INITIALIZED is returned if the queue used is not
 *   initialized.
 *
 * @returns The function returns -1 on error, the size of the queue otherzise.
 */
int32_t queue_size(queue_t *queue, OS_RETURN_E *error);

#endif /* #ifndef __COMM_QUEUE_H_ */