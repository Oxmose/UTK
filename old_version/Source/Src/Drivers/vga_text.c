/***************************************************************************//**
 * @file vga_text.c
 *
 * @see vga_text.h
 *
 * @author Alexy Torres Aurora Dugo
 *
 * @date 03/10/2017
 *
 * @version 1.5
 *
 * @brief VGA text mode driver.
 *
 * @details Allows the kernel to display text and general ASCII characters to be
 * displayed on the screen. Includes cursor management, screen colors management
 * and other fancy screen driver things.
 *
 * @copyright Alexy Torres Aurora Dugo
 ******************************************************************************/

#include <Lib/stdint.h>     /* Generic int types */
#include <Lib/stddef.h>     /* OS_RETURN_E, NULL */
#include <Lib/string.h>     /* memmove */
#include <Cpu/cpu.h>        /* outb */
#include <Cpu/panic.h>      /* kernel panic */
#include <Drivers/serial.h> /* serial_write */
#include <Sync/critical.h>  /* ENTER_CRITICAL, EXIT_CRITICAL */
#include <Memory/paging.h>  /* Memory management */
#include <Memory/paging_alloc.h>  /* Memory management */

/* UTK configuration file */
#include <config.h>

/* Header file */
#include <Drivers/vga_text.h>

/*******************************************************************************
 * GLOBAL VARIABLES
 ******************************************************************************/

/* Screen runtime parameters */
/** @brief Stores the curent screen's color scheme. */
static colorscheme_t screen_scheme = {
    .background = BG_BLACK,
    .foreground = FG_WHITE
};

/** @brief Stores the curent screen's cursor settings. */
static cursor_t      screen_cursor;
/**
 * @brief Stores the curent screen's cursor settings ofthe last printed
 * character.
 */
static cursor_t      last_printed_cursor;

/* Set the last column printed with a char */
/**
 * @brief Stores the column index of the last printed character for each lines
 * of the screen.
 */
static uint8_t last_columns[VGA_TEXT_SCREEN_LINE_SIZE] = {0};

/** @brief VGA frame buffer address. */
static uint16_t* vga_framebuffer = (uint16_t*)VGA_TEXT_FRAMEBUFFER;

/**
 * @brief VGA text driver instance.
 */
kernel_graphic_driver_t vga_text_driver = {
    .clear_screen = vga_clear_screen,
    .put_cursor_at = vga_put_cursor_at,
    .save_cursor = vga_save_cursor,
    .restore_cursor = vga_restore_cursor,
    .scroll = vga_scroll,
    .set_color_scheme = vga_set_color_scheme,
    .save_color_scheme = vga_save_color_scheme,
    .put_string = vga_put_string,
    .put_char = vga_put_char,
    .console_write_keyboard = vga_console_write_keyboard
};

#if MAX_CPU_COUNT > 1
/** @brief Critical section spinlock. */
static  spinlock_t lock = SPINLOCK_INIT_VALUE;
#endif

/*******************************************************************************
 * FUNCTIONS
 ******************************************************************************/

static void vga_pagefault_handler(address_t fault_addr)
{
    uint32_t    page_count;
    uint32_t    size;
    OS_RETURN_E err;

    (void) fault_addr;

    size       = sizeof(uint16_t) * 
                 VGA_TEXT_SCREEN_COL_SIZE * 
                 VGA_TEXT_SCREEN_LINE_SIZE;
    page_count = size / KERNEL_PAGE_SIZE;
    if(size % KERNEL_PAGE_SIZE)
    {
        ++page_count;
    }

    /* Ask for the kernel to map the buffer */
    err = kernel_mmap_hw((void*)VGA_TEXT_FRAMEBUFFER, (void*)VGA_TEXT_FRAMEBUFFER, 
                         size, 0, 0);
    if(err != OS_NO_ERR)
    {
        kernel_panic(err);
    }
}

/**
 * @brief Prints a character to the selected coordinates.
 *
 * @details Prints a character to the selected coordinates by setting the memory
 * accordingly.
 *
 * @param[in] line The line index where to write the character.
 * @param[in] column The colums index where to write the character.
 * @param[in] character The character to display on the screem.
 *
 * @return The success state or the error code. OS_NO_ERR if no error is
 * encountered. OS_ERR_OUT_OF_BOUND is returned if the parameters are
 * out of bound.
 */
static OS_RETURN_E vga_print_char(const uint32_t line, const uint32_t column,
                                  const char character)
{
    uint16_t* screen_mem;
    uint32_t  word;

    if(line > VGA_TEXT_SCREEN_LINE_SIZE - 1 ||
       column > VGA_TEXT_SCREEN_COL_SIZE - 1)
    {
        return OS_ERR_OUT_OF_BOUND;
    }

    /* Get address to inject */
    screen_mem = vga_get_framebuffer(line, column);

    #if MAX_CPU_COUNT > 1
    ENTER_CRITICAL(word, &lock);
    #else
    ENTER_CRITICAL(word);
    #endif

    /* Inject the character with the current colorscheme */
    *screen_mem = character |
                  ((screen_scheme.background << 8) & 0xF000) |
                  ((screen_scheme.foreground << 8) & 0x0F00);

    #if MAX_CPU_COUNT > 1
    EXIT_CRITICAL(word, &lock);
    #else
    EXIT_CRITICAL(word);
    #endif

    return OS_NO_ERR;
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
static void vga_process_char(const char character)
{
    #if (KERNEL_DEBUG == 1) | (TEST_MODE_ENABLED == 1)
    /* Write on serial */
    serial_write(COM1, character);
    #endif

    /* If character is a normal ASCII character */
    if(character > 31 && character < 127)
    {
        /* Display character and move cursor */
        vga_print_char(screen_cursor.y, screen_cursor.x++,
                character);

        /* Manage end of line cursor position */
        if(screen_cursor.x > VGA_TEXT_SCREEN_COL_SIZE - 1)
        {
            vga_put_cursor_at(screen_cursor.y + 1, 0);
            last_columns[screen_cursor.y] = screen_cursor.x;
        }

        /* Manage end of screen cursor position */
        if(screen_cursor.y >= VGA_TEXT_SCREEN_LINE_SIZE)
        {
            vga_scroll(SCROLL_DOWN, 1);

        }
        else
        {
            /* Move cursor */
            vga_put_cursor_at(screen_cursor.y, screen_cursor.x);
            last_columns[screen_cursor.y] = screen_cursor.x;
        }
    }
    else
    {
        /* Manage special ACSII characters*/
        switch(character)
        {
            /* Backspace */
            case '\b':
                if(last_printed_cursor.y == screen_cursor.y)
                {
                    if(screen_cursor.x > last_printed_cursor.x)
                    {
                        vga_put_cursor_at(screen_cursor.y, screen_cursor.x - 1);
                        last_columns[screen_cursor.y] = screen_cursor.x;
                        vga_print_char(screen_cursor.y, screen_cursor.x, ' ');
                    }
                }
                else if(last_printed_cursor.y < screen_cursor.y)
                {
                    if(screen_cursor.x > 0)
                    {
                        vga_put_cursor_at(screen_cursor.y, screen_cursor.x - 1);
                        last_columns[screen_cursor.y] = screen_cursor.x;
                        vga_print_char(screen_cursor.y, screen_cursor.x, ' ');
                    }
                    else
                    {
                        if(last_columns[screen_cursor.y - 1] >=
                              VGA_TEXT_SCREEN_COL_SIZE)
                        {
                            last_columns[screen_cursor.y - 1] =
                               VGA_TEXT_SCREEN_COL_SIZE - 1;
                        }

                        vga_put_cursor_at(screen_cursor.y - 1,
                                      last_columns[screen_cursor.y - 1]);
                        vga_print_char(screen_cursor.y, screen_cursor.x, ' ');
                    }
                }
                break;
            /* Tab */
            case '\t':
                if(screen_cursor.x + 8 < VGA_TEXT_SCREEN_COL_SIZE - 1)
                {
                    vga_put_cursor_at(screen_cursor.y,
                            screen_cursor.x  +
                            (8 - screen_cursor.x % 8));
                }
                else
                {
                    vga_put_cursor_at(screen_cursor.y,
                           VGA_TEXT_SCREEN_COL_SIZE - 1);
                }
                last_columns[screen_cursor.y] = screen_cursor.x;
                break;
            /* Line feed */
            case '\n':
                if(screen_cursor.y < VGA_TEXT_SCREEN_LINE_SIZE - 1)
                {
                    vga_put_cursor_at(screen_cursor.y + 1, 0);
                    last_columns[screen_cursor.y] = screen_cursor.x;
                }
                else
                {
                    vga_scroll(SCROLL_DOWN, 1);
                }
                break;
            /* Clear screen */
            case '\f':
                vga_clear_screen();
                break;
            /* Line return */
            case '\r':
                vga_put_cursor_at(screen_cursor.y, 0);
                last_columns[screen_cursor.y] = screen_cursor.x;
                break;
            /* Undefined */
            default:
                break;
        }
    }
}

uint16_t* vga_get_framebuffer(const uint32_t line, const uint32_t column)
{
    /* Avoid overflow on text mode */
    if(line > VGA_TEXT_SCREEN_LINE_SIZE - 1 ||
       column > VGA_TEXT_SCREEN_COL_SIZE -1)
    {
        return vga_framebuffer;
    }

    /* Returns the mem adress of the coordinates */
    return (uint16_t*)vga_framebuffer +
           (column + line * VGA_TEXT_SCREEN_COL_SIZE);
}

OS_RETURN_E vga_init(void)
{
    OS_RETURN_E err; 

    /* Init page fault handler */
    err = paging_register_fault_handler(
        vga_pagefault_handler, 
        VGA_TEXT_FRAMEBUFFER, 
        VGA_TEXT_FRAMEBUFFER + 2 * 
        VGA_TEXT_SCREEN_COL_SIZE * VGA_TEXT_SCREEN_LINE_SIZE);

    return err;
}

void vga_clear_screen(void)
{
    uint32_t i;
    uint32_t j;
    uint32_t word;
    uint16_t blank = ' ' |
                     ((screen_scheme.background << 8) & 0xF000) |
                     ((screen_scheme.foreground << 8) & 0x0F00);

    /* Clear all screen cases */
    #if MAX_CPU_COUNT > 1
    ENTER_CRITICAL(word, &lock);
    #else
    ENTER_CRITICAL(word);
    #endif
    for(i = 0; i < VGA_TEXT_SCREEN_LINE_SIZE; ++i)
    {
        for(j = 0; j < VGA_TEXT_SCREEN_COL_SIZE; ++j)
        {
            *(vga_get_framebuffer(i, j)) = blank;
        }
        last_columns[i] = 0;
    }
    #if MAX_CPU_COUNT > 1
    EXIT_CRITICAL(word, &lock);
    #else
    EXIT_CRITICAL(word);
    #endif
}

OS_RETURN_E vga_put_cursor_at(const uint32_t line, const uint32_t column)
{
    int16_t  cursor_position;
    uint32_t word;

    /* Checks the values of line and column */
    if(column > VGA_TEXT_SCREEN_COL_SIZE ||
       line > VGA_TEXT_SCREEN_LINE_SIZE)
    {
        return OS_ERR_OUT_OF_BOUND;
    }

    #if MAX_CPU_COUNT > 1
    ENTER_CRITICAL(word, &lock);
    #else
    ENTER_CRITICAL(word);
    #endif

    /* Set new cursor position */
    screen_cursor.x = column;
    screen_cursor.y = line;

    /* Display new position on screen */
    cursor_position = column + line * VGA_TEXT_SCREEN_COL_SIZE;

    /* Send low part to the screen */
    cpu_outb(VGA_TEXT_CURSOR_COMM_LOW, VGA_TEXT_SCREEN_COMM_PORT);
    cpu_outb((int8_t)(cursor_position & 0x00FF), VGA_TEXT_SCREEN_DATA_PORT);

    /* Send high part to the screen */
    cpu_outb(VGA_TEXT_CURSOR_COMM_HIGH, VGA_TEXT_SCREEN_COMM_PORT);
    cpu_outb((int8_t)((cursor_position & 0xFF00) >> 8),
             VGA_TEXT_SCREEN_DATA_PORT);

    #if MAX_CPU_COUNT > 1
    EXIT_CRITICAL(word, &lock);
    #else
    EXIT_CRITICAL(word);
    #endif

    return OS_NO_ERR;
}

OS_RETURN_E vga_save_cursor(cursor_t* buffer)
{
    uint32_t word;

    if(buffer == NULL)
    {
        return OS_ERR_NULL_POINTER;
    }

    #if MAX_CPU_COUNT > 1
    ENTER_CRITICAL(word, &lock);
    #else
    ENTER_CRITICAL(word);
    #endif

    /* Save cursor attributes */
    buffer->x = screen_cursor.x;
    buffer->y = screen_cursor.y;

    #if MAX_CPU_COUNT > 1
    EXIT_CRITICAL(word, &lock);
    #else
    EXIT_CRITICAL(word);
    #endif

    return OS_NO_ERR;
}

OS_RETURN_E vga_restore_cursor(const cursor_t buffer)
{
    if(buffer.x >= VGA_TEXT_SCREEN_COL_SIZE ||
       buffer.y >= VGA_TEXT_SCREEN_LINE_SIZE)
    {
        return OS_ERR_OUT_OF_BOUND;
    }
    /* Restore cursor attributes */
    vga_put_cursor_at(buffer.y, buffer.x);

    return OS_NO_ERR;
}

void vga_scroll(const SCROLL_DIRECTION_E direction, const uint32_t lines_count)
{
    uint32_t to_scroll;
    uint32_t word;

    if(VGA_TEXT_SCREEN_LINE_SIZE < lines_count)
    {
        to_scroll = VGA_TEXT_SCREEN_LINE_SIZE;
    }
    else
    {
        to_scroll = lines_count;
    }

    #if MAX_CPU_COUNT > 1
    ENTER_CRITICAL(word, &lock);
    #else
    ENTER_CRITICAL(word);
    #endif

    /* Select scroll direction */
    if(direction == SCROLL_DOWN)
    {
        uint32_t i;
        uint32_t j;


        /* For each line scroll we want */
        for(j = 0; j < to_scroll; ++j)
        {
            /* Copy all the lines to the above one */
            for(i = 0; i < VGA_TEXT_SCREEN_LINE_SIZE - 1; ++i)
            {
                memmove(vga_get_framebuffer(i, 0),
                        vga_get_framebuffer(i + 1, 0),
                        sizeof(uint16_t) * VGA_TEXT_SCREEN_COL_SIZE);
                last_columns[i] = last_columns[i+1];
            }
            last_columns[VGA_TEXT_SCREEN_LINE_SIZE - 1] = 0;
        }
        /* Clear last line */
        for(i = 0; i < VGA_TEXT_SCREEN_COL_SIZE; ++i)
        {
            vga_print_char(VGA_TEXT_SCREEN_LINE_SIZE - 1, i, ' ');
        }

    }

    /* Replace cursor */
    vga_put_cursor_at(VGA_TEXT_SCREEN_LINE_SIZE - to_scroll, 0);

    if(to_scroll <= last_printed_cursor.y)
    {
        last_printed_cursor.y -= to_scroll;
    }
    else
    {
        last_printed_cursor.x = 0;
        last_printed_cursor.y = 0;
    }

    #if MAX_CPU_COUNT > 1
    EXIT_CRITICAL(word, &lock);
    #else
    EXIT_CRITICAL(word);
    #endif
}

void vga_set_color_scheme(const colorscheme_t color_scheme)
{
    uint32_t word;

    #if MAX_CPU_COUNT > 1
    ENTER_CRITICAL(word, &lock);
    #else
    ENTER_CRITICAL(word);
    #endif

    screen_scheme.foreground = color_scheme.foreground;
    screen_scheme.background = color_scheme.background;

    #if MAX_CPU_COUNT > 1
    EXIT_CRITICAL(word, &lock);
    #else
    EXIT_CRITICAL(word);
    #endif
}

OS_RETURN_E vga_save_color_scheme(colorscheme_t* buffer)
{
    uint32_t word;

    if(buffer == NULL)
    {
        return OS_ERR_NULL_POINTER;
    }

    #if MAX_CPU_COUNT > 1
    ENTER_CRITICAL(word, &lock);
    #else
    ENTER_CRITICAL(word);
    #endif

    /* Save color scheme into buffer */
    buffer->foreground = screen_scheme.foreground;
    buffer->background = screen_scheme.background;

    #if MAX_CPU_COUNT > 1
    EXIT_CRITICAL(word, &lock);
    #else
    EXIT_CRITICAL(word);
    #endif

    return OS_NO_ERR;
}

void vga_put_string(const char* string)
{
    /* Output each character of the string */
    uint32_t i;
    for(i = 0; i < strlen(string); ++i)
    {
        vga_put_char(string[i]);
    }
}

void vga_put_char(const char character)
{
    uint32_t word;

    #if MAX_CPU_COUNT > 1
    ENTER_CRITICAL(word, &lock);
    #else
    ENTER_CRITICAL(word);
    #endif

    vga_process_char(character);
    last_printed_cursor = screen_cursor;

    #if MAX_CPU_COUNT > 1
    EXIT_CRITICAL(word, &lock);
    #else
    EXIT_CRITICAL(word);
    #endif
}

void vga_console_write_keyboard(const char* string, const uint32_t size)
{
    /* Output each character of the string */
    uint32_t i;
    for(i = 0; i < size; ++i)
    {
        vga_process_char(string[i]);
    }
}
