/*******************************************************************************
 * @file vesa.c
 *
 * @see vesa.h
 *
 * @author Alexy Torres Aurora Dugo
 *
 * @date 03/01/2018
 *
 * @version 1.5
 *
 * @brief VESA VBE 2 graphic driver.
 *
 * @details VESA VBE 2 graphic drivers. Allows the kernel to have a generic high
 * resolution output. The driver provides regular console output management and
 * generic screen drawing functions.
 *
 * @copyright Alexy Torres Aurora Dugo
 ******************************************************************************/

#include <lib/stdint.h>       /* Generic int types */
#include <lib/stddef.h>       /* Standard definitions */
#include <lib/string.h>       /* String manipulation */
#include <cpu.h>              /* CPU management  */
#include <bios_call.h>        /* BIOS call driver */
#include <fonts/uni_vga.h>    /* Kernel font */
#include <vga_text.h>         /* VGA driver */
#include <serial.h>           /* Serial driver */
#include <io/graphic.h>       /* GRaphic API */
#include <io/kernel_output.h> /* Kernel output methods */
#include <memory/paging.h>    /* Memory management */
#include <memory/memalloc.h>  /* Memory allocation */
#include <memory/kheap.h>     /* Kernel heap */
#include <arch_paging.h>      /* Architecture specific memory settings */
#include <core/scheduler.h>   /* Kernel scheduler */

/* UTK configuration file */
#include <config.h>

/* Header file */
#include <vesa.h>

/*******************************************************************************
 * GLOBAL VARIABLES
 ******************************************************************************/

/* VESA data structures */
/** @brief BIOS VBE information structure base address. */
extern vbe_info_structure_t      vbe_info_base;
/** @brief BIOS VBE mode information structure base address. */
extern vbe_mode_info_structure_t vbe_mode_info_base;

/* VESA modes */
/** @brief Available VESA modes. */
static vesa_mode_t* saved_modes;
/** @brief Currently selected VESA mode. */
static vesa_mode_t* current_mode;
/** @brief Number of available VESA modes. */
static uint16_t     mode_count     = 0;
/** @brief Tells if the VESA mode is supported (1) or not (0). */
static uint8_t      vesa_supported = 0;

/* VESA console settigns, used in tty mode */
/** @brief Current screen's cursor. */
static cursor_t      screen_cursor;
/** @brief Cursor of the last printed character. */
static cursor_t      last_printed_cursor;
/** @brief Current screen color scheme. */
static colorscheme_t screen_scheme;
/** @brief Stores the last printed character's column for each screen line. */
static uint32_t*     last_columns;

/** @brief VGA color to RGB translation table. */
static const uint32_t vga_color_table[16] = {
    0xFF000000,
    0xFF0000AA,
    0xFF00AA00,
    0xFF00AAAA,
    0xFFAA0000,
    0xFFAA00AA,
    0xFFAA5500,
    0xFFAAAAAA,
    0xFF555555,
    0xFF5555FF,
    0xFF55FF55,
    0xFF55FFFF,
    0xFFFF5555,
    0xFFFF55FF,
    0xFFFFFF55,
    0xFFFFFFFF
};

/**
 * @brief VESA text driver instance.
 */
kernel_graphic_driver_t vesa_driver = {
    .clear_screen = vesa_clear_screen,
    .put_cursor_at = vesa_put_cursor_at,
    .save_cursor = vesa_save_cursor,
    .restore_cursor = vesa_restore_cursor,
    .scroll = vesa_scroll,
    .set_color_scheme = vesa_set_color_scheme,
    .save_color_scheme = vesa_save_color_scheme,
    .put_string = vesa_put_string,
    .put_char = vesa_put_char,
    .console_write_keyboard = vesa_console_write_keyboard
};

/** @brief Sets if the characters background should be of the screen colorscheme
 * or transparent.
 */
static uint32_t transparent_char = 0;

/** @brief Buffer that saves the background of the cursor to restore it next
 * time the cursor moves.
 */
static uint8_t save_buff[256] = {0};

/** @brief Virtual framebuffer used for double-buffering */
static uint8_t* virt_buffer = NULL;

/*******************************************************************************
 * FUNCTIONS
 ******************************************************************************/

static void fast_memset(void* dst, const uint8_t value, uint32_t size)
{
    uint8_t *q = dst;

#if defined(__i386__)
        size_t nl = size >> 2;
        __asm__ __volatile__ ("cld ; rep ; stosl ; movl %3,%0 ; rep ; stosb"
                : "+c" (nl), "+D" (q)
                : "a" ((unsigned char)value * 0x01010101U), "r" (size & 3));
    #elif defined(__x86_64__)
        size_t nq = size >> 3;
        __asm__ __volatile__ ("cld ; rep ; stosq ; movl %3,%%ecx ; rep ; stosb"
                :"+c" (nq), "+D" (q)
                : "a" ((unsigned char)value * 0x0101010101010101U),
                "r" ((unsigned int) size & 7));
#else
        while (n--) {
            *q++ = c;
        }
#endif
}

static void fast_memcpy(void* dst, const void* src, const uint32_t size)
{
    uint32_t i;
    uint32_t n;

    /* Check SSE support */
    if(cpu_is_sse_enabled())
    {
        for(i = 0; i < size / 16; ++i)
        {
            /* Copy by 16bytes chunks */
            __asm__ __volatile__(
                "movups  (%0), %%xmm0\n\t"
                "movntdq %%xmm0, (%1)\n\t"
            ::"r"(src), "r"(dst) : "memory");
            src = (void*)((uintptr_t)src + 16);
            dst = (void*)((uintptr_t)dst + 16);
        }

        /* If some rests */
        if(size % 0x7)
        {
            n = size;
#if defined(__i386__)
            size_t nl = n >> 2;
            __asm__ __volatile__ ("cld ; rep ; movsl ; movl %3,%0 ; rep ; movsb":"+c" (nl),
                    "+S"(src), "+D"(dst)
                    :"r"(n & 3));
#elif defined(__x86_64__)
            size_t nq = n >> 3;
            __asm__ __volatile__ ("cld ; rep ; movsq ; movl %3,%%ecx ; rep ; movsb":"+c"
                    (nq), "+S"(src), "+D"(dst)
                    :"r"((uint32_t) (n & 7)));
#else
            while (n--) {
                *dst++ = *src++;
            }
#endif
        }
    }
    else 
    {
        n = size;
#if defined(__i386__)
        size_t nl = n >> 2;
        __asm__ __volatile__ ("cld ; rep ; movsl ; movl %3,%0 ; rep ; movsb":"+c" (nl),
                "+S"(src), "+D"(dst)
                :"r"(n & 3));
#elif defined(__x86_64__)
        size_t nq = n >> 3;
        __asm__ __volatile__ ("cld ; rep ; movsq ; movl %3,%%ecx ; rep ; movsb":"+c"
                (nq), "+S"(src), "+D"(dst)
                :"r"((uint32_t) (n & 7)));
#else
        while (n--) {
            *dst++ = *src++;
        }
#endif
    }
}

/**
 * @brief Processes the character in parameters.
 *
 * @details Check the character nature and code. Corresponding to the
 * character's code, an action is taken. A regular character will be printed
 * whereas \n will create a line feed.
 *
 * @param[in] character The character to process.
 */
static void vesa_process_char(const char character)
{
    uint32_t i;
    uint32_t j;

#if (KERNEL_DEBUG == 1) | (TEST_MODE_ENABLED == 1)
    /* Write on serial */
    serial_write(COM1, character);
#endif

    /* If character is a normal ASCII character */
    if(character > 31 && character < 127)
    {
        /* Manage end of line cursor position */
        if(screen_cursor.x + font_width >= current_mode->width)
        {
            /* remove cursor */
            for(i = screen_cursor.x; i < current_mode->width; ++i)
            {
                for(j = screen_cursor.y; j < screen_cursor.y + font_height; ++j)
                {
                    vesa_draw_pixel(i, j,
                               (screen_scheme.background & 0xFF000000) >> 24,
                               (screen_scheme.background & 0x00FF0000) >> 16,
                               (screen_scheme.background & 0x0000FF00) >> 8,
                               (screen_scheme.background & 0x000000FF));
                }
            }
            vesa_put_cursor_at(screen_cursor.y + font_height, 0);
            last_columns[(screen_cursor.y / font_height)] = screen_cursor.x;
        }

        /* Manage end of screen cursor position */
        if(screen_cursor.y > current_mode->height - font_height)
        {
            vesa_scroll(SCROLL_DOWN, 1);
        }

        /* Update cursor position */
        vesa_put_cursor_at(screen_cursor.y, screen_cursor.x + font_width);

        /* Display character */
        vesa_drawchar(character, screen_cursor.x - font_width, screen_cursor.y,
                      screen_scheme.foreground,
                      transparent_char ? 0 : screen_scheme.background);

        /* Manage end of line cursor position */
        if(screen_cursor.x + font_width >= current_mode->width)
        {
            /* remove cursor */
            for(i = screen_cursor.x; i < current_mode->width; ++i)
            {
                for(j = screen_cursor.y; j < screen_cursor.y + font_height; ++j)
                {
                    vesa_draw_pixel(i, j,
                                (screen_scheme.background & 0xFF000000) >> 24,
                                (screen_scheme.background & 0x00FF0000) >> 16,
                                (screen_scheme.background & 0x0000FF00) >> 8,
                                (screen_scheme.background & 0x000000FF));
                }
            }
            vesa_put_cursor_at(screen_cursor.y + font_height, 0);
        }
        last_columns[(screen_cursor.y / font_height)] = screen_cursor.x;
    }
    else
    {
        int32_t diff;

        /* Manage special ACSII characters*/
        switch(character)
        {
            /* Backspace */
            case '\b':
                if(last_printed_cursor.y == screen_cursor.y)
                {
                    if(screen_cursor.x > last_printed_cursor.x)
                    {
                        vesa_drawchar(' ', screen_cursor.x, screen_cursor.y,
                                      screen_scheme.foreground,
                                      transparent_char ? 0 : screen_scheme.background);
                        vesa_drawchar(' ',
                                screen_cursor.x - font_width, screen_cursor.y,
                                screen_scheme.foreground,
                                transparent_char ? 0 : screen_scheme.background);
                        vesa_put_cursor_at(screen_cursor.y,
                                           screen_cursor.x - font_width);
                        last_columns[(screen_cursor.y / font_height)] =
                                                                screen_cursor.x;
                    }
                }
                else if(last_printed_cursor.y < screen_cursor.y)
                {
                    if(screen_cursor.x > 0)
                    {
                        vesa_drawchar(' ', screen_cursor.x, screen_cursor.y,
                                      screen_scheme.foreground,
                                      transparent_char ? 0 : screen_scheme.background);
                        vesa_drawchar(' ',
                                screen_cursor.x - font_width, screen_cursor.y,
                                screen_scheme.foreground,
                                transparent_char ? 0 : screen_scheme.background);
                        vesa_put_cursor_at(screen_cursor.y,
                                           screen_cursor.x - font_width);
                        last_columns[(screen_cursor.y / font_height)] =
                                                                screen_cursor.x;

                    }
                    else
                    {
                        vesa_drawchar(' ', screen_cursor.x, screen_cursor.y,
                                      screen_scheme.foreground,
                                      transparent_char ? 0 : screen_scheme.background);
                        vesa_drawchar(' ',
                            last_columns[(screen_cursor.y / font_height) - 1],
                            screen_cursor.y - font_height,
                            screen_scheme.foreground,
                            transparent_char ? 0 : screen_scheme.background);
                        vesa_put_cursor_at(screen_cursor.y - font_height,
                            last_columns[(screen_cursor.y / font_height) - 1]);
                    }
                }
                break;
            /* Tab */
            case '\t':
                diff = current_mode->width -
                       (screen_cursor.x + TAB_WIDTH * font_width);

                if(diff < 0)
                {
                    diff = TAB_WIDTH + (diff / font_width);
                }
                else
                {
                    diff = TAB_WIDTH;
                }
                while(diff-- > 0)
                {
                    vesa_process_char(' ');
                }
                last_columns[(screen_cursor.y / font_height)] = screen_cursor.x;
                break;
            /* Line feed */
            case '\n':

                /* remove cursor */
                for(i = screen_cursor.x; i < current_mode->width; ++i)
                {
                    for(j = screen_cursor.y;
                        j < screen_cursor.y + font_height;
                        ++j)
                    {
                        vesa_draw_pixel(i, j,
                                (screen_scheme.background & 0xFF000000) >> 24,
                                (screen_scheme.background & 0x00FF0000) >> 16,
                                (screen_scheme.background & 0x0000FF00) >> 8,
                                (screen_scheme.background & 0x000000FF));
                    }
                }
                last_columns[(screen_cursor.y / font_height)] = screen_cursor.x;
                if(screen_cursor.y + font_height <=
                   current_mode->height - font_height)
                {
                    /* remove cursor */
                    for(i = screen_cursor.x;
                        i < current_mode->width &&
                        i < screen_cursor.x + font_width;
                         ++i)
                    {
                        for(j = screen_cursor.y;
                            j < screen_cursor.y + font_height;
                            ++j)
                        {
                            vesa_draw_pixel(i, j,
                                (screen_scheme.background & 0xFF000000) >> 24,
                                (screen_scheme.background & 0x00FF0000) >> 16,
                                (screen_scheme.background & 0x0000FF00) >> 8,
                                (screen_scheme.background & 0x000000FF));
                        }
                    }
                    vesa_put_cursor_at(screen_cursor.y + font_height, 0);
                    last_columns[(screen_cursor.y / font_height)] =
                                                                screen_cursor.x;
                }
                else
                {
                    vesa_scroll(SCROLL_DOWN, 1);
                }
                break;
            /* Clear screen */
            case '\f':
                vesa_clear_screen();
                break;
            /* Line return */
            case '\r':
                vesa_put_cursor_at(screen_cursor.y, 0);
                last_columns[(screen_cursor.y / font_height)] = screen_cursor.x;
                break;
            /* Undefined */
            default:
                break;
        }
    }
}

OS_RETURN_E vesa_init(void)
{
    bios_int_regs_t regs;
    uint32_t        i;
    uint16_t*       modes;
    vesa_mode_t*    new_mode;

    OS_RETURN_E err;

#if VESA_KERNEL_DEBUG == 1
    kernel_serial_debug("VESA Initialization start\n");
#endif

    /* Init data */
    mode_count     = 0;
    vesa_supported = 0;
    current_mode   = NULL;
    saved_modes    = NULL;

    /* Console mode init */
    screen_cursor.x = 0;
    screen_cursor.y = 0;
    screen_scheme.foreground = 0xFFFFFFFF;
    screen_scheme.background = 0xFF000000;
    screen_scheme.vga_color  = 0;

    /* Mapp memory */
    err = kernel_mmap_hw(&vbe_info_base, &vbe_info_base, 0x1000, 0, 0);
    if(err != OS_NO_ERR)
    {
        return err;
    }
    err = kernel_mmap_hw(&vbe_mode_info_base, 
                         &vbe_mode_info_base, 0x1000, 0, 0); 
    if(err != OS_NO_ERR)
    {
        return err;
    }

    /* Init structure */
    strncpy(vbe_info_base.signature, "VBE2", 4);

     /* Init the registers for the bios call */
    regs.ax = BIOS_CALL_GET_VESA_INFO;
    regs.es = 0;
    regs.di = (uint16_t)(uintptr_t)(&vbe_info_base);

    /* Issue call */
    bios_call(BIOS_INTERRUPT_VESA, &regs);

    /* Check call result */
    if(regs.ax != 0x004F || strncmp(vbe_info_base.signature, "VESA", 4) != 0)
    {
        return OS_ERR_VESA_NOT_SUPPORTED;
    }

    /* Get modes */
    modes = (uint16_t*)(uintptr_t)vbe_info_base.video_modes;
    for (i = 0 ; mode_count < MAX_VESA_MODE_COUNT && modes[i] != 0xFFFF ; ++i)
    {
        /* Prepare registers for mode query call */
        regs.ax = BIOS_CALL_GET_VESA_MODE;
        regs.cx = modes[i];
        regs.es = 0;
        regs.di = (uint16_t)(uintptr_t)(&vbe_mode_info_base);

        bios_call(BIOS_INTERRUPT_VESA, &regs);

        /* Check call result */
        if(regs.ax != 0x004F)
        {
            continue;
        }

        /* The driver only support linear frame buffer management */
        if ((vbe_mode_info_base.attributes & VESA_FLAG_LINEAR_FB) !=
            VESA_FLAG_LINEAR_FB)
        {
            continue;
        }

        /* Check if this is a packed pixel or direct color mode */
        if (vbe_mode_info_base.memory_model != 4 &&
            vbe_mode_info_base.memory_model != 6 )
        {
            continue;
        }

        new_mode = kmalloc(sizeof(vesa_mode_t));
        if(new_mode == NULL)
        {
            continue;
        }

        /* The mode is compatible, save it */
        new_mode->width           = vbe_mode_info_base.width;
        new_mode->height          = vbe_mode_info_base.height;
        new_mode->bpp             = vbe_mode_info_base.bpp;
        new_mode->mode_id         = modes[i];
        new_mode->framebuffer_phy = (void*)(uintptr_t)vbe_mode_info_base.framebuffer;

        /* Save mode in list */
        new_mode->next = saved_modes;
        saved_modes = new_mode;

        ++mode_count;
    }

    /* Unmap memory */
    kernel_munmap(&vbe_info_base, 0x1000); 
    kernel_munmap(&vbe_mode_info_base, 0x1000); 

    if(mode_count > 0)
    {
        vesa_supported = 1;
    }
    else
    {
        return OS_ERR_VESA_NOT_SUPPORTED;
    }

#if VESA_KERNEL_DEBUG == 1
    kernel_serial_debug("VESA Initialization end\n");
#endif

    return OS_NO_ERR;
}

OS_RETURN_E vesa_text_vga_to_vesa(void)
{
    OS_RETURN_E       err;
    uint32_t          i;
    uint32_t          j;
    vesa_mode_info_t  selected_mode;
    vesa_mode_t*      cursor;
    uint16_t*         vga_fb;
    cursor_t          vga_cursor;
    uint16_t          temp_buffer[VGA_TEXT_SCREEN_LINE_SIZE *
                                  VGA_TEXT_SCREEN_COL_SIZE];
    colorscheme_t     new_colorscheme;
    colorscheme_t     old_colorscheme = screen_scheme;

    /* Save VGA content */
    vga_save_cursor(&vga_cursor);
    vga_fb = vga_get_framebuffer(0, 0);
    fast_memcpy(temp_buffer, vga_fb,
           sizeof(uint16_t) *
           VGA_TEXT_SCREEN_LINE_SIZE * VGA_TEXT_SCREEN_COL_SIZE);

    /* Set VESA mode */
    if(vesa_supported == 0 || mode_count == 0)
    {
        return OS_ERR_VESA_NOT_SUPPORTED;
    }

    selected_mode.width = 0;
    selected_mode.height = 0;
    selected_mode.bpp = 0;

    cursor = saved_modes;
    while(cursor != NULL)
    {
        if(cursor->width > MAX_SUPPORTED_WIDTH ||
           cursor->height > MAX_SUPPORTED_HEIGHT ||
           cursor->bpp > MAX_SUPPORTED_BPP)
        {
            cursor = cursor->next;
            continue;
        }

        if(cursor->width >= selected_mode.width &&
           cursor->height >= selected_mode.height &&
           cursor->bpp  >= selected_mode.bpp)
        {
            selected_mode.width = cursor->width;
            selected_mode.height = cursor->height;
            selected_mode.bpp = cursor->bpp;
            selected_mode.mode_id = cursor->mode_id;
        }
        cursor = cursor->next;
    }
#if VESA_KERNEL_DEBUG
    kernel_serial_debug("Selected VESA mode %dx%d %dbits\n",
                                                  selected_mode.width,
                                                  selected_mode.height,
                                                  selected_mode.bpp);
#endif

    err = vesa_set_vesa_mode(selected_mode);
    if(err != OS_NO_ERR)
    {
        return err;
    }

    vesa_clear_screen();


    vga_fb = temp_buffer;

    /* Browse the framebuffer to get the cahracters and the colorscheme */
    for(i = 0; i < VGA_TEXT_SCREEN_LINE_SIZE; ++i)
    {
        for(j = 0; j < VGA_TEXT_SCREEN_COL_SIZE; ++j)
        {
            if(vga_cursor.y < i)
            {
                break;
            }
            else if(vga_cursor.y == i && vga_cursor.x == j)
            {
                break;
            }

            /* Get data character */
            char character = (*vga_fb) & 0x00FF;
            new_colorscheme.foreground = ((*vga_fb) & 0x0F00) >> 8;
            new_colorscheme.background = ((*vga_fb) & 0xF000) >> 12;
            new_colorscheme.vga_color = 1;

            /* Print char */
            vesa_set_color_scheme(new_colorscheme);

            vesa_process_char(character);

            ++vga_fb;
        }
        if(vga_cursor.y == i)
        {
            break;
        }
        vesa_process_char('\n');
    }

    /* Restore previous screen scheme */
    screen_scheme = old_colorscheme;

#if VESA_KERNEL_DEBUG == 1
    kernel_serial_debug("VESA VGA Text to VESA\n");
#endif

    return OS_NO_ERR;
}

uint16_t vesa_get_vesa_mode_count(void)
{
    return mode_count;
}

OS_RETURN_E vesa_get_vesa_modes(vesa_mode_info_t* buffer, const uint32_t size)
{
    uint32_t i;
    vesa_mode_t* cursor;

    if(vesa_supported == 0 || mode_count == 0)
    {
        return OS_ERR_VESA_NOT_SUPPORTED;
    }

    if(buffer == NULL)
    {
        return OS_ERR_NULL_POINTER;
    }

    i = 0;
    cursor = saved_modes;
    while(cursor != NULL && i < size)
    {
        buffer[i].width       = cursor->width;
        buffer[i].height      = cursor->height;
        buffer[i].bpp         = cursor->bpp;
        buffer[i].mode_id     = cursor->mode_id;
        cursor = cursor->next;
        ++i;
    }

    return OS_NO_ERR;
}

OS_RETURN_E vesa_set_vesa_mode(const vesa_mode_info_t mode)
{
    bios_int_regs_t regs;
    uint32_t        last_columns_size;
    vesa_mode_t*    cursor;
    OS_RETURN_E     err;
    uint32_t        page_count;
    uint32_t        buffer_size;
    size_t          current_buffer_size;
    size_t          current_buffer_page_count;

#if DISPLAY_TYPE == DISPLAY_VESA_BUF  
    void*           virt_buffer_frames;
#endif

    if(vesa_supported == 0)
    {
        return OS_ERR_VESA_NOT_SUPPORTED;
    }

    err = OS_NO_ERR;

    /* Search for the mode in the saved modes */
    cursor = saved_modes;
    while(cursor != NULL)
    {
        if(cursor->mode_id == mode.mode_id &&
           cursor->width == mode.width     &&
           cursor->height == mode.height   &&
           cursor->bpp == mode.bpp)
        {
            break;
        }
        cursor = cursor->next;
    }

    /* The mode was not found */
    if(cursor == NULL)
    {
        return OS_ERR_VESA_MODE_NOT_SUPPORTED;
    }

    if(cursor->bpp == 24)
    {
        cursor->bpp = 32;
    }

    buffer_size = cursor->width *
                  cursor->height *
                  (cursor->bpp / 8);
    page_count  = buffer_size / KERNEL_PAGE_SIZE;
    if(buffer_size % KERNEL_PAGE_SIZE != 0)
    {
        ++page_count;
    }

    if(current_mode != NULL)
    {
        current_buffer_size = current_mode->width *
                                current_mode->height *
                                (current_mode->bpp / 8);        
        current_buffer_page_count = current_buffer_size / KERNEL_PAGE_SIZE;
        if(current_buffer_size % KERNEL_PAGE_SIZE != 0)
        {
            ++current_buffer_page_count;
        }
    }

    /* Get a new pages */
    if(current_mode != NULL && current_mode->framebuffer != NULL)
    {
        memalloc_free_kpages(cursor->framebuffer, page_count);
    }
    cursor->framebuffer = memalloc_alloc_kpages(page_count, &err);
    if(cursor->framebuffer == 0 || err != OS_NO_ERR)
    {
        return err;
    }

#if DISPLAY_TYPE == DISPLAY_VESA_BUF
    /* Dealocate old virtual buffer */
    if(current_mode != NULL)
    {
        memalloc_free_kpages(virt_buffer, current_buffer_page_count);
    }

    /* Allocate the virtual buffer, aligned on 16Bytes*/

    virt_buffer = memalloc_alloc_kpages(page_count, &err);
    if(err != OS_NO_ERR)
    {
        memalloc_free_kpages(cursor->framebuffer, page_count);
        cursor->framebuffer = NULL;
        return err;
    }
    err = kernel_mmap(virt_buffer, page_count * KERNEL_PAGE_SIZE,
                      0, 0);
    if(err != OS_NO_ERR)
    {
        memalloc_free_kpages(cursor->framebuffer, page_count);
        memalloc_free_kpages(virt_buffer, page_count);
        cursor->framebuffer = NULL;
        virt_buffer = NULL;
        return err;
    }
#else 
    virt_buffer = (uint8_t*)cursor->framebuffer;
#endif 

    /* Mmap the new buffer */
    if(cursor->bpp == 24)
    {
        cursor->bpp = 32;
    }
    err = kernel_mmap_hw((void*)cursor->framebuffer, (void*)cursor->framebuffer_phy,
                         cursor->width * cursor->height * (cursor->bpp / 8),
                         0, 0);
    if(err != OS_NO_ERR)
    {
        memalloc_free_kpages(cursor->framebuffer, page_count);
        cursor->framebuffer = NULL;
#if DISPLAY_TYPE == DISPLAY_VESA_BUF
        memalloc_free_kpages(virt_buffer, page_count);
        virt_buffer = NULL;
#endif
        return err;
    }

    /* Set the last collumn array */
    last_columns_size = sizeof(uint32_t) * cursor->height / font_height;
    if(last_columns != NULL)
    {
        kfree(last_columns);
    }
    last_columns = kmalloc(last_columns_size);
    if(last_columns == NULL)
    {
        memalloc_free_kpages(cursor->framebuffer, page_count);
        cursor->framebuffer = NULL;
#if DISPLAY_TYPE == DISPLAY_VESA_BUF
        memalloc_free_kpages(virt_buffer, page_count);
        virt_buffer = NULL;
#endif
        return OS_ERR_MALLOC;
    }
    fast_memset(last_columns, 0, last_columns_size);

    /* Set the VESA mode */
    regs.ax = BIOS_CALL_SET_VESA_MODE;
    regs.bx = cursor->mode_id | VESA_FLAG_LFB_ENABLE;
    bios_call(BIOS_INTERRUPT_VESA, &regs);

    /* Check call result */
    if(regs.ax != 0x004F)
    {
        memalloc_free_kpages(cursor->framebuffer, page_count);
        cursor->framebuffer = NULL;
        kfree(last_columns);
        last_columns = NULL;
#if DISPLAY_TYPE == DISPLAY_VESA_BUF
        memalloc_free_kpages(virt_buffer, page_count);
        virt_buffer = NULL;
#endif
        return OS_ERR_VESA_MODE_NOT_SUPPORTED;
    }
    /* Tell generic driver we loaded a VESA mode, ID mapped */
    graphic_set_selected_driver(&vesa_driver);

    current_mode = cursor;

    if(err != OS_NO_ERR)
    {
        memalloc_free_kpages(cursor->framebuffer, page_count);
        cursor->framebuffer = NULL;
        kfree(last_columns);
        last_columns = NULL;
#if DISPLAY_TYPE == DISPLAY_VESA_BUF
        memalloc_free_kpages(virt_buffer, page_count);
        virt_buffer = NULL;
#endif
    }

#if VESA_KERNEL_DEBUG == 1
    kernel_serial_debug("VESA Mode set %d (%d)\n", mode.mode_id, err);
#endif

    return err;
}

OS_RETURN_E vesa_get_pixel(const uint16_t x, const uint16_t y,
                            uint8_t* alpha, uint8_t* red,
                            uint8_t* green, uint8_t* blue)
{
    uint8_t* addr;

    if(alpha == NULL || red == NULL || green == NULL || blue == NULL)
    {
        return OS_ERR_NULL_POINTER;
    }

    if(vesa_supported == 0)
    {
        return OS_ERR_VESA_NOT_SUPPORTED;
    }

    if(current_mode == NULL)
    {
        return OS_ERR_VESA_NOT_INIT;
    }

    if(x > current_mode->width || y > current_mode->height)
    {
        return OS_ERR_OUT_OF_BOUND;
    }

    /* Get framebuffer address */
    addr = (uint8_t*)(((uint32_t*)virt_buffer) +
                        (current_mode->width * y) + x);

    *blue  = *(addr++);
    *green = *(addr++);
    *red   = *(addr++);
    *alpha = 0xFF;

    return OS_NO_ERR;
}

__inline__ OS_RETURN_E vesa_draw_pixel(const uint16_t x, const uint16_t y,
                                       const uint8_t alpha, const uint8_t red,
                                       const uint8_t green, const uint8_t blue)
{
    uint32_t* addr;
    uint8_t   pixel[4] = {0};
    uint8_t*  back;

    if(vesa_supported == 0)
    {
        return OS_ERR_VESA_NOT_SUPPORTED;
    }

    if(current_mode == NULL)
    {
        return OS_ERR_VESA_NOT_INIT;
    }

    if(x > current_mode->width || y > current_mode->height)
    {
        return OS_ERR_OUT_OF_BOUND;
    }

    /* Get framebuffer address */
    addr = ((uint32_t*)virt_buffer) +
                        (current_mode->width * y) + x;

    back = (uint8_t*)addr;

    if(alpha == 0xFF)
    {
        pixel[0] = blue;
        pixel[1] = green;
        pixel[2] = red;
        pixel[3] = 0;
    }
    else if(alpha != 0x00)
    {
        pixel[0] = (blue * alpha + back[0] * (255 - alpha)) >> 8;
        pixel[1] = (green * alpha + back[1] * (255 - alpha)) >> 8;
        pixel[2] = (red * alpha + back[2] * (255 - alpha)) >> 8;
        pixel[3] = 0;
    }
    else
    {
        return OS_NO_ERR;
    }

    *addr = *((uint32_t*)pixel);

    return OS_NO_ERR;
}

__inline__ OS_RETURN_E vesa_draw_rectangle(const uint16_t x, const uint16_t y,
                                           const uint16_t width,
                                           const uint16_t height,
                                           const uint8_t alpha,
                                           const uint8_t red,
                                           const uint8_t green,
                                           const uint8_t blue)
{
    uint16_t i;
    uint16_t j;

    if(vesa_supported == 0)
    {
        return OS_ERR_VESA_NOT_SUPPORTED;
    }

    if(current_mode == NULL)
    {
        return OS_ERR_VESA_NOT_INIT;
    }

    if(x + width > current_mode->width || y + height > current_mode->height)
    {
        return OS_ERR_OUT_OF_BOUND;
    }

    for(i = y; i < y + height; ++i)
    {
        for(j = x; j < x + width; ++j)
        {
            vesa_draw_pixel(j, i, alpha, red, green, blue);
        }
    }

    return OS_NO_ERR;
}

void vesa_drawchar(const unsigned char charracter,
                   const uint32_t x, const uint32_t y,
                   const uint32_t fgcolor, const uint32_t bgcolor)
{
    uint32_t cx;
    uint32_t cy;

    uint32_t mask[8] = {1, 2, 4, 8, 16, 32, 64, 128};

    unsigned char *glyph = font_bitmap + (charracter - 31) * 16;

    uint8_t pixel[4] = {0};

    for(cy = 0; cy < 16; ++cy)
    {
        for(cx = 0; cx < 8; ++cx)
        {
            *((uint32_t*)pixel) = glyph[cy] & mask[cx] ? fgcolor : bgcolor;

            vesa_draw_pixel(x + (7 - cx ), y + cy,
                            pixel[3], pixel[2], pixel[1], pixel[0]);
        }
    }
}

uint32_t vesa_get_screen_width(void)
{
    if(vesa_supported == 0 || current_mode == NULL)
    {
        return 0;
    }

    return current_mode->width;
}

uint32_t vesa_get_screen_height(void)
{
    if(vesa_supported == 0 || current_mode == NULL)
    {
        return 0;
    }

    return current_mode->height;
}

uint8_t vesa_get_screen_bpp(void)
{
    if(vesa_supported == 0)
    {
        return 0;
    }

    if(current_mode == NULL)
    {
        return 0;
    }

    return current_mode->bpp;
}

void vesa_clear_screen(void)
{
    uint32_t* buffer;

    buffer = (uint32_t*)virt_buffer;
    
    fast_memset(buffer, 0, current_mode->width *
           current_mode->height *
           (current_mode->bpp / 8));
}

OS_RETURN_E vesa_put_cursor_at(const uint32_t line, const uint32_t column)
{
    /* Restore background */
    uint8_t i;
    uint32_t index = 0;
    for(i = 0; i < 16; ++i)
    {
        vesa_draw_pixel(screen_cursor.x, screen_cursor.y + i,
        save_buff[index], save_buff[index + 1],
        save_buff[index + 2], save_buff[index + 3]);
        index += 4;
        vesa_draw_pixel(screen_cursor.x + 1, screen_cursor.y + i,
        save_buff[index], save_buff[index + 1],
        save_buff[index + 2], save_buff[index + 3]);
        index += 4;
    }

    screen_cursor.x = column;
    screen_cursor.y = line;

    if(column + 2 < current_mode->width)
    {
        uint8_t i;
        uint32_t index = 0;
        for(i = 0; i < 16; ++i)
        {
            vesa_get_pixel(column, line + i,
            &save_buff[index], &save_buff[index + 1],
            &save_buff[index + 2], &save_buff[index + 3]);
            index += 4;
            vesa_draw_pixel(column, line + i, 0xFF, 0xFF, 0xFF, 0xFF);
            vesa_get_pixel(column + 1, line + i,
            &save_buff[index], &save_buff[index + 1],
            &save_buff[index + 2], &save_buff[index + 3]);
            index += 4;
            vesa_draw_pixel(column + 1, line + i, 0xFF, 0xFF, 0xFF, 0xFF);
        }
    }

    return OS_NO_ERR;
}

OS_RETURN_E vesa_save_cursor(cursor_t* buffer)
{
    if(buffer == NULL)
    {
        return OS_ERR_NULL_POINTER;
    }

    /* Save cursor attributes */
    buffer->x = screen_cursor.x;
    buffer->y = screen_cursor.y;

    return OS_NO_ERR;
}

OS_RETURN_E vesa_restore_cursor(const cursor_t buffer)
{
    if(buffer.x >= current_mode->width || buffer.y >= current_mode->height)
    {
        return OS_ERR_OUT_OF_BOUND;
    }

    /* Restore cursor attributes */
    vesa_put_cursor_at(buffer.y, buffer.x);

    return OS_NO_ERR;
}

/* Scroll in the desired direction of lines_count lines.
 *
 * @param direction The direction to whoch the console should be scrolled.
 * @param lines_count The number of lines to scroll.
 */
void vesa_scroll(const SCROLL_DIRECTION_E direction,
                 const uint32_t lines_count)
{
    uint8_t to_scroll;

    int32_t q;
    int32_t m;
    uint32_t* buffer_addr;
    uint32_t line_size;
    uint32_t bpp_size;
    uint32_t line_mem_size;

    to_scroll = lines_count;

    q = current_mode->height / font_height;
    m = current_mode->height % (q * font_height);

    buffer_addr = (uint32_t*)virt_buffer;

    line_size = font_height * current_mode->width;
    bpp_size = ((current_mode->bpp | 7) >> 3);
    line_mem_size = bpp_size * line_size;

    /* Select scroll direction */
    if(direction == SCROLL_DOWN)
    {
        uint8_t i;
        uint8_t j;
        uint32_t* src;
        uint32_t* dst;

        i = 0;
        dst = buffer_addr + i * line_size;
        src = dst + line_size;
        /* For each line scroll we want */
        for(j = 0; j < to_scroll; ++j)
        {
            /* Copy all the lines to the above one */
            for(i = 0; i < q - 1; ++i)
            {
                dst = buffer_addr + i * line_size;
                src = dst + line_size;
                memmove(dst, src ,line_mem_size);
                last_columns[i] = last_columns[i + 1];
            }
        }
        fast_memset(src, 0, line_mem_size);
    }

    /* Replace cursor */
    vesa_put_cursor_at(current_mode->height - m - font_height, 0);
    last_columns[(screen_cursor.y / font_height)] = 0;

    if(to_scroll <= last_printed_cursor.y)
    {
        last_printed_cursor.y -= to_scroll * font_height;
    }
    else
    {
        last_printed_cursor.x = 0;
        last_printed_cursor.y = 0;
    }
}

void vesa_set_color_scheme(const colorscheme_t color_scheme)
{
    screen_scheme.vga_color = color_scheme.vga_color;
    /* Translate color to RGB */
    if(screen_scheme.vga_color != 0)
    {
        screen_scheme.foreground =
                                vga_color_table[color_scheme.foreground & 0x0F];
		screen_scheme.background =
                                vga_color_table[color_scheme.background >> 4];
    }
    else
    {
        screen_scheme.foreground = color_scheme.foreground;
        screen_scheme.background = color_scheme.background;
    }
}

OS_RETURN_E vesa_save_color_scheme(colorscheme_t* buffer)
{
    if(buffer == NULL)
    {
        return OS_ERR_NULL_POINTER;
    }

    /* Save color scheme into buffer */
    buffer->vga_color = screen_scheme.vga_color;
    buffer->foreground = screen_scheme.foreground;
    buffer->background = screen_scheme.background;

    return OS_NO_ERR;
}

void vesa_put_string(const char* string)
{
    /* Output each character of the string */
    uint32_t i;
    for(i = 0; i < strlen(string); ++i)
    {
        vesa_process_char(string[i]);
        last_printed_cursor = screen_cursor;
    }
}

void vesa_put_char(const char charactrer)
{
    vesa_process_char(charactrer);
    last_printed_cursor = screen_cursor;
}

void vesa_console_write_keyboard(const char* string, const uint32_t size)
{
    /* Output each character of the string */
    uint32_t i;
    for(i = 0; i < size; ++i)
    {
        vesa_process_char(string[i]);
    }
}

void vesa_fill_screen(const void* pointer)
{
    uint32_t* buffer;

    buffer = (uint32_t*)virt_buffer;
    fast_memcpy(buffer, pointer,
           current_mode->width *
           current_mode->height *
           (current_mode->bpp / 8));
}

void vesa_flush_buffer(void)
{
#if DISPLAY_TYPE == DISPLAY_VESA_BUF
    uint32_t* buffer;
    uint32_t* hw_buffer;

    buffer = (uint32_t*)virt_buffer_;
    hw_buffer = (uint32_t*)current_mode->framebuffer;

    fast_memcpy(hw_buffer, buffer,
           current_mode->width *
           current_mode->height *
           (current_mode->bpp / 8));
#endif
}

void vesa_set_transparent_char(const uint32_t enabled)
{
    transparent_char = enabled == 1 ? 1 : 0;
}

void* vesa_double_buffer_thread(void* args)
{
    (void)args;

    while(1)
    {
        vesa_flush_buffer();
        sched_sleep(30);
    }

    return NULL;
}