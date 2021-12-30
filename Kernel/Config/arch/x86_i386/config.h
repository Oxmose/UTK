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

#ifndef __GLOBAL_CONFIG_H_
#define __GLOBAL_CONFIG_H_

/* Architecture definitions */
#define ARCH_I386    1
#define ARCH_32_BITS 1

/* Kernel memory offset
 * WARNING This value should be updated to fit other configuration files
 */
#define KERNEL_MEM_OFFSET 0xE0000000
#define KERNEL_MEM_START  0x00100000

/* Defines the processes virtual space start address */
#define PROCESS_START_VIRT_SPACE 0x00100000

/* Kernel stack default size
 * WARNING This value should be updated to fit other configuration files
 */
#define KERNEL_STACK_SIZE 0x1000

/* Thread's kernel stack size in bytes */
#define THREAD_KERNEL_STACK_SIZE 0x1000

/* Defines the process heap size, in next version this could be a dynamic value
 * based on the available memory. But right now we use a very simple heap.
 */
#define PROCESS_HEAP_SIZE 0xA00000

/* Maximal number of CPU supported by the architecture */
#define MAX_CPU_COUNT 4

/* Kernel log level */
#define DEBUG_LOG_LEVEL   3
#define INFO_LOG_LEVEL    2
#define ERROR_LOG_LEVEL   1
#define NONE_LOG_LEVEL    0

#define KERNEL_LOG_LEVEL DEBUG_LOG_LEVEL

/* Kernel log on UART */
#define DEBUG_LOG_UART 1

/* Defines the limit address allocable by the kernel */
#define KERNEL_VIRTUAL_ADDR_MAX 0x100000000
#define KERNEL_VIRTUAL_ADDR_MAX_MASK 0xFFFFFFFF

/**
 * @brief Defines the minimal amount of memory blocks reserved for kernel paging
 * One block is 4Kb and can map 4MB. Set this number to map the entierety of the
 * kernel.
 */
#define KERNEL_RESERVED_PAGING 4

/** @brief Defines the maximum number of process in the system. This number is
 * limited by the PCID feature.*/
#define KERNEL_MAX_PROCESS 4096

/** @brief System's main timer interrupt frequency */
#define KERNEL_MAIN_TIMER_FREQ 200

/** @brief System's RTC interrupt frequency */
#define KERNEL_RTC_TIMER_FREQ 5

/** @brief Current year */
#define CURRENT_YEAR 2022

/** @brief Thread's private storage size in bytes */
#define THREAD_PRIVATE_STORAGE_SIZE 0x200

/* Stack default alignement */
#define STACK_ALIGN 4

/*******************************************************************************
 * DEBUG Configuration
 *
 * Set to 0 to disable debug output for a specific module
 * Set to 1 to enable debug output for a specific module
 ******************************************************************************/
#define ACPI_DEBUG_ENABLED 0
#define CPU_DEBUG_ENABLED 0
#define EXCEPTIONS_DEBUG_ENABLED 0
#define INTERRUPTS_DEBUG_ENABLED 0
#define IOAPIC_DEBUG_ENABLED 0
#define KHEAP_DEBUG_ENABLED 0
#define KICKSTART_DEBUG_ENABLED 0
#define LAPIC_DEBUG_ENABLED 0
#define MEMMGT_DEBUG_ENABLED 0
#define PIC_DEBUG_ENABLED 0
#define PIT_DEBUG_ENABLED 0
#define QUEUE_DEBUG_ENABLED 0
#define KQUEUE_DEBUG_ENABLED 0
#define RTC_DEBUG_ENABLED 0
#define SCHED_DEBUG_ENABLED 0
#define SCHED_ELECT_DEBUG_ENABLED 0
#define SCHED_SWITCH_DEBUG_ENABLED 0
#define SERIAL_DEBUG_ENABLED 0
#define TIME_MGT_DEBUG_ENABLED 0
#define VGA_DEBUG_ENABLED 0
#define SYSCALL_DEBUG_ENABLED 0
#define INITRD_DEBUG_ENABLED 0
#define USTAR_DEBUG_ENABLED 0
#define MUTEX_DEBUG_ENABLED 0
#define TEMP_DEBUG_ENABLED 0
#define USER_HEAP_DEBUG_ENABLED 0
#define SEMAPHORE_DEBUG_ENABLED 0

#endif /* #ifndef __GLOBAL_CONFIG_H_ */