/*******************************************************************************
 * @file keyboard.h
 * 
 * @see keyboard.c
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

#ifndef __X86_KEYBOARD_H_
#define __X86_KEYBOARD_H_

#include <lib/stdint.h> /* Generic int types */
#include <lib/stddef.h> /* Standard definitions */

/*******************************************************************************
 * CONSTANTS
 ******************************************************************************/

/** @brief Keyboard's CPU command port. */
#define KEYBOARD_COMM_PORT      0x64
/** @brief Keyboard's CPU data port. */
#define KEYBOARD_DATA_PORT      0x60

/** @brief Kayboard's input buffer size in bytes. */
#define KEYBOARD_BUFFER_SIZE 512

/** @brief Keyboard specific key code: backspace. */
#define KEY_BACKSPACE                   '\b'
/** @brief Keyboard specific key code: tab. */
#define KEY_TAB                         '\t'
/** @brief Keyboard specific key code: return. */
#define KEY_RETURN                     '\n'
/** @brief Keyboard specific key code: left shift. */
#define KEY_LSHIFT                      0x0400
/** @brief Keyboard specific key code: right shift. */
#define KEY_RSHIFT                      0x0500

/** @brief Keyboard specific key code: left shift. */
#define KBD_LSHIFT 0x00000001
/** @brief Keyboard specific key code: right shift. */
#define KBD_RSHIFT 0x00000002

/** @brief Keyboard function key: F1. */
#define KEY_F1                          0x82
/** @brief Keyboard function key: F2. */
#define KEY_F2                          0x82
/** @brief Keyboard function key: F3. */
#define KEY_F3                          0x82
/** @brief Keyboard function key: F4. */
#define KEY_F4                          0x82
/** @brief Keyboard function key: F5. */
#define KEY_F5                          0x82
/** @brief Keyboard function key: F6. */
#define KEY_F6                          0x82
/** @brief Keyboard function key: F7. */
#define KEY_F7                          0x82
/** @brief Keyboard function key: F8. */
#define KEY_F8                          0x82
/** @brief Keyboard function key: F9. */
#define KEY_F9                          0x82
/** @brief Keyboard function key: F10. */
#define KEY_F10                         0x82
/** @brief Keyboard function key: F11. */
#define KEY_F11                         0x82
/** @brief Keyboard function key: F12. */
#define KEY_F12                         0x82

/** @brief Keyboard specific key code: page up. */
#define KEY_PGUP                        0x80
/** @brief Keyboard specific key code: page down. */
#define KEY_PGDOWN                      0x81

/*******************************************************************************
 * STRUCTURES
 ******************************************************************************/

/** @brief Keyboark code to key mapping. */
struct key_mapper
{
    /** @brief Regular mapping. */
    uint16_t regular[128];
    /** @brief Maj mapping. */
    uint16_t shifted[128];
};

/** 
 * @brief Defines key_mapper type as a shorcut for struct key_mapper_t.
 */
typedef struct key_mapper key_mapper_t;

/** @brief Keyboard input buffer definition, */
struct kbd_buffer
{
    /** @brief Current buffer mode (0: not initialized, 1: read, 2: getchar). */
    int32_t     type;
    /** @brief Character array as buffer. */
    char*       char_buf;
    /** @brief Number of characters to read. */
    uint32_t    read_size;
    /** @brief Number of characters already read. */
    uint32_t    read;
};

/** 
 * @brief Defines kbd_buffer_t type as a shorcut for struct kbd_buffer.
 */
typedef struct kbd_buffer kbd_buffer_t;

/*******************************************************************************
 * FUNCTIONS
 ******************************************************************************/

/**
 * @brief Initializes the keyboard.
 * 
 * @details Initialize keyboard settings, structures and sets the interrupt
 * manager for the keyboard.
 *
 * @return The error or success state.
 */
OS_RETURN_E keyboard_init(void);

/**
 * @brief Fills the buffer with at maximum size characters.
 * @details Fills the buffer with keyboard buffer. This function is blocking 
 * while the keyboard buffer is empty.
 *
 * @param[out] buffer The bufer to fill with the user input.
 * @param[in] size The maximum size of the buffer.
 * 
 * @return The actual number of characters read.
 */
uint32_t keyboard_read(char* buffer, const size_t size);

/**
 * @brief Fills the buffer with at maximum size characters.
 * @details Fills the buffer with keyboard buffer. This function is blocking 
 * while the keyboard buffer is empty. The function NULL terminate the string 
 * read.
 *
 * @param[out] buffer The bufer to fill with the user input.
 * @param[in] size The maximum size of the buffer.
 * 
 * @return The actual number of characters read.
 */
uint32_t keyboard_secure_read(char* buffer, const size_t size);

/**
 * @brief Reads one character from the keyboard.
 * 
 * @details The function will read one character from the kaybard of the 
 * keyboard bufffer. The function is blocking if no character can be read.
 *
 * @param[out] character The buffer to write the character to.
 */
void keyboard_getch(char* character);

/**
 * @brief Enables keyboard secure input.
 * @details Enables the keyboard seure input feature: replaces the input with
 * '*' character.
 */
void keyboard_enable_secure(void);

/**
 * @brief Disables keyboard secure input.
 * @details Disables the keyboard seure input feature.
 */
void keyboard_disable_secure(void);

/**
 * @brief Enables keyboard character display.
 * @details Enables keyboard character display: displays the keyboard input to 
 * the console.
 */
void keyboard_enable_display(void);

/**
 * @brief Disables keyboard character display.
 * @details Disables keyboard character display: hides the keyboard input from 
 * the console.
 */
void keyboard_disable_display(void);

#endif /* #ifndef __X86_KEYBOARD_H_ */