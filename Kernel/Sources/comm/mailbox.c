/***************************************************************************//**
 * @file mailbox.c
 *
 * @see mailbox.h
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

#include <lib/stddef.h>       /* Standard definitions */
#include <lib/stdint.h>       /* Generic int types */
#include <lib/string.h>       /* String manipulation */
#include <io/kernel_output.h> /* Kernel output methods */
#include <core/panic.h>       /* Kernel panic */
#include <memory/kheap.h>     /* Kernel heap kfree */
#include <sync/semaphore.h>   /* Semaphores */
#include <sync/critical.h>    /* Critical sections */

/* UTK configuration file */
#include <config.h>

/* Header file */
#include <comm/mailbox.h>

/*******************************************************************************
 * GLOBAL VARIABLES
 ******************************************************************************/

/*******************************************************************************
 * FUNCTIONS
 ******************************************************************************/

OS_RETURN_E mailbox_init(mailbox_t* mailbox)
{
    OS_RETURN_E err;

    /* Pointer check */
    if(mailbox == NULL)
    {
        return OS_ERR_NULL_POINTER;
    }

    /* Init the mailbox */
    memset(mailbox, 0, sizeof(mailbox_t));

#if MAX_CPU_COUNT > 1
    INIT_SPINLOCK(&mailbox->lock);
#endif

    err = sem_init(&mailbox->mailbox_sem_read, 0);
    if(err != OS_NO_ERR)
    {
        return err;
    }
    err = sem_init(&mailbox->mailbox_sem_write, 1);
    if(err != OS_NO_ERR)
    {
        err = sem_destroy(&mailbox->mailbox_sem_read);
        if(err != OS_NO_ERR)
        {
            return err;
        }
        return err;
    }

    mailbox->init = 1;

#if MAILBOX_KERNEL_DEBUG == 1
    kernel_serial_debug("Mailbox 0x%p INIT\n", mailbox);
#endif

    return OS_NO_ERR;
}

OS_RETURN_E mailbox_destroy(mailbox_t* mailbox)
{
    OS_RETURN_E err;
    uint32_t    int_state;

#if MAILBOX_KERNEL_DEBUG == 1
    kernel_serial_debug("Mailbox 0x%p DESTROY\n", mailbox);
#endif

    if(mailbox == NULL)
    {
        return OS_ERR_NULL_POINTER;
    }

#if MAX_CPU_COUNT > 1
    ENTER_CRITICAL(int_state, &mailbox->lock);
#else
    ENTER_CRITICAL(int_state);
#endif

    if(mailbox->init != 1)
    {
#if MAX_CPU_COUNT > 1
        EXIT_CRITICAL(int_state, &mailbox->lock);
#else
        EXIT_CRITICAL(int_state);
#endif
        return OS_ERR_MAILBOX_NON_INITIALIZED;
    }

    /* Set the mailbox to a destroyed state */
    mailbox->init  = 0;

    err = sem_destroy(&mailbox->mailbox_sem_read);
    err |= sem_destroy(&mailbox->mailbox_sem_write);

#if MAX_CPU_COUNT > 1
    EXIT_CRITICAL(int_state, &mailbox->lock);
#else
    EXIT_CRITICAL(int_state);
#endif

    return err;
}

void* mailbox_pend(mailbox_t* mailbox, OS_RETURN_E* error)
{
    OS_RETURN_E err;
    void*       ret_val;
    uint32_t    int_state;

#if MAILBOX_KERNEL_DEBUG == 1
    kernel_serial_debug("Mailbox 0x%p PEND\n", mailbox);
#endif

    /* Check mailbox pointer */
    if(mailbox == NULL)
    {
        if(error != NULL)
        {
            *error = OS_ERR_NULL_POINTER;
        }

        return NULL;
    }

#if MAX_CPU_COUNT > 1
    ENTER_CRITICAL(int_state, &mailbox->lock);
#else
    ENTER_CRITICAL(int_state);
#endif

    /* Check for mailbox initialization */
    if(mailbox->init != 1)
    {
#if MAX_CPU_COUNT > 1
        EXIT_CRITICAL(int_state, &mailbox->lock);
#else
        EXIT_CRITICAL(int_state);
#endif

        if(error != NULL)
        {
            *error = OS_ERR_MAILBOX_NON_INITIALIZED;
        }

        return NULL;
    }

#if MAX_CPU_COUNT > 1
    EXIT_CRITICAL(int_state, &mailbox->lock);
#else
    EXIT_CRITICAL(int_state);
#endif

    /* If the mailbox is empty block thread */
    err = sem_pend(&mailbox->mailbox_sem_read);
    if(err != OS_NO_ERR)
    {
        if(error != NULL)
        {
            *error = OS_ERR_MAILBOX_NON_INITIALIZED;
        }
        return NULL;
    }

#if MAX_CPU_COUNT > 1
    ENTER_CRITICAL(int_state, &mailbox->lock);
#else
    ENTER_CRITICAL(int_state);
#endif

    if(mailbox->init != 1)
    {
#if MAX_CPU_COUNT > 1
        EXIT_CRITICAL(int_state, &mailbox->lock);
#else
        EXIT_CRITICAL(int_state);
#endif

        if(error != NULL)
        {
            *error = OS_ERR_MAILBOX_NON_INITIALIZED;
        }
        return NULL;
    }

    /* Get mailbox value */
    ret_val = mailbox->value;

    err = sem_post(&mailbox->mailbox_sem_write);

#if MAX_CPU_COUNT > 1
    EXIT_CRITICAL(int_state, &mailbox->lock);
#else
    EXIT_CRITICAL(int_state);
#endif

    if(err != OS_NO_ERR)
    {
        if(error != NULL)
        {
            *error = OS_ERR_MAILBOX_NON_INITIALIZED;
        }
        return NULL;
    }

    if(error != NULL)
    {
        *error = OS_NO_ERR;
    }

#if MAILBOX_KERNEL_DEBUG == 1
    kernel_serial_debug("Mailbox 0x%p ACQUIRED\n", mailbox);
#endif

    return ret_val;
}

OS_RETURN_E mailbox_post(mailbox_t* mailbox, void* element)
{
    OS_RETURN_E err;
    uint32_t    int_state;

#if MAILBOX_KERNEL_DEBUG == 1
    kernel_serial_debug("Mailbox 0x%p POST\n", mailbox);
#endif

    if(mailbox == NULL)
    {
        return OS_ERR_NULL_POINTER;
    }

#if MAX_CPU_COUNT > 1
    ENTER_CRITICAL(int_state, &mailbox->lock);
#else
    ENTER_CRITICAL(int_state);
#endif

    if(mailbox->init != 1)
    {
#if MAX_CPU_COUNT > 1
        EXIT_CRITICAL(int_state, &mailbox->lock);
#else
        EXIT_CRITICAL(int_state);
#endif

        return OS_ERR_MAILBOX_NON_INITIALIZED;
    }

#if MAX_CPU_COUNT > 1
    EXIT_CRITICAL(int_state, &mailbox->lock);
#else
    EXIT_CRITICAL(int_state);
#endif

    err = sem_pend(&mailbox->mailbox_sem_write);
    if(err != OS_NO_ERR)
    {
        return OS_ERR_MAILBOX_NON_INITIALIZED;
    }

#if MAX_CPU_COUNT > 1
    ENTER_CRITICAL(int_state, &mailbox->lock);
#else
    ENTER_CRITICAL(int_state);
#endif

    /* Set value of the mailbox */
    mailbox->value = element;

    err = sem_post(&mailbox->mailbox_sem_read);

#if MAX_CPU_COUNT > 1
    EXIT_CRITICAL(int_state, &mailbox->lock);
#else
    EXIT_CRITICAL(int_state);
#endif

    if(err != OS_NO_ERR)
    {
        return OS_ERR_MAILBOX_NON_INITIALIZED;
    }

    return OS_NO_ERR;
}

int32_t mailbox_isempty(mailbox_t* mailbox, OS_RETURN_E* error)
{
    int32_t   ret;
    uint32_t int_state;

    if(mailbox == NULL)
    {
        if(error != NULL)
        {
            *error = OS_ERR_NULL_POINTER;
        }

        return -1;
    }

#if MAX_CPU_COUNT > 1
    ENTER_CRITICAL(int_state, &mailbox->lock);
#else
    ENTER_CRITICAL(int_state);
#endif

    if(mailbox->init != 1)
    {
#if MAX_CPU_COUNT > 1
        EXIT_CRITICAL(int_state, &mailbox->lock);
#else
        EXIT_CRITICAL(int_state);
#endif

        if(error != NULL)
        {
            *error = OS_ERR_MAILBOX_NON_INITIALIZED;
        }

        return -1;
    }

    ret = (mailbox->mailbox_sem_read.sem_level == 0);

#if MAX_CPU_COUNT > 1
    EXIT_CRITICAL(int_state, &mailbox->lock);
#else
    EXIT_CRITICAL(int_state);
#endif

    if(error != NULL)
    {
        *error = OS_NO_ERR;
    }

    return ret;
}