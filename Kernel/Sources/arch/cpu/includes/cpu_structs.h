/*******************************************************************************
 * @file cpu_structs.h
 *
 * @see cpu_structs.c
 * 
 * @author Alexy Torres Aurora Dugo
 *
 * @date 27/02/2021
 *
 * @version 1.0
 *
 * @brief CPU structures.
 *
 * @details CPU structures definitions
 *
 * @copyright Alexy Torres Aurora Dugo
 ******************************************************************************/

#ifndef __CPU_STRUCTS_H_
#define __CPU_STRUCTS_H_

#include <config.h> /* Configuration file */

#ifdef ARCH_I386
#include <../i386/includes/cpu_structs.h>
#else 
#error Unknown CPU architecture
#endif

#endif /* #ifndef __CPU_STRUCTS_H_ */