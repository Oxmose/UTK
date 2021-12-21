/*******************************************************************************
 * @file vga_text.c
 *
 * @see vga_text.h
 *
 * @author Alexy Torres Aurora Dugo
 *
 * @date 28/02/2021
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

#include <stdint.h>          /* Generic int types */
#include <string.h>          /* String manipualtion */
#include <cpu.h>             /* CPU port manipulation */
#include <uart.h>            /* UART driver */
#include <kernel_output.h>   /* Kernel output manager */
#include <memmgt.h>          /* Memory management */
#include <graphic.h>         /* Graphic driver manager */
#include <panic.h>           /* Kernel panic */

/* UTK configuration file */
#include <config.h>

/* Tests header file */
#include <test_bank.h>

/* Header file */
#include <vga_text.h>

/*******************************************************************************
 * CONSTANTS
 ******************************************************************************/

/** @brief VGA frame buffer base physical address. */
#define VGA_TEXT_FRAMEBUFFER 0xB8000

/** @brief VGA frame buffer size */
#define VGA_TEXT_FRAMEBUFFER_SIZE 0x7D00

/** @brief VGA CPU management data port. */
#define VGA_TEXT_SCREEN_DATA_PORT 0x3D5
/** @brief VGA CPU management command port. */
#define VGA_TEXT_SCREEN_COMM_PORT 0x3D4
/** @brief VGA screen width. */
#define VGA_TEXT_SCREEN_COL_SIZE  80
/** @brief VGA screen height. */
#define VGA_TEXT_SCREEN_LINE_SIZE 25

/** @brief VGA cursor position command low. */
#define VGA_TEXT_CURSOR_COMM_LOW  0x0F
/** @brief VGA cursor position command high. */
#define VGA_TEXT_CURSOR_COMM_HIGH 0x0E

/*******************************************************************************
 * STRUCTURES
 ******************************************************************************/

/* None */

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
static kernel_graphic_driver_t vga_text_driver = {
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

/*******************************************************************************
 * STATIC FUNCTIONS DEFINITIONS
 ******************************************************************************/

/**
 * @brief Prints a character to the selected coordinates.
 *
 * @details Prints a character to the selected coordinates by setting the memory
 * accordingly.
 *
 * @param[in] line The line index where to write the character.
 * @param[in] column The colums index where to write the character.
 * @param[in] character The character to display on the screem.
 */
inline static void vga_print_char(const uint32_t line,
                                  const uint32_t column,
                                  const char character);

/**
 * @brief Processes the character in parameters.
 *
 * @details Check the character nature and code. Corresponding to the
 * character's code, an action is taken. A regular character will be printed
 * whereas \n will create a line feed.
 *
 * @param[in] character The character to process.
 */
static void vga_process_char(const char character);

/**
 * @brief Returns the VGA frame buffer virtual address.
 *
 * @details Returns the VGA frame buffer virtual address correponding to a
 * certain region of the buffer given the parameters.
 *
 * @param[in] line The frame buffer line.
 * @param[in] column The frame buffer column.
 *
 * @return The frame buffer virtual address is returned correponding to a
 * certain region of the buffer given the parameters is returned.
 */
inline static uint16_t* vga_get_framebuffer(const uint32_t line,
                                            const uint32_t column);

/*******************************************************************************
 * FUNCTIONS
 ******************************************************************************/

#define VGA_ASSERT(COND, MSG, ERROR) {                      \
    if((COND) == FALSE)                                     \
    {                                                       \
        PANIC(ERROR, "VGA", MSG, TRUE);                     \
    }                                                       \
}

inline static void vga_print_char(const uint32_t line,
                                  const uint32_t column,
                                  const char character)
{
    uint16_t* screen_mem;

    if(line > VGA_TEXT_SCREEN_LINE_SIZE - 1 ||
       column > VGA_TEXT_SCREEN_COL_SIZE - 1)
    {
        return;
    }

    /* Get address to inject */
    screen_mem = vga_get_framebuffer(line, column);

    /* Inject the character with the current colorscheme */
    *screen_mem = character |
                  ((screen_scheme.background << 8) & 0xF000) |
                  ((screen_scheme.foreground << 8) & 0x0F00);
}

static void vga_process_char(const char character)
{
#if KERNEL_LOG_LEVEL >= DEBUG_LOG_LEVEL
    /* Write on serial */
    uart_put_char(character);
#endif

    /* If character is a normal ASCII character */
    if(character > 31 && character < 127)
    {
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

        /* Display character and move cursor */
        vga_print_char(screen_cursor.y, screen_cursor.x++,
                character);
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

static inline uint16_t* vga_get_framebuffer(const uint32_t line,
                                            const uint32_t column)
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

void vga_init(void)
{
    OS_RETURN_E err;

    KERNEL_DEBUG(VGA_DEBUG_ENABLED, "[VGA] Initializing VGA text driver");

    /* Init framebuffer */
    vga_framebuffer = (uint16_t*)VGA_TEXT_FRAMEBUFFER;

    /* Declared the new hardware */
    err = memory_declare_hw((uintptr_t)vga_framebuffer,
                             VGA_TEXT_FRAMEBUFFER_SIZE);
    VGA_ASSERT(err == OS_NO_ERR, "Could not initialize VGA kernel driver", err);

    /* Map the driver's memory */
    memory_mmap_direct(vga_framebuffer,
                       vga_framebuffer,
                       VGA_TEXT_FRAMEBUFFER_SIZE,
                       0,
                       0,
                       0,
                       1,
                       NULL);
}

void vga_clear_screen(void)
{
    uint32_t i;
    uint32_t j;
    uint16_t blank = ' ' |
                     ((screen_scheme.background << 8) & 0xF000) |
                     ((screen_scheme.foreground << 8) & 0x0F00);

    /* Clear all screen cases */
    for(i = 0; i < VGA_TEXT_SCREEN_LINE_SIZE; ++i)
    {
        for(j = 0; j < VGA_TEXT_SCREEN_COL_SIZE; ++j)
        {
            *(vga_get_framebuffer(i, j)) = blank;
        }
        last_columns[i] = 0;
    }
}

void vga_put_cursor_at(const uint32_t line, const uint32_t column)
{
    int16_t  cursor_position;

    /* Checks the values of line and column */
    if(column > VGA_TEXT_SCREEN_COL_SIZE ||
       line > VGA_TEXT_SCREEN_LINE_SIZE)
    {
        return;
    }

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
}

void vga_save_cursor(cursor_t* buffer)
{
    if(buffer != NULL)
    {
        /* Save cursor attributes */
        buffer->x = screen_cursor.x;
        buffer->y = screen_cursor.y;
    }
}

void vga_restore_cursor(const cursor_t buffer)
{
    if(buffer.x >= VGA_TEXT_SCREEN_COL_SIZE ||
       buffer.y >= VGA_TEXT_SCREEN_LINE_SIZE)
    {
        return;
    }
    /* Restore cursor attributes */
    vga_put_cursor_at(buffer.y, buffer.x);
}

void vga_scroll(const SCROLL_DIRECTION_E direction, const uint32_t lines_count)
{
    uint32_t to_scroll;

    if(VGA_TEXT_SCREEN_LINE_SIZE < lines_count)
    {
        to_scroll = VGA_TEXT_SCREEN_LINE_SIZE;
    }
    else
    {
        to_scroll = lines_count;
    }

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
}

void vga_set_color_scheme(const colorscheme_t color_scheme)
{
    screen_scheme.foreground = color_scheme.foreground;
    screen_scheme.background = color_scheme.background;
}

void vga_save_color_scheme(colorscheme_t* buffer)
{
    if(buffer != NULL)
    {
        /* Save color scheme into buffer */
        buffer->foreground = screen_scheme.foreground;
        buffer->background = screen_scheme.background;
    }
}

void vga_put_string(const char* string)
{
    size_t i;

    /* Output each character of the string */
    for(i = 0; i < strlen(string); ++i)
    {
        vga_put_char(string[i]);
    }
}

void vga_put_char(const char character)
{
    vga_process_char(character);
    last_printed_cursor = screen_cursor;
}

void vga_console_write_keyboard(const char* string, const size_t size)
{
    size_t i;

    /* Output each character of the string */
    for(i = 0; i < size; ++i)
    {
        vga_process_char(string[i]);
    }
}

const kernel_graphic_driver_t* vga_text_get_driver(void)
{
    return &vga_text_driver;
}