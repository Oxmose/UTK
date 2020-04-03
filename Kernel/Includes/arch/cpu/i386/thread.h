/*******************************************************************************
 * @file thread.h
 *
 * @author Alexy Torres Aurora Dugo
 *
 * @date 03/10/2017
 *
 * @version 2.0
 *
 * @brief Thread's structures definitions.
 *
 * @details Thread's structures definitions. The files contains all the data
 * relative to the thread's management in the system (thread structure, thread
 * state).
 *
 * @copyright Alexy Torres Aurora Dugo
 ******************************************************************************/

#ifndef __I386_THREAD_H_
#define __I386_THREAD_H_

#include <lib/stdint.h>        /* Generic int types */
#include <cpu_settings.h>      /* CPU settings */

/* UTK configuration file */
#include <config.h>

/*******************************************************************************
 * CONSTANTS
 ******************************************************************************/

/** @brief Thread's initial EFLAGS register value. */
#define THREAD_INIT_EFLAGS 0x202 /* INT | PARITY */
/** @brief Thread's initial EAX register value. */
#define THREAD_INIT_EAX    0
/** @brief Thread's initial EBX register value. */
#define THREAD_INIT_EBX    0
/** @brief Thread's initial ECX register value. */
#define THREAD_INIT_ECX    0
/** @brief Thread's initial EDX register value. */
#define THREAD_INIT_EDX    0
/** @brief Thread's initial ESI register value. */
#define THREAD_INIT_ESI    0
/** @brief Thread's initial EDI register value. */
#define THREAD_INIT_EDI    0
/** @brief Thread's initial CS register value. */
#define THREAD_INIT_CS     THREAD_KERNEL_CS
/** @brief Thread's initial SS register value. */
#define THREAD_INIT_SS     THREAD_KERNEL_DS
/** @brief Thread's initial DS register value. */
#define THREAD_INIT_DS     THREAD_KERNEL_DS
/** @brief Thread's initial ES register value. */
#define THREAD_INIT_ES     THREAD_KERNEL_DS
/** @brief Thread's initial FS register value. */
#define THREAD_INIT_FS     THREAD_KERNEL_DS
/** @brief Thread's initial GS register value. */
#define THREAD_INIT_GS     THREAD_KERNEL_DS

/*******************************************************************************
 * STRUCTURES
 ******************************************************************************/

/*******************************************************************************
 * FUNCTIONS
 ******************************************************************************/

#endif /* #ifndef __I386_THREAD_H_ */