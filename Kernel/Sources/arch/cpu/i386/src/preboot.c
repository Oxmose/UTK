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

#include <stdint.h>    /* Generic int types */
#include <stddef.h>    /* Standard definitions */
#include <multiboot.h> /* Multiboot definitions */

/* UTK configuration file */
#include <config.h>

/* Tests header file */
#ifdef TEST_MODE_ENABLED
#include <test_bank.h>
#endif

/*******************************************************************************
 * CONSTANTS
 ******************************************************************************/

/** @brief VGA frame buffer base physical address. */
#define VGA_TEXT_FRAMEBUFFER 0xB8000

/*******************************************************************************
 * STRUCTURES
 ******************************************************************************/

/* None. */

/*******************************************************************************
 * GLOBAL VARIABLES
 ******************************************************************************/

static char hex_table[] =
     {'0','1','2','3','4','5','6','7','8','9','A','B','C','D','E','F'};

extern uintptr_t _kernel_multiboot_ptr;

extern uint8_t _KERNEL_MULTIBOOT_MEM_ADDR;

extern uint8_t _KERNEL_MULTIBOOT_MEM_SIZE;

extern uint8_t _KERNEL_INITRD_MEM_ADDR;

extern uint8_t _KERNEL_INITRD_MEM_SIZE;

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
    /* Fill buffer */
 	for (opos = 0; opos < top; --pos, ++opos)
    {
 		buf[opos] = tmp[pos];
    }

    /* Null termitate */
 	buf[opos] = 0;
}

static void copy_multiboot(void)
{
    struct multiboot_tag*        multiboot_tag;
    struct multiboot_tag_module* module_tag;

    uint32_t multiboot_info_size;
    uint8_t* copy_addr;
    uint8_t* src_addr;
    uint8_t* initrd_dst_addr;
    uint8_t* initrd_src_addr;
    uint8_t* module_start_addr;
    char     buff[32] = {0};
    uint32_t mem_size;
    uint32_t i;
    uint32_t entry_size;
    uint32_t save_addr;

    initrd_found = FALSE;

    mem_size  = (uint32_t)&_KERNEL_MULTIBOOT_MEM_SIZE;
    copy_addr = ((uint8_t*)&_KERNEL_MULTIBOOT_MEM_ADDR) - KERNEL_MEM_OFFSET;

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
    module_start_addr = (uint8_t*)((uintptr_t)&_KERNEL_MULTIBOOT_MEM_ADDR +
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
            module_tag = (struct multiboot_tag_module*)multiboot_tag;

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
                initrd_dst_addr = &_KERNEL_INITRD_MEM_ADDR - KERNEL_MEM_OFFSET;
                
                printf_vga("Loading INITRD from 0x", 22);   
                uitoa((uintptr_t)module_tag->mod_start, buff, 16);
                printf_vga(buff, 8);
                printf_vga(" at 0x", 6);  
                uitoa((uintptr_t)initrd_dst_addr, buff, 16);
                printf_vga(buff, 44); 

                initrd_found = TRUE;

                 /* Check bounds */
                if((size_t)&_KERNEL_INITRD_MEM_SIZE <
                   module_tag->mod_end - module_tag->mod_start)
                {
                    printf_vga(" ", 80);
                    printf_vga("ERROR: Allocated memory is for initrd is too "
                               "small", 80);
                    while(1);
                }

                /* Copy */
                while((uintptr_t)initrd_src_addr < module_tag->mod_end)
                {
                    *initrd_dst_addr = *initrd_src_addr;
                    ++initrd_dst_addr;
                    ++initrd_src_addr;
                }
            }
            printf_vga("Copy module 0x", 14);
            uitoa((uintptr_t)module_tag->mod_start, buff, 16);
            printf_vga(buff, 8);
            printf_vga(" -> 0x", 6);
            uitoa((uintptr_t)module_tag->mod_end, buff, 16);
            printf_vga(buff, 8);
            printf_vga(" to 0x", 6);
            uitoa((uintptr_t)module_start_addr, buff, 16);
            printf_vga(buff, 38);

            /* Check bounds and update size */
            if(mem_size < module_tag->mod_end - module_tag->mod_start)
            {
                printf_vga(" ", 80);
                printf_vga("ERROR: Allocated memory is smaller than "
                            "Multiboot modules", 80);
                while(1);
            }
            mem_size -= module_tag->mod_end - module_tag->mod_start;
            save_addr = (uint32_t)module_start_addr;

            /* Copy */
            while(module_tag->mod_start < module_tag->mod_end)
            {
                *module_start_addr = *((uint8_t*)module_tag->mod_start);
                ++module_start_addr;
                ++module_tag->mod_start;
            }

            /* Update the new value */
            module_tag->mod_start = save_addr;
            module_tag->mod_end = (uint32_t)module_start_addr;
           
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