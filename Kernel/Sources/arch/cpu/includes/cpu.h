/*******************************************************************************
 * @file cpu.h
 *
 * @see cpu.c
 * 
 * @author Alexy Torres Aurora Dugo
 *
 * @date 27/02/2021
 *
 * @version 1.0
 *
 * @brief CPU management functions
 *
 * @details CPU manipulation functions. Wraps inline assembly calls for 
 * ease of development.
 *
 * @copyright Alexy Torres Aurora Dugo
 ******************************************************************************/

#ifndef __CPU_H_
#define __CPU_H_

#include <config.h> /* Configuration file */

#ifdef ARCH_I386
#include <../i386/includes/cpu.h>
#else 
#error Unknown CPU architecture
#endif

#endif /* #ifndef __CPU_H_ */