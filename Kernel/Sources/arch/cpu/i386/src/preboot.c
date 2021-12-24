/*******************************************************************************
 * @file preboot.c
 *
 * @author Alexy Torres Aurora Dugo
 *
 * @date 28/03/2021
 *
 * @version 1.0
 *
 * @brief Kernel's pre boot C routines.
 *
 * @warning At this point interrupts should be disabled.
 *
 * @details Kernel's pre boot C routines. These function are to be executed
 * when paging is disabled. At this point the kernel is set with basic GRUB
 * IDT and GDT configuration.
 *
 * @copyright Alexy Torres Aurora Dugo
 ******************************************************************************/

/*******************************************************************************
 * INCLUDES
 ******************************************************************************/

/* Included headers */
#include <stdint.h>    /* Generic int types */
#include <stddef.h>    /* Standard definitions */
#include <multiboot.h> /* Multiboot definitions */

/* Configuration files */
#include <config.h>
#include <test_bank.h>

/* Header file */
/* None */

/*******************************************************************************
 * CONSTANTS
 ******************************************************************************/

/** @brief VGA frame buffer base physical address. */
#define VGA_TEXT_FRAMEBUFFER 0xB8000

#define SHT_SYMTAB 2
#define SHT_STRTAB 3

/*******************************************************************************
 * STRUCTURES AND TYPES
 ******************************************************************************/

/* TODO: Remove thins once the ELF loader is implemented */
typedef struct
{
    uint32_t sh_name;
    uint32_t sh_type;
    uint32_t sh_flags;
    uint32_t sh_addr;
    uint32_t sh_offset;
    uint32_t sh_size;
    uint32_t sh_link;
    uint32_t sh_info;
    uint32_t sh_addralign;
    uint32_t sh_entsize;
} elf_section_header_t;

/*******************************************************************************
 * MACROS
 ******************************************************************************/

/* None */

/*******************************************************************************
 * GLOBAL VARIABLES
 ******************************************************************************/

/************************* Imported global variables **************************/
extern uintptr_t _kernel_multiboot_ptr;

extern uint8_t* _KERNEL_MULTIBOOT_MEM_BASE;

extern uint8_t _KERNEL_MULTIBOOT_MEM_SIZE;

extern uint8_t* _KERNEL_INITRD_MEM_BASE;

extern uint8_t _KERNEL_INITRD_MEM_SIZE;



extern uintptr_t _KERNEL_SYMTAB_ADDR;
extern uintptr_t _KERNEL_SYMTAB_SIZE;
extern uintptr_t _KERNEL_STRTAB_ADDR;
extern uintptr_t _KERNEL_STRTAB_SIZE;

extern uint8_t* _KERNEL_SYMTAB_FREE_START;

/************************* Exported global variables **************************/
/* None */

/************************** Static global variables ***************************/
static char hex_table[] =
     {'0','1','2','3','4','5','6','7','8','9','A','B','C','D','E','F'};

static uint16_t* current_framebuffer_addr = (uint16_t*)VGA_TEXT_FRAMEBUFFER;

static bool_t initrd_found;

/*******************************************************************************
 * STATIC FUNCTIONS DECLARATIONS
 ******************************************************************************/
static void uitoa(uint32_t i, char* buf, uint32_t base);

static void copy_multiboot(void);

static void printf_vga(const char* str, const size_t size);

static void clear_vga(void);

static uint8_t cmp_str(const char* str1, const char* str2, uint32_t size);

/*******************************************************************************
 * FUNCTIONS
 ******************************************************************************/

/* This file should not be optimized */
#pragma GCC push_options
#pragma GCC optimize ("O0")

static uint8_t cmp_str(const char* str1, const char* str2, uint32_t size)
{
    while(size)
    {
        if(*str1 != *str2)
        {
            return 1;
        }
        ++str1;
        ++str2;
        --size;
    }

    return 0;
}

static void clear_vga(void)
{
    uint32_t i;
    for(i = 0; i < 80 * 24; ++i)
    {
        *((uint16_t*)VGA_TEXT_FRAMEBUFFER + i) = 0;
    }
}

static void printf_vga(const char* str, const size_t size)
{
    uint32_t i;
    for(i = 0; i < size && str[i] != 0; ++i)
    {
        *current_framebuffer_addr = str[i] | 0xF00;
        ++current_framebuffer_addr;
    }
    while(i < size)
    {
        *current_framebuffer_addr = ' ' | 0xF00;
        ++current_framebuffer_addr;
        ++i;
    }
}

static void uitoa(uint32_t i, char* buf, uint32_t base)
{

    char tmp[64] = {0};

    uint32_t pos  = 0;
 	uint32_t opos = 0;
 	uint32_t top  = 0;
    uint32_t idx;

 	if (i == 0 || base > 16)
    {
 		*buf++ = '0';
 		*buf = '\0';
 		return;
 	}

    /* Fill temp buffer */
 	while (i != 0)
    {
 		tmp[pos++] = hex_table[i % base];
 		i /= base;
 	}

 	top = pos--;
    idx = 0;
    /* Fill buffer */
    if(top < 8)
    {
        for (opos = 0; opos < 8 - top; ++opos)
        {
            buf[idx++] = '0';
        }
    }
 	for (opos = 0; opos < top; --pos, ++opos)
    {
 		buf[idx++] = tmp[pos];
    }

    /* Null termitate */
 	buf[idx] = 0;
}

static void copy_module(struct multiboot_tag_module* module_tag,
                        uint8_t** module_start_addr,
                        uint32_t* mem_size,
                        uint32_t* save_addr)
{
    uint8_t* initrd_dst_addr;
    uint8_t* initrd_src_addr;
    char     buff[32] = {0};

    /* Check if it is the initrd module */
    if(cmp_str(module_tag->cmdline, "initrd", module_tag->size - 16) ==
        0)
    {
        if(initrd_found == TRUE)
        {
            printf_vga(" ", 80);
            printf_vga("ERROR: Cannot load multiple INITRD", 80);
        }
        initrd_src_addr = (uint8_t*)module_tag->mod_start;
        initrd_dst_addr = (uint8_t*)&_KERNEL_INITRD_MEM_BASE -
                            KERNEL_MEM_OFFSET;

        printf_vga("Copy INITRD 0x", 14);
        uitoa((uintptr_t)module_tag->mod_start, buff, 16);
        printf_vga(buff, 8);
        printf_vga(" -> 0x", 6);
        uitoa((uintptr_t)module_tag->mod_end, buff, 16);
        printf_vga(buff, 8);
        printf_vga(" to 0x", 6);
        uitoa((uintptr_t)initrd_dst_addr, buff, 16);
        printf_vga(buff, 38);

        initrd_found = TRUE;

            /* Check bounds */
        if((size_t)&_KERNEL_INITRD_MEM_SIZE <
            module_tag->mod_end - module_tag->mod_start)
        {
            printf_vga(" ", 80);
            printf_vga("ERROR: Allocated memory for initrd is too "
                        "small", 80);
            while(1);
        }

        *mem_size -= module_tag->mod_end - module_tag->mod_start;
        *save_addr = (uint32_t)*module_start_addr;

        /* Copy */
        while((uintptr_t)initrd_src_addr < module_tag->mod_end)
        {
            *initrd_dst_addr = *initrd_src_addr;
            ++initrd_dst_addr;
            ++initrd_src_addr;
        }

        module_tag->mod_start = *save_addr;
        module_tag->mod_end = (uint32_t)*module_start_addr;
    }
    else
    {
        printf_vga("Copy module 0x", 14);
        uitoa((uintptr_t)module_tag->mod_start, buff, 16);
        printf_vga(buff, 8);
        printf_vga(" -> 0x", 8);
        uitoa((uintptr_t)module_tag->mod_end, buff, 16);
        printf_vga(buff, 8);
        printf_vga(" to 0x", 8);
        uitoa((uintptr_t)*module_start_addr, buff, 16);
        printf_vga(buff, 38);

        /* Check bounds and update size */
        if(*mem_size < module_tag->mod_end - module_tag->mod_start)
        {
            printf_vga(" ", 80);
            printf_vga("ERROR: Allocated memory is smaller than "
                        "Multiboot modules", 80);
            while(1);
        }
        *mem_size -= module_tag->mod_end - module_tag->mod_start;
        *save_addr = (uint32_t)*module_start_addr;

        /* Copy */
        while(module_tag->mod_start < module_tag->mod_end)
        {
            **module_start_addr = *((uint8_t*)module_tag->mod_start);
            ++*module_start_addr;
            ++module_tag->mod_start;
        }

        /* Update the new value */
        module_tag->mod_start = *save_addr;
        module_tag->mod_end = (uint32_t)*module_start_addr;
    }
}

static void copy_symbols(struct multiboot_tag_elf_sections* elf_tag)
{
    char buff[32] = {0};
    elf_section_header_t* header;
    uint32_t i;
    uint32_t j;
    uint8_t* copy_addr;
    uint8_t* src_addr;
    uint32_t symtab_link = 0;

    uint32_t* symtab_addr;
    uint32_t* symtab_size;
    uint32_t* strtab_addr;
    uint32_t* strtab_size;

    symtab_addr = (uint32_t*)
                    ((uintptr_t)&_KERNEL_SYMTAB_ADDR - KERNEL_MEM_OFFSET);
    symtab_size = (uint32_t*)
                    ((uintptr_t)&_KERNEL_SYMTAB_SIZE - KERNEL_MEM_OFFSET);
    strtab_addr = (uint32_t*)
                    ((uintptr_t)&_KERNEL_STRTAB_ADDR - KERNEL_MEM_OFFSET);
    strtab_size = (uint32_t*)
                    ((uintptr_t)&_KERNEL_STRTAB_SIZE - KERNEL_MEM_OFFSET);

    *symtab_addr = 0;
    *symtab_size = 0;
    *strtab_addr = 0;
    *strtab_size = 0;

    copy_addr = ((uint8_t*)&_KERNEL_SYMTAB_FREE_START) - KERNEL_MEM_OFFSET;
    for(i = 0; i < elf_tag->num; ++i)
    {
        header = ((elf_section_header_t*)elf_tag->sections) + i;
        /* We copy the symbols only */
        if(header->sh_type == SHT_SYMTAB)
        {
            symtab_link = header->sh_link;
            copy_addr = ((uint8_t*)&_KERNEL_SYMTAB_FREE_START) -
                        KERNEL_MEM_OFFSET;
            *symtab_addr = (uintptr_t)copy_addr;
            *symtab_size = *symtab_addr;
            src_addr = (uint8_t*)header->sh_addr;

            printf_vga("Copy symtab 0x", 14);
            uitoa((uintptr_t)src_addr, buff, 16);
            printf_vga(buff, 8);
            printf_vga(" -> 0x", 6);
            uitoa((uintptr_t)src_addr + header->sh_size, buff, 16);
            printf_vga(buff, 8);
            printf_vga(" to 0x", 6);
            uitoa((uintptr_t)copy_addr, buff, 16);
            printf_vga(buff, 38);

            /* Copy */
            for(j = 0; j < header->sh_size; ++j)
            {
                *copy_addr = *src_addr;
                ++copy_addr;
                ++src_addr;
            }
            *symtab_size = (uintptr_t)copy_addr - *symtab_size;

            *symtab_addr += KERNEL_MEM_OFFSET;

            break;
        }
    }

    /* Align table */
    if((uintptr_t)copy_addr % sizeof(uintptr_t) != 0)
    {
        copy_addr += sizeof(uintptr_t) -
                     ((uintptr_t)copy_addr % sizeof(uintptr_t));
    }

    /* Get the tring table */
    for(i = 0; i < elf_tag->num; ++i)
    {
        header = ((elf_section_header_t*)elf_tag->sections) + i;
        if(header->sh_type == SHT_STRTAB && i == symtab_link)
        {
            *strtab_addr = (uintptr_t)copy_addr;
            *strtab_size = *strtab_size;
            src_addr = (uint8_t*)header->sh_addr;

            printf_vga("Copy symbols 0x", 15);
            uitoa((uintptr_t)src_addr, buff, 16);
            printf_vga(buff, 8);
            printf_vga(" -> 0x", 6);
            uitoa((uintptr_t)src_addr + header->sh_size, buff, 16);
            printf_vga(buff, 8);
            printf_vga(" to 0x", 6);
            uitoa((uintptr_t)copy_addr, buff, 16);
            printf_vga(buff, 37);

            /* Copy */
            for(j = 0; j < header->sh_size; ++j)
            {
                *copy_addr = *src_addr;
                ++copy_addr;
                ++src_addr;
            }
            *strtab_size = (uintptr_t)copy_addr - *strtab_size;

            *strtab_addr += KERNEL_MEM_OFFSET;

            break;
        }
    }
}

static void copy_multiboot(void)
{
    struct multiboot_tag*        multiboot_tag;

    uint32_t multiboot_info_size;
    uint8_t* copy_addr;
    uint8_t* src_addr;

    uint8_t* module_start_addr;
    char     buff[32] = {0};
    uint32_t mem_size;
    uint32_t i;
    uint32_t entry_size;
    uint32_t save_addr;

    initrd_found = FALSE;

    mem_size  = (uint32_t)&_KERNEL_MULTIBOOT_MEM_SIZE;
    copy_addr = ((uint8_t*)&_KERNEL_MULTIBOOT_MEM_BASE) - KERNEL_MEM_OFFSET;

    /* Update the pointer to low half kernel */
    multiboot_tag = (struct multiboot_tag*)_kernel_multiboot_ptr;
    src_addr = (uint8_t*)multiboot_tag;

    uitoa((uintptr_t)multiboot_tag, buff, 16);
    printf_vga("Multiboot (P): 0x", 17);
    printf_vga(buff, 8);
    multiboot_info_size = *(uint32_t*)multiboot_tag;
    uitoa(multiboot_info_size, buff, 16);
    printf_vga(" Size: 0x", 9);
    printf_vga(buff, 46);

    printf_vga("Load (P): 0x", 12);
    uitoa((uintptr_t)copy_addr, buff, 16);
    printf_vga(buff, 8);
    printf_vga(" Size: 0x", 9);
    uitoa((uintptr_t)mem_size, buff, 16);
    printf_vga(buff, 8);

    /* Now parse structures */
    module_start_addr = (uint8_t*)((uintptr_t)&_KERNEL_MULTIBOOT_MEM_BASE +
                                    multiboot_info_size - KERNEL_MEM_OFFSET);
    module_start_addr = (uint8_t*)(((uintptr_t)module_start_addr + 7) & ~7);
    multiboot_tag = (struct multiboot_tag*)((uintptr_t)multiboot_tag + 8);

    uitoa((uintptr_t)module_start_addr, buff, 16);
    printf_vga(" Mod Load (P): 0x", 17);
    printf_vga(buff, 26);

    /* Check bounds */
    if(mem_size < multiboot_info_size)
    {
        printf_vga(" ", 80);
        printf_vga("ERROR: Allocated memory is smaller than Multiboot"
                   " structure", 80);
        while(1);
    }
    mem_size -= multiboot_info_size;

    /* Check if we need to copy modules */
    while((uintptr_t)multiboot_tag < (uintptr_t)src_addr + multiboot_info_size)
    {
        entry_size = ((multiboot_tag->size + 7) & ~7);
        if(multiboot_tag->type == MULTIBOOT_TAG_TYPE_MODULE)
        {
            copy_module((struct multiboot_tag_module*)multiboot_tag,
                        &module_start_addr, &mem_size, &save_addr);
        }
        else if(multiboot_tag->type == MULTIBOOT_TAG_TYPE_ELF_SECTIONS)
        {
            copy_symbols((struct multiboot_tag_elf_sections*)multiboot_tag);
        }
        multiboot_tag = (struct multiboot_tag*)
                        ((uintptr_t)multiboot_tag + entry_size);
    }

    /* Copy the entries */
    for(i = 0; i < multiboot_info_size; ++i)
    {
        copy_addr[i] = src_addr[i];
    }
}

void kernel_preboot(void)
{
    clear_vga();
    printf_vga("Kernel pre-boot v0.1", 80);

    /* Copy multiboot structure somewhere in the reserved kernel area */
    copy_multiboot();

    printf_vga(" ", 80);
    printf_vga("Copied Multiboot structures", 80);

    if(initrd_found == FALSE)
    {
        printf_vga(" ", 80);
        printf_vga("ERROR: Could not find init ram disk", 80);
        while(1);
    }
}

#pragma GCC pop_options

/************************************ EOF *************************************/