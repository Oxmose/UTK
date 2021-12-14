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
 * WARNING: Must be aligned on 4MB
 */
KERNEL_MEM_OFFSET = 0xE0000000;

/* Memory map */
MEMORY
{
    KERNEL_BIOS_CALL_MEM    (rwx)   :   ORIGIN = 0x00001000,    LENGTH = 4K
    LOW_STARTUP_CODE        (rx)    :   ORIGIN = 0x00100000,    LENGTH = 4K
    HIGH_STARTUP_CODE       (rx)    :   ORIGIN = 0xE0101000,    LENGTH = 4K
    KERNEL_CODE             (rx)    :   ORIGIN = 0xE0102000,    LENGTH = 256K
    KERNEL_RO_DATA          (r)     :   ORIGIN = 0xE0200000,    LENGTH = 64K
    KERNEL_RW_DATA          (rw)    :   ORIGIN = 0xE0210000,    LENGTH = 512K
    KERNEL_STACKS           (rw)    :   ORIGIN = 0xE0310000,    LENGTH = 32K
    KERNEL_HEAP             (rw)    :   ORIGIN = 0xE0318000,    LENGTH = 10M    
    KERNEL_MULTIBOOT_MEM    (rw)    :   ORIGIN = 0xE0D18000,    LENGTH = 64K
    KERNEL_INITRD_MEM       (rw)    :   ORIGIN = 0xE0D28000,    LENGTH = 64K
    KERNEL_RECUR_PG_TABLE   (rw)    :   ORIGIN = 0xFFC00000,    LENGTH = 4M 
    KERNEL_RECUR_PG_DIR     (rw)    :   ORIGIN = 0xFFFFF000,    LENGTH = 4K  
}

/* Memory layout */
SECTIONS
{
    /* Contains the startup code used by grub to bootstrap the kernel */
    . = ORIGIN(LOW_STARTUP_CODE);
    .low_startup ALIGN(4K) :
    {
        _START_LOW_STARTUP_ADDR = .;
        *(.multiboot)
        . = ALIGN(16);
        *(.low_startup_code)
        *preboot.o(.text .text.* .rodata .rodata* .data .data* .bss .eh_frame)
        _END_LOW_STARTUP_ADDR = .;
    } > LOW_STARTUP_CODE
    . = ORIGIN(HIGH_STARTUP_CODE);
    .high_startup ALIGN(4K) : AT(ADDR(.high_startup) - KERNEL_MEM_OFFSET)
    {
        . = ALIGN(16);
        _START_HIGH_STARTUP_ADDR = .;
        *(.high_startup_code)
        _END_HIGH_STARTUP_ADDR = .;
    } > HIGH_STARTUP_CODE

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

        *(.rodata)
        *(.rodata*)

    } > KERNEL_RO_DATA
    .eh_frame ALIGN(4K) : AT(ADDR(.eh_frame) - KERNEL_MEM_OFFSET)
    {
        *(.eh_frame)

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

    _KERNEL_STATIC_END = .;
}

/* Symbols */
_KERNEL_STACKS_BASE = ORIGIN(KERNEL_STACKS);
_KERNEL_STACKS_SIZE = LENGTH(KERNEL_STACKS);

_KERNEL_HEAP_BASE  = ORIGIN(KERNEL_HEAP);
_KERNEL_HEAP_SIZE  = LENGTH(KERNEL_HEAP);

_KERNEL_BIOS_MEMORY_BASE = ORIGIN(KERNEL_BIOS_CALL_MEM);
_KERNEL_BIOS_MEMORY_SIZE = LENGTH(KERNEL_BIOS_CALL_MEM);

_KERNEL_MULTIBOOT_MEM_ADDR = ORIGIN(KERNEL_MULTIBOOT_MEM);
_KERNEL_MULTIBOOT_MEM_SIZE = LENGTH(KERNEL_MULTIBOOT_MEM);

_KERNEL_INITRD_MEM_ADDR = ORIGIN(KERNEL_INITRD_MEM);
_KERNEL_INITRD_MEM_SIZE = LENGTH(KERNEL_INITRD_MEM);

_KERNEL_MEMORY_END = _KERNEL_INITRD_MEM_ADDR + _KERNEL_INITRD_MEM_SIZE;

_KERNEL_RECUR_PG_TABLE_BASE = ORIGIN(KERNEL_RECUR_PG_TABLE);
_KERNEL_RECUR_PG_DIR_BASE   = ORIGIN(KERNEL_RECUR_PG_DIR);