ENTRY(__kinit)  /* Entry label */
SECTIONS
{
    /* Kernel starts at 1M in physical addresse space
     * Add the offset, to fit with values in config.h AND kinit.S
     */
    KERNEL_MEM_OFFSET = 0xE0000000;

    _kernel_start_phys = . + 0x00100000;

    . = KERNEL_MEM_OFFSET;
    _kernel_start = .;
    . = KERNEL_MEM_OFFSET + _kernel_start_phys;

    /* Put the multiboot header */
    .multiboot ALIGN(4K) : AT(ADDR(.multiboot) - KERNEL_MEM_OFFSET)
    {
        *(.multiboot)
    }

    /* Executable code */
    .text ALIGN(4K) : AT(ADDR(.text) - KERNEL_MEM_OFFSET)
    {
        _kernel_code_start = .;
        *(.text*)
        _kernel_code_end = .;
    }

    /* Read-only data. */
    .rodata ALIGN(4K) : AT(ADDR(.rodata) - KERNEL_MEM_OFFSET)
    {
        _kernel_rodata_start = .;
        *(.eh_frame)
        *(.rodata*)
        _kernel_rodata_end = .;
    }

    /* Read-write data */
    .data ALIGN(4K) : AT(ADDR(.data) - KERNEL_MEM_OFFSET)
    {
        _kernel_data_start = .;
        *(.data)
        _kernel_data_end = .;
    }

    /* Read-write bss */
    .bss ALIGN(4K) : AT(ADDR(.bss) - KERNEL_MEM_OFFSET)
    {
        _kernel_bss_start = .;
        *(COMMON)
        *(.bss)
        _kernel_bss_end = .;
    }

    /* Kernel configuration structures */
    .kernel_struct ALIGN(4K) : AT(ADDR(.kernel_struct) - KERNEL_MEM_OFFSET)
    {
        _kernel_struct_start = .;
        *(.conf_data)
        _kernel_struct_end = .;
    }
    _kernel_static_limit = .;

    /* Kernel heap */
    . = ALIGN(4K);
    /* Beginning of kernel memory heap */
    _kernel_heap_start = .;
    /* End of kernel memory heap */
    _kernel_heap_end = _kernel_heap_start + 0x00800000;
    . = _kernel_heap_end;
    _kernel_end = .;

    /**********************************************
     * SPECIFIC DATA STRUCTURES BASE ADDRESS
     *********************************************/

    /* VESA Structures, must be placed in lower memory */
    vbe_info_base      = 0x2000; /* SIZE = 4096bits, ends at 0x3000 */
    vbe_mode_info_base = 0x3000; /* SIZE = 2048bits, ends at 0x3800 */

    /* Init AP relocated code */
    init_ap_code = 0x4000; /* SIZE = 2048bits, ends at 0x4800, also define in kinit.S */

    /* Bios calls */
    bios_call_memory = 0x7000; /* 4096bits, ends at 0x8000 */
}