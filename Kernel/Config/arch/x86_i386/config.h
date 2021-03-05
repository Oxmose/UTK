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

/* Architecture definitions */
#define ARCH_I386    1
#define ARCH_32_BITS 1

/* Kernel memory offset 
 * WARNING This value should be updated to fit other configuration files
 */
#define KERNEL_MEM_OFFSET 0xE0000000 
#define KERNEL_MEM_START  0x00100000

/* Kernel stack default size
 * WARNING This value should be updated to fit other configuration files
 */
#define KERNEL_STACK_SIZE 0x1000

/* Maximal number of CPU supported by the architecture */
#define MAX_CPU_COUNT 4

/* Kernel log level */
#define DEBUG_LOG_LEVEL   3
#define INFO_LOG_LEVEL    2
#define ERROR_LOG_LEVEL   1
#define NONE_LOG_LEVEL    0

#define KERNEL_LOG_LEVEL DEBUG_LOG_LEVEL

/* Defines the kernel frame and page size */ 
#define KERNEL_FRAME_SIZE 0x1000
#define KERNEL_PAGE_SIZE  KERNEL_FRAME_SIZE

/* Enable or disable VESA drivers */
#define KERNEL_VESA_ENABLE 0

/* Defines the limit address allocable by the kernel */
#define KERNEL_VIRTUAL_ADDR_MAX 0x100000000
#define KERNEL_VIRTUAL_ADDR_MAX_MASK 0xFFFFFFFF

/** @brief Defines the minimal amount of memory blocks reserved for kernel paging.
 * One block is 4Kb and can map 4MB. Set this number to map the entierety of the
 * kernel.
 */
#define KERNEL_RESERVED_PAGING 4

#endif /* #ifndef __GLOBAL_CONFIG_H__ */