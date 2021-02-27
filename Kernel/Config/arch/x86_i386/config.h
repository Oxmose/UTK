/*******************************************************************************
 * @file config.h
 *
 * @author Alexy Torres Aurora Dugo
 *
 * @date 27/02/2021
 *
 * @version 1.0
 *
 * @brief X86 i386 global configuration file.
 *
 * @details X86 i386 global configuration file. This file contains all the
 * settings that the user can set before generating the kernel's binary.
 ******************************************************************************/

#ifndef __GLOBAL_CONFIG_H__
#define __GLOBAL_CONFIG_H__

/* Kernel memory offset 
 * WARNING This value should be updated to fit other configuration files
 */
#define KERNEL_MEM_OFFSET 0xE0000000 

/* Kernel stack default size
 * WARNING This value should be updated to fit other configuration files
 */
#define KERNEL_STACK_SIZE 0x1000

/* Architecture type */
#define ARCH_32_BITS

#endif /* #ifndef __GLOBAL_CONFIG_H__ */