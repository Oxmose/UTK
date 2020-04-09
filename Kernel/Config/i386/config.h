/*******************************************************************************
 * @file config.h
 *
 * @see loader.S
 *
 * @author Alexy Torres Aurora Dugo
 *
 * @date 09/03/2018
 *
 * @version 2.0
 *
 * @brief Kernel's main configuration file.
 *
 * @details Kernel configuration's definitions. This file stores the different
 * settings used when compiling UTK.
 *
 * @copyright Alexy Torres Aurora Dugo
 ******************************************************************************/

#ifndef __CONFIG_H_
#define __CONFIG_H_

/*******************************************************************************
 * Memory settings
 ******************************************************************************/

/**
 * @brief Kernel's main stack size in bytes.
 *
 * @warning When modifying this value, do not forget to modify it in the
 * loader.S file.
 */
#define KERNEL_STACK_SIZE 0x4000

/**
 * @brief Kernel's high memory offset.
 *
 * @warning Must be 4MB aligned.
 */
#define KERNEL_MEM_OFFSET 0xE0000000


/** @brief Defines the minimal amount of memory blocks reserved for kernel paging.
 * One block is 4Kb and can map 4MB. Set this number to map the entierety of the
 * kernel.
 */
#define KERNEL_RESERVED_PAGING 4


/*******************************************************************************
 * Architecture settings
 ******************************************************************************/

/** @brief Use i386 architecture. */
#define ARCH_I386  1
/** @brief Use i386 basic BSP. */
#define BSP_I386 1

/** @brief UTK current architecture. */
#define UTK_ARCH ARCH_I386
/** @brief UTK current BSP. */
#define UTK_BSP  BSP_I386

/*******************************************************************************
 * Features settings
 ******************************************************************************/
/** @brief Maximal number of CPU to be supported by the kernel. */
#define MAX_CPU_COUNT  4

/** @brief Display with serial driver. */
#define DISPLAY_SERIAL   0
/** @brief Display with VGA driver. */
#define DISPLAY_VGA      1
/** @brief Display with VESA driver. */
#define DISPLAY_VESA     2
/** @brief Display with buffered VESA driver. */
#define DISPLAY_VESA_BUF 3

/*******************************************************************************
 * I386 Arch Settings
 ******************************************************************************/

/** @brief Enables support for graphic drivers. */
#define DISPLAY_TYPE       DISPLAY_VGA

/** @brief Enables ATA drivers support. */
#define ENABLE_ATA         1

/** @brief Enables GUI support. */
#define ENABLE_GUI         0

/** @brief Enables mouse driver support. */
#define ENABLE_MOUSE       0

/** @brief When VESA drivers are enabled, defines the maximal supported height
 * resolution.
 */
#define MAX_SUPPORTED_HEIGHT 768
/** @brief When VESA drivers are enabled, defines the maximal supported width
 * resolution.
 */
#define MAX_SUPPORTED_WIDTH  1024
/** @brief When VESA drivers are enabled, defines the maximal supported color
 * depth.
 */
#define MAX_SUPPORTED_BPP    32

/*******************************************************************************
 * Global Arch Settings
 ******************************************************************************/
/** @brief Defines which serial port is used for debug purposes. */
#define SERIAL_DEBUG_PORT  COM1


/*******************************************************************************
 * Timers settings
 ******************************************************************************/
/** @brief Defines the current year (usefull for the RTC). */
#define CURRENT_YEAR 2020

/** @brief Defines the kernel's main timer frequency. This will set the maximal
 * scheduling frequency. */
#define KERNEL_MAIN_TIMER_FREQ 200
/** @brief Defines the kernel's auxiliary timer frequency. */
#define KERNEL_AUX_TIMER_FREQ  20
/** @brief Defines the kernel's rtc timer frequency. */
#define KERNEL_RTC_TIMER_FREQ 16

/*******************************************************************************
 * Threads settings
 ******************************************************************************/

/** @brief Defines the maximal length of a thread's name. */
#define THREAD_MAX_NAME_LENGTH   32
/** @brief Defines the thread's maximal stack size in bytes. */
#define THREAD_MAX_STACK_SIZE    0x400000  /* 4 MB */
/** @brief Defines the thread's kernel stack size in bytes. */
#define THREAD_KERNEL_STACK_SIZE 0x4000 /* 16KB */

/*******************************************************************************
 * Peripherals settings
 ******************************************************************************/

/** @brief Enables ATA PIO detection on the primary ATA port. */
#define ATA_PIO_DETECT_PRIMARY_PORT   1
/** @brief Enables ATA PIO detection on the secondary ATA port. */
#define ATA_PIO_DETECT_SECONDARY_PORT 0
/** @brief Enables ATA PIO detection on the third ATA port. */
#define ATA_PIO_DETECT_THIRD_PORT     0
/** @brief Enables ATA PIO detection on the fourth ATA port. */
#define ATA_PIO_DETECT_FOURTH_PORT    0

/*******************************************************************************
 * DEBUG CONFIGURATION
 ******************************************************************************/
/** @brief Enables kernel debuging features. */
#define KERNEL_DEBUG 1

/** @brief Enables PIC driver debuging feature. */
#define PIC_KERNEL_DEBUG 0

/** @brief Enables Serial driver debuging feature. */
#define SERIAL_KERNEL_DEBUG 1

/** @brief Enables CPU feature. */
#define CPU_KERNEL_DEBUG 0

/** @brief Enables Interrupt debuging feature. */
#define INTERRUPT_KERNEL_DEBUG 0

/** @brief Enables Exception debugging feature. */
#define EXCEPTION_KERNEL_DEBUG 0

/** @brief Enables memory debuging feature. */
#define MEMORY_KERNEL_DEBUG 0

/** @brief Enables PIT driver debuging feature. */
#define PIT_KERNEL_DEBUG 0

/** @brief Enables RTC driver debuging feature. */
#define RTC_KERNEL_DEBUG 0

/** @brief Enables timers debuging feature. */
#define TIME_KERNEL_DEBUG 0

/** @brief Enables VESA driver debuging feature. */
#define VESA_KERNEL_DEBUG 1

/** @brief Enables kernel heap debuging feature. */
#define KHEAP_KERNEL_DEBUG 0

/** @brief Enables kernel acpi debuging feature. */
#define ACPI_KERNEL_DEBUG 0

/** @brief Enables kernel io apic debuging feature. */
#define IOAPIC_KERNEL_DEBUG 0

/** @brief Enables kernel lapic debuging feature. */
#define LAPIC_KERNEL_DEBUG 0

/** @brief Enables kernel ata pio debuging feature. */
#define ATA_PIO_KERNEL_DEBUG 0

/** @brief Enables kernel queue debuging feature. */
#define QUEUE_KERNEL_DEBUG 0

/** @brief Enables kernel scheduler debuging feature. */
#define SCHED_KERNEL_DEBUG 0

/** @brief Enables kernel mutex debuging feature. */
#define MUTEX_KERNEL_DEBUG 0

/** @brief Enables kernel semaphore debuging feature. */
#define SEMAPHORE_KERNEL_DEBUG 0

/** @brief Enables kernel mailbox debuging feature. */
#define MAILBOX_KERNEL_DEBUG 0

/** @brief Enables kernel queue debuging feature. */
#define USERQUEUE_KERNEL_DEBUG 0

/** @brief Enables kernel paging debuging feature. */
#define PAGING_KERNEL_DEBUG 0

/** @brief Enables test mode features. */
#define TEST_MODE_ENABLED 0

#endif /* __CONFIG_H_ */
