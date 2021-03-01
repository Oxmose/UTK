/*******************************************************************************
 * UTK linker file for x86 i386 target
 *
 * Created: 27/02/2020
 *
 * Author: Alexy Torres Aurora Dugo
 *
 ******************************************************************************/

/* Entry point */
ENTRY(__kinit)

/* Kernel starts at 1M in physical addresse space
 * Add the offset, to fit with values in config.h and config.inc
 */
KERNEL_MEM_OFFSET = 0xE0000000;

/* Memory map */
MEMORY
{
    LOW_STARTUP_CODE    (rx)    :   ORIGIN = 0x00100000,    LENGTH = 4K
    HIGH_TARTUP_CODE    (rx)    :   ORIGIN = 0xE0101000,    LENGTH = 4K
    KERNEL_CODE         (rx)    :   ORIGIN = 0xE0102000,    LENGTH = 256K
    KERNEL_RO_DATA      (r)     :   ORIGIN = 0xE0200000,    LENGTH = 32K
    KERNEL_RW_DATA      (rw)    :   ORIGIN = 0xE0208000,    LENGTH = 512K
    KERNEL_STACKS       (rw)    :   ORIGIN = 0xE0300000,    LENGTH = 32K
    KERNEL_HEAP         (rw)    :   ORIGIN = 0xE0308000,    LENGTH = 32K    
}

/* Memory layout */
SECTIONS
{
    /* Contains the startup code used by grub to bootstrap the kernel */
    . = ORIGIN(LOW_STARTUP_CODE);
    .low_startup ALIGN(4K) :
    {
        *(.multiboot)

        . = ALIGN(16);
        *(.low_startup_code)
    } > LOW_STARTUP_CODE
    . = ORIGIN(HIGH_TARTUP_CODE);
    .high_startup ALIGN(4K) : AT(ADDR(.high_startup) - KERNEL_MEM_OFFSET)
    {
        . = ALIGN(16);
        *(.high_startup_code)
    } > HIGH_TARTUP_CODE

    /* Contains the kernel code */
    . = ORIGIN(KERNEL_CODE);
    .text ALIGN(4K) : AT(ADDR(.text) - KERNEL_MEM_OFFSET)
    {      
        _START_TEXT_ADDR = .;

        *(.text)
        *(.text*)

        _END_TEXT_ADDR = .;
    } > KERNEL_CODE

    /* Contains the kernel read only data */
    . = ORIGIN(KERNEL_RO_DATA);
    .rodata ALIGN(4K) : AT(ADDR(.rodata) - KERNEL_MEM_OFFSET)
    {
        _START_RO_DATA_ADDR = .;

        *(.eh_frame)
        *(.rodata)
        *(.rodata*)

        _END_RODATA_ADDR = .;
    } > KERNEL_RO_DATA

    /* Contains the kernel data */
    . = ORIGIN(KERNEL_RW_DATA);
    .data ALIGN(4K) : AT(ADDR(.data) - KERNEL_MEM_OFFSET)
    {
        _START_DATA_ADDR = .; 

        *(.data)         
        *(.data*)

        _END_DATA_ADDR = .;           
    } > KERNEL_RW_DATA

    /* Contains the kernel BSS */
    .bss ALIGN(4K) : AT(ADDR(.bss) - KERNEL_MEM_OFFSET)
    {
        _START_BSS_ADDR = .; 

        *(COMMON)
        *(.bss)       

        _END_BSS_ADDR = .;
    } > KERNEL_RW_DATA
}

/* Symbols */
_KERNEL_STACKS_BASE = ORIGIN(KERNEL_STACKS);
_KERNEL_STACKS_SIZE = LENGTH(KERNEL_STACKS);

_KERNEL_HEAP_BASE  = ORIGIN(KERNEL_HEAP);
_KERNEL_HEAP_SIZE  = LENGTH(KERNEL_HEAP);