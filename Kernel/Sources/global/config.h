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

/* Kernel stack default size
 * WARNING This value should be updated to fit other configuration files
 */
#define KERNEL_STACK_SIZE 0x1000

/* Maximal number of CPU supported by the architecture */
#define MAX_CPU_COUNT 4

/* Maximal number of memory regions to be detected by the kernel, increase this
 * value if not all your memory or hardware is detected.
 */
#define MAX_MEMORY_REGION_DETECT 20

/* Kernel log level */
#define DEBUG_LOG_LEVEL   3
#define INFO_LOG_LEVEL    2
#define ERROR_LOG_LEVEL   1
#define NONE_LOG_LEVEL    0

#define KERNEL_LOG_LEVEL DEBUG_LOG_LEVEL

#endif /* #ifndef __GLOBAL_CONFIG_H__ */