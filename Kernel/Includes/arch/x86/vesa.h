/*******************************************************************************
 * @file vesa.h
 *
 * @see vesa.c
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

#ifndef __X86_VESA_H_
#define __X86_VESA_H_

#include <lib/stdint.h> /* Generic int types */
#include <lib/stddef.h> /* Standard definitions */
#include <io/graphic.h> /* Graphic API */

/*******************************************************************************
 * CONSTANTS
 ******************************************************************************/

/** @brief VESA BIOS interrupt id. */
#define BIOS_INTERRUPT_VESA 0x10

/** @brief VESA BIOS get VBE information command id. */
#define BIOS_CALL_GET_VESA_INFO 0x4F00
/** @brief VESA BIOS get mode command id. */
#define BIOS_CALL_GET_VESA_MODE 0x4F01
/** @brief VESA BIOS set mode command id. */
#define BIOS_CALL_SET_VESA_MODE 0x4F02

/** @brief VESA mode information flag: linear framebuffer. */
#define VESA_FLAG_LINEAR_FB  0x90
/** @brief VESA mode command: enable linear framebuffer. */
#define VESA_FLAG_LFB_ENABLE 0x4000

/** @brief Maximal number of VESA mode supported by the kernel. */
#define MAX_VESA_MODE_COUNT 245

/** @brief Defines the tabulation space width. */
#define TAB_WIDTH 4

/*******************************************************************************
 * STRUCTURES
 ******************************************************************************/

/** @brief VBE information structure, see the VBE standard for more information
 * about the contained data.
 */
struct vbe_info_structure
{
    char     signature[4];
    uint16_t version;
    uint32_t oem;
    uint32_t capabilities;
    uint32_t video_modes;
    uint16_t video_memory;
    uint16_t software_rev;
    uint32_t vendor;
    uint32_t product_name;
    uint32_t product_rev;
    uint8_t  reserved[222];
    uint8_t  oem_data[256];
} __attribute__((packed));

/**
 * @brief Defines vbe_info_structure_t type as a shorcut for struct
 * vbe_info_structure.
 */
typedef struct vbe_info_structure vbe_info_structure_t;

/** @brief VBE mode information structure, see the VBE standard for more
 * information about the contained data.
 */
struct vbe_mode_info_structure
{
    uint16_t attributes;
    uint8_t  window_a;
    uint8_t  window_b;
    uint16_t granularity;
    uint16_t window_size;
    uint16_t segment_a;
    uint16_t segment_b;
    uint32_t win_func_ptr;
    uint16_t pitch;
    uint16_t width;
    uint16_t height;
    uint8_t  w_char;
    uint8_t  y_char;
    uint8_t  planes;
    uint8_t  bpp;
    uint8_t  banks;
    uint8_t  memory_model;
    uint8_t  bank_size;
    uint8_t  image_pages;
    uint8_t  reserved0;

    uint8_t  red_mask;
    uint8_t  red_position;
    uint8_t  green_mask;
    uint8_t  green_position;
    uint8_t  blue_mask;
    uint8_t  blue_position;
    uint8_t  reserved_mask;
    uint8_t  reserved_position;
    uint8_t  direct_color_attributes;

    uint32_t framebuffer;
    uint32_t off_screen_mem_off;
    uint16_t off_screen_mem_size;
    uint8_t  reserved1[206];
} __attribute__((packed));
/**
 * @brief Defines vbe_info_structure_t type as a shorcut for struct
 * vbe_mode_info_structure.
 */
typedef struct vbe_mode_info_structure vbe_mode_info_structure_t;

/** @brief Kernel's representation of a VBE mode. It is used to chain modes in a
 * linked list.
 */
struct vesa_mode
{
    /** @brief The mode resolution's width. */
    uint16_t width;
    /** @brief The mode resolution's height. */
    uint16_t height;
    /** @brief The mode's color depth. */
    uint16_t bpp;
    /** @brief The mode's id. */
    uint16_t mode_id;

    /** @brief Start of the physical address of the mode's framebuffer. */
    void* framebuffer_phy;
    /** @brief Start of the virtual address of the mode's framebuffer. */
    void* framebuffer;

    /** @brief Next mode in the list. */
    struct vesa_mode* next;
};

/**
 * @brief Defines vesa_mode_t type as a shorcut for struct vesa_mode.
 */
typedef struct vesa_mode vesa_mode_t;

/** @brief User's representation of a VBE mode. */
struct vesa_mode_info
{
    /** @brief The mode resolution's width. */
    uint16_t width;
    /** @brief The mode resolution's height. */
    uint16_t height;
    /** @brief The mode's color depth. */
    uint16_t bpp;
    /** @brief The mode's id. */
    uint16_t mode_id;
};

/**
 * @brief Defines vesa_mode_info_t type as a shorcut for struct vesa_mode_info.
 */
typedef struct vesa_mode_info vesa_mode_info_t;

/*******************************************************************************
 * FUNCTIONS
 ******************************************************************************/

/**
 * @brief Initializes the VESA driver.
 *
 * @details Initializes the VESA driver, sets its structures and gathers
 * available VESA modes.
 *
 * @return The success state or the error code.
 * - OS_NO_ERR is returned if no error is encountered.
 * - OS_ERR_VESA_NOT_SUPPORTED if the graphic driver is cannot handle VESA on
 * the system.
 */
OS_RETURN_E vesa_init(void);

/**
 * @brief Switch fror VGA text mode to VESA mode.
 *
 * @details Switch fror VGA text mode to VESA mode. The function will copy the
 * content of the VGA framebuffer to write it on the screen after the video mode
 * switch.
 *
 * @return The success state or the error code.
 * - OS_NO_ERR is returned if no error is encountered.
 * - OS_ERR_VESA_NOT_SUPPORTED if the graphic driver is cannot handle VESA on
 * the system.
 */
OS_RETURN_E vesa_text_vga_to_vesa(void);

/**
 * @brief Returns the number of VESA modes detected in the system.
 *
 * @details Returns the number of VESA modes detected in the system.
 *
 * @return The  number of VESA modes detected in the system.
 */
uint16_t vesa_get_vesa_mode_count(void);

/**
 * @brief Fills a buffer with the available VESA modes.
 *
 * @details Fills a buffer given as parameter with the list of all VESA modes
 * detected in the system. If the buffer is too big, the function just fills
 * the needed space.If the buffer is too small, the function will stop filling
 * the buffer when the parameter size is reached.
 *
 * @param[out] buffer The buffer that needs to be filled with the data.
 * @param[in] size The size of the buffer in number of elements.
 *
 * @return The success state or the error code.
 * - OS_NO_ERR is returned if no error is encountered.
 * - OS_ERR_VESA_NOT_SUPPORTED if the graphic driver is cannot handle VESA on
 * the system.
 * - OS_ERR_NULL_POINTER if the buffer to fill is NULL.
 */
OS_RETURN_E vesa_get_vesa_modes(vesa_mode_info_t* buffer, const uint32_t size);

/**
 * @brief Sets a new VESA mode.
 *
 * @details Sets the VESA mode given as parameter. If the VESA mode is not
 * correct or does not correspond to any mode detected in the system, the
 * function willreturn an error. To get the list of all available modes please
 * check the vesa_get_vesa_modes function.
 *
 * @param[in] mode The VESA mode to be set.
 *
 * @return The success state or the error code.
 * - OS_NO_ERR is returned if no error is encountered.
 * - OS_ERR_VESA_NOT_SUPPORTED if the graphic driver is cannot handle VESA on
 * the system.
 * - OS_ERR_VESA_MODE_NOT_SUPPORTED if the desired mode is not supported by the
 * system.
 */
OS_RETURN_E vesa_set_vesa_mode(const vesa_mode_info_t mode);

/**
 * @brief Gets the value of the pixel placed at the given coordinates.
 *
 * @details Gets the value of the pixel placed at the given coordinates. The top
 * left hand corner of the screen has coordinates x = 0 and y = 0. The color of
 * the pixel is expressed in the bpp format of the current set mode.
 *
 * @param[in] x The x coordinate of the pixel.
 * @param[in] y The y coordinate of the pixel.
 * @param[out] alpha The alpha component of the pixel buffer.
 * @param[out] red The red component of the pixel buffer.
 * @param[out] green The green component of the pixel buffer.
 * @param[out] blue The blue component of the pixel buffer.
 *
 * @return The success state or the error code.
 * - OS_NO_ERR is returned if no error is encountered.
 * - OS_ERR_VESA_NOT_SUPPORTED if the graphic driver is cannot handle VESA on
 *   the system.
 * - OS_ERR_VESA_NOT_INIT if the VESA driver has not been initialized before
 *   calling this function.
 * - OS_ERR_OUT_OF_BOUND if the given coordinates are out of the screen bounds.
 * - OS_ERR_NULL_POINTER if one of the value buffer (a, r, g, b) is NULL.
 */
OS_RETURN_E vesa_get_pixel(const uint16_t x, const uint16_t y,
                           uint8_t* alpha, uint8_t* red,
                           uint8_t* green, uint8_t* blue);

/**
 * @brief Draws a pixel on the screen.
 *
 * @details Draws a pixel on the screen at the given coordinates. The top left
 * hand corner of the screen has coordinates x = 0 and y = 0. The color of the
 * pixel should be expressed in the bpp format of the current set mode.
 *
 * @param[in] x The x coordinate of the pixel.
 * @param[in] y The y coordinate of the pixel.
 * @param[in] alpha The alpha component of the pixel.
 * @param[in] red The red component of the pixel.
 * @param[in] green The green component of the pixel.
 * @param[in] blue The blue component of the pixel.
 *
 * @return The success state or the error code.
 * - OS_NO_ERR is returned if no error is encountered.
 * - OS_ERR_VESA_NOT_SUPPORTED if the graphic driver is cannot handle VESA on
 *   the system.
 * - OS_ERR_VESA_NOT_INIT if the VESA driver has not been initialized before
 *   calling this function.
 * - OS_ERR_OUT_OF_BOUND if the given coordinates are out of the screen bounds.
 */
OS_RETURN_E vesa_draw_pixel(const uint16_t x, const uint16_t y,
                            const uint8_t alpha, const uint8_t red,
                            const uint8_t green, const uint8_t blue);

/**
 * @brief Draws a rectngle on the screen.
 *
 * @details Draws a rectangle on the screen at the given coordinates. The top
 * left hand corner of the screen has coordinates x = 0 and y = 0. The color of
 * the rectangle should be expressed in the bpp format of the current set mode.
 *
 * @param[in] x The x coordinate of the pixel.
 * @param[in] y The y coordinate of the pixel.
 * @param[in] width The width in pixels of the rectangle.
 * @param[in] height The height in pixels of the rectangle.
 * @param[in] alpha The alpha component of the pixel.
 * @param[in] red The red component of the pixel.
 * @param[in] green The green component of the pixel.
 * @param[in] blue The blue component of the pixel.
 *
 * @return The success state or the error code.
 * - OS_NO_ERR is returned if no error is encountered.
 * - OS_ERR_VESA_NOT_SUPPORTED if the graphic driver is cannot handle VESA on
 *   the system.
 * - OS_ERR_VESA_NOT_INIT if the VESA driver has not been initialized before
 *   calling this function.
 * - OS_ERR_OUT_OF_BOUND if the given coordinates are out of the screen bounds.
 */
OS_RETURN_E vesa_draw_rectangle(const uint16_t x, const uint16_t y,
                                const uint16_t width, const uint16_t height,
                                const uint8_t alpha, const uint8_t red,
                                const uint8_t green, const uint8_t blue);

/**
 * @brief Draws a character on the screen.
 *
 * @details Draw a character on the screen at the given coordinates. The top
 * left hand corner of the screen has coordinates x = 0 and y = 0. The
 * coordinates reffer to the top left hand corner of the character.
 *
 * @param[in] character The character to write.
 * @param[in] x The x coordinate of the character.
 * @param[in] y The y coordinate of the character.
 * @param[in] fgcolor The foreground color of the character.
 * @param[in] bgcolor The background color of the character.
 *
 * @return The success state or the error code.
 * - OS_NO_ERR is returned if no error is encountered.
 * - OS_ERR_VESA_NOT_SUPPORTED if the graphic driver is cannot handle VESA on
 *   the system.
 * - OS_ERR_VESA_NOT_INIT if the VESA driver has not been initialized before
 *   calling this function.
 * - OS_ERR_OUT_OF_BOUND if the given coordinates are out of the screen bounds.
 */
void vesa_drawchar(const unsigned char character,
                   const uint32_t x, const uint32_t y,
                   const uint32_t fgcolor, const uint32_t bgcolor);

/**
 * @brief Returns the current resolution's width.
 *
 * @details Returns the current VESA mode screen width. 0 is returned if VESA
 * is not supported, initialized or used.
 *
 * @returns The current resolution's width.
 */
uint32_t vesa_get_screen_width(void);

/**
 * @brief Returns the current resolution's height.
 *
 * @details Returns the current VESA mode screen height. 0 is returned if VESA
 * is not supported, initialized or used.
 *
 * @returns The current resolution's height.
 */
uint32_t vesa_get_screen_height(void);

/**
 * @brief Returns the current resolution's color depth.
 *
 * @details Returns the current VESA mode screen color depth. 0 is returned if
 * VESA is not supported, initialized or used.
 *
 * @returns The current resolution's color depth.
 */
uint8_t vesa_get_screen_bpp(void);

/**
 * @brief Clears the screen, the background color is set to black.
 */
void vesa_clear_screen(void);

/**
 * @brief Places the cursor to the coordinates given as parameters.
 *
 * @details The function places the screen cursor at the desired coordinates
 * based on the line and column parameter.
 *
 * @param[in] line The line index where to place the cursor.
 * @param[in] column The column index where to place the cursor.
 *
 * @return The success state or the error code.
 * - OS_NO_ERR is returned if no error is encountered.
 * - OS_ERR_OUT_OF_BOUND is returned if the parameters are out of bound.
 */
OS_RETURN_E vesa_put_cursor_at(const uint32_t line, const uint32_t column);

/**
 * @brief Saves the cursor attributes in the buffer given as paramter.
 *
 * @details Fills the buffer given s parameter with the current value of the
 * cursor.
 *
 * @param[out] buffer The cursor buffer in which the current cursor position is
 * going to be saved.
 *
 * @return The success state or the error code.
 * - OS_NO_ERR is returned if no error is encountered.
 * - OS_ERR_NULL_POINTER is returned if the buffer pointer is NULL.
 */
OS_RETURN_E vesa_save_cursor(cursor_t* buffer);

/**
 * @brief Restores the cursor attributes from the buffer given as parameter.
 *
 * @details The function will restores the cursor attributes from the buffer
 * given as parameter.
 *
 * @param[in] buffer The buffer containing the cursor's attributes.
 *
 * @return The success state or the error code.
 * - OS_NO_ERR is returned if no error is encountered.
 * - OS_ERR_UNAUTHORIZED_ACTION is returned if the positions of the buffer are
 * out of bound.
 */
OS_RETURN_E vesa_restore_cursor(const cursor_t buffer);

/**
 * @brief Scrolls in the desired direction of lines_count lines.
 *
 * @details The function will scroll of lines_count line in the desired
 * direction.
 *
 * @param[in] direction The direction to which the screen should be scrolled.
 * @param[in] lines_count The number of lines to scroll.
 */
void vesa_scroll(const SCROLL_DIRECTION_E direction,
                 const uint32_t lines_count);

/**
 * @brief Sets the color scheme of the screen.
 *
 * @details Replaces the curent color scheme used t output data with the new
 * one given as parameter.
 *
 * @param[in] color_scheme The new color scheme to apply to the screen.
 */
void vesa_set_color_scheme(const colorscheme_t color_scheme);

/**
 * @brief Saves the color scheme in the buffer given as parameter.
 *
 * @details Fills the buffer given as parameter with the current screen's
 * color scheme value.
 *
 * @param[out] buffer The buffer that will receive the current color scheme used
 * by the screen.
 *
 * @return The success state or the error code.
 * - OS_NO_ERR is returned if no error is encountered.
 * - OS_ERR_NULL_POINTER is returned if the buffer pointer is NULL.
 */
OS_RETURN_E vesa_save_color_scheme(colorscheme_t* buffer);

/**
 * ­@brief Put a string to screen.
 *
 * @details The function will display the string given as parameter to the
 * screen.
 *
 * @param[in] str The string to display on the screen.
 *
 * @warning string must be NULL terminated.
 */
void vesa_put_string(const char* str);

/**
 * ­@brief Put a character to screen.
 *
 * @details The function will display the character given as parameter to the
 * screen.
 *
 * @param[in] character The char to display on the screen.
 */
void vesa_put_char(const char character);

/**
 * @brief Used by the kernel to display strings on the screen from a keyboard
 * input.
 *
 * @details Display a character from the keyboard input. This allows
 * the kernel to know these character can be backspaced later.
 *
 * @param[in] str The string to display on the screen from a keybaord input.
 * @param[in] len The length of the string to display.
 */
void vesa_console_write_keyboard(const char* str, const size_t len);

/**
 * @brief Fills the screen with the buffer given as parameter.
 *
 * @details  Fill the entire screen with the content of the buffer given as
 * parameter. The buffer should be se size of the video buffer.
 *
 * @warning The buffer should be se size of the video buffer.
 *
 * @param[in] pointer The pointer to the buffer to copy to the video memory.
 */
void vesa_fill_screen(const void* pointer);


/**
 * @brief Flushes the buffer to the graphic card.
 *
 * @details  Flushes the buffer to the graphic card. The virtual buffer
 * will be copied to the hardware buffer.
 */
void vesa_flush_buffer(void);

/**
 * @brief Enables or disables transparent background for characters.
 *
 * @details Enables or disables transparent background for characters.
 *
 * @param[in] enabled If set to other than 0, characters will be printed with
 * a transparent background.
 */
void vesa_set_transparent_char(const uint32_t enabled);

/**
 * @brief VESA buffered thread. 
 *
 * @details VESA buffered thread. Copies the virtual bufer into the hardware
 * buffer. 
 * 
 * @param[in] args Unused.
 * 
 * @return Never returns, NULL in case of bad return.
 */
void* vesa_double_buffer_thread(void* args);

#endif /* #ifndef __X86_VESA_H_ */