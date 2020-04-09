/*******************************************************************************
 * @file mailbox.h
 *
 * @see mailbox.c
 *
 * @author Alexy Torres Aurora Dugo
 *
 * @date 16/12/2017
 *
 * @version 3.0
 *
 * @brief Mailbox communication and synchronization primitive.
 *
 * @details Mailbox used to send single messages between threads. The mailboxes
 * will block the threads when either full (on a sending thread) or empty (on a
 * receiving thread). The synchronization method used is the semaphore.
 *
 * @warning Mailboxes can only be used when the current system is running and
 * the scheduler initialized.
 *
 * @copyright Alexy Torres Aurora Dugo
 ******************************************************************************/


#ifndef __COMM_MAILBOX_H_
#define __COMM_MAILBOX_H_

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

/** @brief Mailbox definition structure */
struct mailbox
{
    /** @brief The value contained in the mailbox. */
    void* value;

    /** @brief Mailbox's initialization sate. */
    int16_t init;
    /** @brief Mailbox's current state (0 = empty, 1 = full). */
    int16_t state;

    /** @brief Mailbox's reader synchronization semaphore. */
    semaphore_t mailbox_sem_read;
    /** @brief Mailbox's writer synchronization semaphore. */
    semaphore_t mailbox_sem_write;

#if MAX_CPU_COUNT > 1
    /** @brief Critical section spinlock. */
    spinlock_t lock;
#endif
};

/**
 * @brief Defines mailbox_t type as a shorcut for struct mailbox.
 */
typedef struct mailbox mailbox_t;

/*******************************************************************************
 * FUNCTIONS
 ******************************************************************************/

/**
 * @brief Initialize the mailbox given as parameter.
 *
 * @details Initialize the mailbox given as parameter. The function will set the
 * mailbox structure and init the mailbox as empty. See system returns type for
 * error handling.
 *
 * @param[out] mailbox A pointer to the mailbox to initialize. If NULL, the
 * function will immediatly return with the according error code.
 *
 * @return The success state or the error code.
 * - OS_NO_ERR is returned if no error is encountered.
 * - OS_ERR_NULL_POINTER is returned if the pointer to the mailbox to
 *   initialize is NULL.
 */
OS_RETURN_E mailbox_init(mailbox_t *mailbox);

/**
 * @brief Pends on the mailbox given as parameter.
 *
 * @details Pends on the mailbox given as parameter. This function will block
 * the calling thread if the mailbox is empty. See system returns type for error
 * handling.
 *
 * @param[in] mailbox A pointer to the mailbox to pend. If NULL, the function
 * will immediatly return and set error with the according error code.
 * @param[out] error A pointer to the variable that contains the function
 * success state. May be NULL.
 *
 * @return The success state or the error code.
 * - OS_NO_ERR is returned if no error is encountered.
 * - OS_ERR_NULL_POINTER is returned if the pointer to the mailbox to
 *   initialize is NULL.
 * - OS_ERR_MAILBOX_NON_INITIALIZED is returned if the mailbox used is not
 *   initialized.
 */
void* mailbox_pend(mailbox_t *mailbox, OS_RETURN_E *error);

/**
 * @brief Posts on the mailbox given as parameter.
 *
 * @details Posts on the mailbox given as parameter. This function will block
 * the calling thread if the mailbox is full. See system returns type for error
 * handling.
 *
 * @param[in] mailbox A pointer to the mailbox to post. If NULL, the function
 * will immediatly return with the according error code.
 * @param[in] element A pointer to the element to store in the mailbox. Only the
 * pointer is stored in the mailbox, meaning the content of the pointed address
 * might change.
 *
 * @return The success state or the error code.
 * - OS_NO_ERR is returned if no error is encountered.
 * - OS_ERR_NULL_POINTER is returned if the pointer to the mailbox to
 *   initialize is NULL.
 * - OS_ERR_MAILBOX_NON_INITIALIZED is returned if the mailbox used is not
 *   initialized.
 */
OS_RETURN_E mailbox_post(mailbox_t *mailbox, void *element);

/**
 * @brief Destroys the mailbox given as parameter.
 *
 * @details Destroys the mailbox given as parameter. The function will set the
 * mailbox structure to uninitialized and destroy the mailbox. See system
 * returns type for error handling.
 *
 * @param[in, out] mailbox A pointer to the mailbox to destroy. If NULL, the
 * function will immediatly return with the according error code.
 *
 * @return The success state or the error code.
 * - OS_NO_ERR is returned if no error is encountered.
 * - OS_ERR_NULL_POINTER is returned if the pointer to the mailbox to
 *   initialize is NULL.
 * - OS_ERR_MAILBOX_NON_INITIALIZED is returned if the mailbox used is not
 *   initialized.
 */
OS_RETURN_E mailbox_destroy(mailbox_t *mailbox);

/**
 * @brief Returns the mailbox emptyness status.
 *
 * @details Returns the mailbox emptyness status. The function will return -1 in
 * case of error and the error pointer given as parameter will be set
 * accordingly. Returns values are 1 if the mailbox is empty and 0 otherwise.
 *
 * @param[in] mailbox A pointer to the mailbox to test. If NULL, the function
 * will immediatly return and set error with the according error code.
 * @param[out] error A pointer to the variable that contains the function
 * success state. May be NULL. The error values can be the following:
 * - OS_NO_ERR is returned if no error is encountered.
 * - OS_ERR_NULL_POINTER is returned if the pointer to the mailbox to
 *   initialize is NULL.
 * - OS_ERR_MAILBOX_NON_INITIALIZED is returned if the mailbox used is not
 *   initialized.
 *
 * @return 1 if the mailbox is empty and 0 otherwise.
 */
int32_t mailbox_isempty(mailbox_t *mailbox, OS_RETURN_E *error);

#endif /* #ifndef __COMM_MAILBOX_H_ */