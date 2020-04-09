/*******************************************************************************
 * @file keyboard.c
 * 
 * @see keyboard.h
 *
 * @author Alexy Torres Aurora Dugo
 *
 * @date 08/10/2017
 *
 * @version 1.5
 *
 * @brief Keyboard driver (PS2/USB) for the kernel.
 * 
 * @details Keyboard driver (PS2/USB) for the kernel. Enables the user inputs 
 * through the keyboard.
 * 
 * @copyright Alexy Torres Aurora Dugo
 ******************************************************************************/

#include <io/graphic.h>           /* Graphic API */
#include <cpu.h>                  /* CPU management */
#include <interrupt/interrupts.h> /* Interrupt management */
#include <interrupt_settings.h>   /* Interrupt settings */
#include <lib/stdint.h>           /* Generic int types */
#include <lib/stddef.h>           /* Standard definitions */
#include <lib/string.h>           /* String manipulation */

/* UTK configuration file */
#include <config.h>

/* Header file */
#include <keyboard.h>

/*******************************************************************************
 * GLOBAL VARIABLES
 ******************************************************************************/

/** @brief Secure input mode. */
static uint32_t secure_input;
/** @brief Imput display mode. */
static uint32_t display_keyboard;

/** @brief Shift key used flag. */
static uint32_t keyboard_flags;

/** @brief Keyboard buffer. */
static kbd_buffer_t kbd_buf;
/** @brief Keyboard map. */
static const key_mapper_t qwerty_map =
{
    .regular =
    {
        0,
        0,   // ESCAPE
        '1',
        '2',
        '3',
        '4',
        '5',
        '6',
        '7',
        '8',
        '9',                      // 10
        '0',
        '-',
        '=',
        KEY_BACKSPACE,   // BACKSPACE
        KEY_TAB,   // TAB
        'q',
        'w',
        'e',
        'r',
        't',                    // 20
        'y',
        'u',
        'i',
        'o',
        'p',
        0,   // MOD ^
        0,   // MOD ¸
        KEY_RETURN,   // ENTER
        0,   // VER MAJ
        'a',                    // 30
        's',
        'd',
        'f',
        'g',
        'h',
        'j',
        'k',
        'l',
        ';',
        0,   // MOD `           // 40
        0,   // TODO
        KEY_LSHIFT,   // LEFT SHIFT
        '<',
        'z',
        'x',
        'c',
        'v',
        'b',
        'n',
        'm',                    // 50
        ',',
        '.',
        ' ', // é
        KEY_RSHIFT,    // RIGHT SHIFT
        0,   // TODO
        0,     // ALT left / right
        ' ',
        ' ',
        0,     // F1
        0,     // F2               // 60
        0,     // F3
        0,     // F4
        0,     // F5
        0,     // F6
        0,     // F7
        0,     // F8
        0,     // F9
        0,     // SCROLL LOCK
        0,     // PAUSE
        0,                     // 70
        0,
        0,
        KEY_PGUP,
        0,
        0,
        0,
        0,
        0,
        0,
        0,                  // 80
        KEY_PGDOWN,
        0
    },

    .shifted =
    {
        0,
        0,   // ESCAPE
        '!',
        '"',
        '/',
        '$',
        '%',
        '?',
        '&',
        '*',
        '(',                      // 10
        ')',
        '_',
        '+',
        KEY_BACKSPACE,   // BACKSPACE
        KEY_TAB,         // TAB
        'Q',
        'W',
        'E',
        'R',
        'T',                    // 20
        'Y',
        'U',
        'I',
        'O',
        'P',
        0,   // MOD ^
        0,   // MOD ¨
        KEY_RETURN,   // ENTER
        0,   // VER MAJ
        'A',                    // 30
        'S',
        'D',
        'F',
        'G',
        'H',
        'J',
        'K',
        'L',
        ':',
        0,   // MOD `           // 40
        0,   // TODO
        KEY_LSHIFT,   // LEFT SHIFT
        '>',
        'Z',
        'X',
        'C',
        'V',
        'B',
        'N',
        'M',                    // 50
        '\'',
        '.',
        ' ', // É
        KEY_RSHIFT,    // RIGHT SHIFT
        0,   // TODO
        0,     // ALT left / right
        ' ',
        ' ',
        0,     // F1
        0,     // F2               // 60
        0,     // F3
        0,     // F4
        0,     // F5
        0,     // F6
        0,     // F7
        0,     // F8
        0,     // F9
        0,     // SCROLL LOCK
        0,     // PAUSE
        0,                     // 70
        0,
        0,
        KEY_PGUP,
        0,
        0,
        0,
        0,
        0,
        0,
        0,                  // 80
        KEY_PGDOWN,
        0
    }
};

/*******************************************************************************
 * FUNCTIONS
 ******************************************************************************/

/**
 * @brief Parses a keyboard keycode.
 * 
 * @details Parses the keycode given as parameter and execute the corresponding 
 * action.
 *
 * @param[in] keycode The keycode to parse.
 */
static void manage_keycode(const int32_t keycode)
{
    char    character;
    //OS_RETURN_E err;

    /* Manage push of release */
    if(keycode > 0)
    {
        int8_t  mod;

        mod = 0;

        /* Manage modifiers */
        switch(qwerty_map.regular[keycode])
        {
            case KEY_LSHIFT:
                keyboard_flags |= KBD_LSHIFT;
                mod = 1;
                break;
            case KEY_RSHIFT:
                keyboard_flags |= KBD_RSHIFT;
                mod = 1;
                break;
            default:
                break;
        }
        /* Manage only set characters */
        if(!mod &&
           (qwerty_map.regular[keycode] || qwerty_map.shifted[keycode]))
        {
            int8_t  shifted;

            shifted = (keyboard_flags & KBD_LSHIFT) |
                      (keyboard_flags & KBD_RSHIFT);
            character = (shifted > 0) ?
                         qwerty_map.shifted[keycode] :
                         qwerty_map.regular[keycode];

            /* Manage user buffer */
            if(kbd_buf.type != 0)
            {
                if(kbd_buf.type == 1)
                {
                    /* read */
                    if(character == KEY_RETURN)
                    {
                        if(kbd_buf.read < kbd_buf.read_size)
                        {
                            kbd_buf.char_buf[kbd_buf.read++] = character;
                        }

                        kbd_buf.type = 0;
#if 0
                        err = sem_post(&kbd_buf.sem);
                        if(err != OS_NO_ERR)
                        {
                            kernel_error("Keyboard driver failure");
                            kernel_panic(err);
                        }
#endif
                    }
                    else if(character == KEY_BACKSPACE)
                    {
                        if(kbd_buf.read > 0)
                        {
                            --kbd_buf.read;
                        }

                        kbd_buf.char_buf[kbd_buf.read] = 0;
                    }
                    else if(kbd_buf.read < kbd_buf.read_size)
                    {
                        kbd_buf.char_buf[kbd_buf.read++] = character;
                    }
                }
                else if(kbd_buf.type  == 2)
                {
                    /* getch */
                    if(kbd_buf.read < kbd_buf.read_size)
                    {
                        kbd_buf.char_buf[kbd_buf.read++] = character;
                    }

                    kbd_buf.type = 0;
#if 0
                    err = sem_post(&kbd_buf.sem);
                    if(err != OS_NO_ERR)
                    {
                        kernel_error("Keyboard driver failure");
                        kernel_panic(err);
                    }
#endif
                }
            }

            /* Display character */
            if(display_keyboard)
            {
                if(secure_input &&
                    character != KEY_RETURN &&
                    character != KEY_BACKSPACE)
                {
                    graphic_console_write_keyboard("*", 1);
                }
                else
                {
                    graphic_console_write_keyboard(&character, 1);
                }
            }
        }
    }
    else
    {
        int32_t new_keycode;

        new_keycode = 128 + keycode;
        /* Manage modifiers */
        switch(qwerty_map.regular[new_keycode])
        {
            case KEY_LSHIFT:
                keyboard_flags &= ~KBD_LSHIFT;
                break;
            case KEY_RSHIFT:
                keyboard_flags &= ~KBD_RSHIFT;
                break;
            default:
                break;
        }
    }
}

/**
 * @brief Keyboard IRQ handler.
 * 
 * @details Keyboard IRQ handler, reads the key value and manage thread blocked 
 * on IO.
 *
 * @param[in] cpu_state The cpu registers before the interrupt.
 * @param[in] int_id The interrupt line that called the handler.
 * @param[in] stack_state The stack state before the interrupt.
 */
static void keyboard_interrupt_handler(cpu_state_t* cpu_state, uintptr_t int_id,
                                       stack_state_t* stack_state)
{
    (void)cpu_state;
    (void)int_id;
    (void)stack_state;

    /* Read if not empty and not from auxiliary port */
    if((cpu_inb(KEYBOARD_COMM_PORT) & 0x100001) == 1)
    {
        int8_t keycode;
      
        /* Retrieve key code and test it */
        keycode = cpu_inb(KEYBOARD_DATA_PORT);

        /* Manage keycode */
        manage_keycode(keycode);
    }

    kernel_interrupt_set_irq_eoi(KBD_IRQ_LINE);
}

OS_RETURN_E keyboard_init(void)
{
    OS_RETURN_E err;

    /* Init keyboard setings */
    keyboard_flags   = 0;
    display_keyboard = 1;
#if 0
    err = mutex_init(&kbd_mutex, MUTEX_FLAG_NONE,
                     MUTEX_PRIORITY_ELEVATION_NONE);
    if(err != OS_NO_ERR)
    {
        return err;
    }
#endif
    memset(&kbd_buf, 0, sizeof(kbd_buffer_t));
    /* Init interuption settings */
    err = kernel_interrupt_register_irq_handler(KBD_IRQ_LINE,
                                                keyboard_interrupt_handler);
    if(err != OS_NO_ERR)
    {
        return err;
    }
    err = kernel_interrupt_set_irq_mask(KBD_IRQ_LINE, 1);

    return err;
}

uint32_t keyboard_read(char* buffer, const uint32_t size)
{
    uint32_t    read = 0;

    if(buffer == NULL || size == 0)
    {
        return 0;
    }

    kbd_buf.char_buf  = buffer;
    kbd_buf.read_size = size;
    kbd_buf.read      = 0;
    kbd_buf.type      = 1;

    read = kbd_buf.read;

    kbd_buf.char_buf  = NULL;
    kbd_buf.read_size = 0;
    kbd_buf.read      = 0;
    kbd_buf.type      = 0;

    return read;
}

uint32_t keyboard_secure_read(char* buffer, const uint32_t size)
{
    /* Read string */
    uint32_t new_size = keyboard_read(buffer, size);

    /* Secure output */
    if(new_size < size - 1)
    {
        buffer[new_size] = '\0';
    }
    else
    {
        buffer[size - 1] = '\0';
    }

    return new_size;
}

void keyboard_getch(char* character)
{
    //OS_RETURN_E err;

    if(character == NULL)
    {
        return;
    }
#if 0
    err = mutex_pend(&kbd_mutex);
    if(err != OS_NO_ERR)
    {
        kernel_error("Keyboard cannot release its mutex[%d]\n", err);
        kernel_panic(err);
    }

    /* Set current buffer */
    err = sem_init(&kbd_buf.sem, 0);
    if(err != OS_NO_ERR)
    {
        kernel_error("Keyboard cannot create buffer semaphore[%d]\n", err);
        kernel_panic(err);
    }
#endif
    kbd_buf.char_buf  = character;
    kbd_buf.read_size = 1;
    kbd_buf.read      = 0;
    kbd_buf.type      = 2;
#if 0
    /* Wait for completion */
    err = sem_pend(&kbd_buf.sem);
    if(err != OS_NO_ERR)
    {
        kernel_error("Keyboard cannot pend buffer semaphore[%d]\n", err);
        kernel_panic(err);
    }

    /* Release resources */
    err = sem_destroy(&kbd_buf.sem);
    if(err != OS_NO_ERR)
    {
        kernel_error("Keyboard cannot delete buffer semaphore[%d]\n", err);
        kernel_panic(err);
    }
#endif
    kbd_buf.char_buf  = NULL;
    kbd_buf.read_size = 0;
    kbd_buf.read      = 0;
    kbd_buf.type      = 0;
#if 0
    err = mutex_post(&kbd_mutex);
    if(err != OS_NO_ERR)
    {
        kernel_error("Keyboard cannot release its mutex[%d]\n", err);
        kernel_panic(err);
    }
#endif
}

void keyboard_enable_secure(void)
{
    secure_input = 1;
}

void keyboard_disable_secure(void)
{
    secure_input = 0;
}

void keyboard_enable_display(void)
{
    display_keyboard = 1;
}

void keyboard_disable_display(void)
{
    display_keyboard = 0;
}