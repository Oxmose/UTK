/*******************************************************************************
 * @file meminfo.c
 *
 * @see meminfo.h
 *
 * @author Alexy Torres Aurora Dugo
 *
 * @date 02/03/2020
 *
 * @version 2.0
 *
 * @brief Kernel memory detector.
 *
 * @details This module is used to detect the memory mapping of the system.
 *
 * @copyright Alexy Torres Aurora Dugo
 ******************************************************************************/

#include <stdint.h>        /* Generic int types */
#include <stddef.h>        /* Standard definitions */
#include <kernel_output.h> /* Kernel output methods */
#include <panic.h>         /* Kernel panic */

/* UTK configuration file */
#include <config.h>

/* Header file */
#include <meminfo.h>

/*******************************************************************************
 * GLOBAL VARIABLES
 ******************************************************************************/
extern uint8_t _START_LOW_STARTUP_ADDR;
extern uint8_t _END_LOW_STARTUP_ADDR;
extern uint8_t _START_HIGH_STARTUP_ADDR;
extern uint8_t _END_HIGH_STARTUP_ADDR;
extern uint8_t _START_TEXT_ADDR;
extern uint8_t _END_TEXT_ADDR;
extern uint8_t _START_RO_DATA_ADDR;
extern uint8_t _END_RODATA_ADDR;
extern uint8_t _START_DATA_ADDR;
extern uint8_t _END_DATA_ADDR;
extern uint8_t _START_BSS_ADDR;
extern uint8_t _END_BSS_ADDR;
extern uint8_t _KERNEL_STACKS_BASE;
extern uint8_t _KERNEL_STACKS_SIZE;
extern uint8_t _KERNEL_HEAP_BASE;
extern uint8_t _KERNEL_HEAP_SIZE;

/*******************************************************************************
 * FUNCTIONS
 ******************************************************************************/

OS_RETURN_E memory_map_init(void)
{
    KERNEL_INFO("=== Kernel memory layout\n");
    KERNEL_INFO("Startup low     0x%p -> 0x%p | %17uKB\n",
                    &_START_LOW_STARTUP_ADDR,
                    &_END_LOW_STARTUP_ADDR,
                    ((uintptr_t)&_END_LOW_STARTUP_ADDR - 
                    (uintptr_t)&_START_LOW_STARTUP_ADDR) >> 10);
    KERNEL_INFO("Startup high    0x%p -> 0x%p | %17uKB\n",
                    &_START_HIGH_STARTUP_ADDR,
                    &_END_HIGH_STARTUP_ADDR,
                    ((uintptr_t)&_END_HIGH_STARTUP_ADDR - 
                    (uintptr_t)&_START_HIGH_STARTUP_ADDR) >> 10);
    KERNEL_INFO("Code            0x%p -> 0x%p | %17uKB\n",
                    &_START_TEXT_ADDR,
                    &_END_TEXT_ADDR,
                    ((uintptr_t)&_END_TEXT_ADDR - 
                    (uintptr_t)&_START_TEXT_ADDR) >> 10);
    KERNEL_INFO("RO-Data         0x%p -> 0x%p | %17uKB\n",
                    &_START_RO_DATA_ADDR,
                    &_END_RODATA_ADDR,
                    ((uintptr_t)&_END_RODATA_ADDR - 
                    (uintptr_t)&_START_RO_DATA_ADDR) >> 10);
    KERNEL_INFO("Data            0x%p -> 0x%p | %17uKB\n",
                    &_START_DATA_ADDR,
                    &_END_DATA_ADDR,
                    ((uintptr_t)&_END_DATA_ADDR - 
                    (uintptr_t)&_START_DATA_ADDR) >> 10);
    KERNEL_INFO("BSS             0x%p -> 0x%p | %17uKB\n",
                    &_START_BSS_ADDR,
                    &_END_BSS_ADDR,
                    ((uintptr_t)&_END_BSS_ADDR - 
                    (uintptr_t)&_START_BSS_ADDR) >> 10);
    KERNEL_INFO("Stacks          0x%p -> 0x%p | %17uKB\n",
                    &_KERNEL_STACKS_BASE,
                    &_KERNEL_STACKS_BASE + (uintptr_t)&_KERNEL_STACKS_SIZE,
                    ((uintptr_t)&_KERNEL_STACKS_SIZE) >> 10);
    KERNEL_INFO("Heap            0x%p -> 0x%p | %17uKB\n",
                    &_KERNEL_HEAP_BASE,
                    &_KERNEL_HEAP_BASE + (uintptr_t)&_KERNEL_HEAP_SIZE,
                    ((uintptr_t)&_KERNEL_HEAP_SIZE) >> 10);


    return OS_NO_ERR;
}