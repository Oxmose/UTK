/*******************************************************************************
 * @file kernel_output.c
 * 
 * @see kernel_output.h
 *
 * @author Alexy Torres Aurora Dugo
 *
 * @date 15/12/2017
 *
 * @version 2.0
 *
 * @brief Kernel's output methods.
 * 
 * @details Simple output functions to print messages to screen. These are 
 * really basic output too allow early kernel boot output and debug. These 
 * functions can be used in interrupts handlers since no lock is required to use
 * them. This also makes them non thread safe.
 * 
 * @copyright Alexy Torres Aurora Dugo
 ******************************************************************************/

#include <lib/string.h>   /* memset, strlen */
#include <lib/stdlib.h>   /* uitoa, itoa */
#include <serial.h>       /* Serial driver */
#include <io/graphic.h>   /* Graphic definitions */

/* UTK configuration file */
#include <config.h>

/* Header file */
#include <io/kernel_output.h>

/*******************************************************************************
 * GLOBAL VARIABLES
 ******************************************************************************/

/** @brief Stores the current output type. */
static output_t current_output;

/*******************************************************************************
 * FUNCTIONS
 ******************************************************************************/

#define PAD_SEQ                         \
{                                       \
    str_size = strlen(tmp_seq);         \
                                        \
    while(padding_mod > str_size)       \
    {                                   \
        used_output.putc(pad_char_mod); \
        --padding_mod;                  \
    }                                   \
}

/**
 * @brief Converts a string to upper case characters.
 * 
 * @details Transforms all lowercase character of a NULL terminated string to 
 * uppercase characters.
 * 
 * @param[in,out] string The string to tranform.
 */
static void toupper(char* string)
{
    /* For each character of the string */
    while(*string != 0)
    {
        /* If the character is lowercase, makes it uppercase */
        if(*string > 96 && *string < 123)
        {
            *string = *string - 32;
        }
        ++string;
    }
}

/**
 * @brief Converts a string to upper case characters.
 * 
 * @details Transforms all uppercase character of a NULL terminated string to 
 * lowercase characters.
 * 
 * @param[in,out] string The string to tranform.
 */
static void tolower(char* string)
{
    /* For each character of the string */
    while(*string != 0)
    {
        /* If the character is uppercase, makes it lowercase */
        if(*string > 64 && *string < 91)
        {
            *string = *string + 32;
        }
        ++string;
    }
}

#define GET_SEQ_VAL(val, args, length_mod)                     \
{                                                              \
                                                               \
    /* Harmonize length */                                     \
    if(length_mod > 8)                                         \
    {                                                          \
        length_mod = 8;                                        \
    }                                                          \
                                                               \
    switch(length_mod)                                         \
    {                                                          \
        case 1:                                                \
            val = (__builtin_va_arg(args, uint32_t) & 0xFF);   \
            break;                                             \
        case 2:                                                \
            val = (__builtin_va_arg(args, uint32_t) & 0xFFFF); \
            break;                                             \
        case 4:                                                \
            val = __builtin_va_arg(args, uint32_t);            \
            break;                                             \
        case 8:                                                \
            val = __builtin_va_arg(args, uint64_t);            \
            break;                                             \
        default:                                               \
           val = __builtin_va_arg(args, uint32_t);             \
    }                                                          \
                                                               \
}


/**
 * @brief Prints a formated string.
 * 
 * @details Prints a formated string to the output and managing the formated 
 * string arguments.
 * 
 * @param[in] str The formated string to output.
 * @param[in] args The arguments to use with the formated string.
 * @param[in] used_output The output to use.
 */
static void formater(const char* str, __builtin_va_list args, 
                     output_t used_output)
{
    size_t pos;
    size_t str_length;
    size_t seq_val;
    size_t str_size;

   
    uint8_t  modifier;

    uint8_t  length_mod;
    uint8_t  padding_mod;
    uint8_t  upper_mod;
    char     pad_char_mod;

    char tmp_seq[128];

    char* args_value;

    modifier     = 0;
    length_mod   = 4;
    padding_mod  = 0;
    upper_mod    = 0;
    pad_char_mod = ' ';
    str_length   = strlen(str);

    for(pos = 0; pos < str_length; ++pos)
    {
        if(str[pos] == '%')
        {
            /* If we encouter this character in a modifier sequence, it was 
             * just an escape one.
             */
            modifier = !modifier;
            if(modifier)
            {
                continue;
            }
            else 
            {
                used_output.putc(str[pos]);
            }
        }
        else if(modifier)
        {
            switch(str[pos]) 
            {
                /* Length mods */
                case 'h': 
                    length_mod /= 2;
                    continue;
                case 'l':
                    length_mod *= 2;
                    continue;

                /* Specifier mods */
                case 's':
					args_value = __builtin_va_arg(args, char*);
					current_output.puts(args_value);
                    break;
                case 'd':
                case 'i':
                    GET_SEQ_VAL(seq_val, args, length_mod);
					memset(tmp_seq, 0, sizeof(tmp_seq));
					itoa(seq_val, tmp_seq, 10);
                    PAD_SEQ
					used_output.puts(tmp_seq);
                    break;
                case 'u':
                    GET_SEQ_VAL(seq_val, args, length_mod);
					memset(tmp_seq, 0, sizeof(tmp_seq));
					uitoa(seq_val, tmp_seq, 10);
                    PAD_SEQ
					used_output.puts(tmp_seq);
                    break;
                case 'X': 
                    upper_mod = 1;
                    __attribute__ ((fallthrough));
                case 'x':
                    GET_SEQ_VAL(seq_val, args, length_mod);
					memset(tmp_seq, 0, sizeof(tmp_seq));
					uitoa(seq_val, tmp_seq, 16);
                    PAD_SEQ
                    if(upper_mod)
                    {
                        toupper(tmp_seq);
                    }
                    else 
                    {
                        tolower(tmp_seq);
                    }
					used_output.puts(tmp_seq);
                    break;                    
                case 'P': 
                    upper_mod = 1;
                    __attribute__ ((fallthrough));
                case 'p':
                    padding_mod  = 2 * sizeof(uintptr_t);
                    pad_char_mod = '0';
                    length_mod = sizeof(uintptr_t);
                    GET_SEQ_VAL(seq_val, args, length_mod);
					memset(tmp_seq, 0, sizeof(tmp_seq));
					uitoa(seq_val, tmp_seq, 16);
                    PAD_SEQ
                    if(upper_mod)
                    {
                        toupper(tmp_seq);
                    }
                    else 
                    {
                        tolower(tmp_seq);
                    }
					used_output.puts(tmp_seq);
                    break;
                case 'c':
                    length_mod = sizeof(char);
                    GET_SEQ_VAL(tmp_seq[0], args, length_mod);
					used_output.putc(tmp_seq[0]);
                    break;

                /* Padding mods */
                case '0':
                    if(padding_mod == 0)
                    {
                        pad_char_mod = '0';
                    }
                    else 
                    {
                        padding_mod *= 10;
                    }
                    continue;
                case '1':
                    padding_mod = padding_mod * 10 + 1;
                    continue;
                case '2':
                    padding_mod = padding_mod * 10 + 2;
                    continue;
                case '3':
                    padding_mod = padding_mod * 10 + 3;
                    continue;
                case '4':
                    padding_mod = padding_mod * 10 + 4;
                    continue;
                case '5':
                    padding_mod = padding_mod * 10 + 5;
                    continue;
                case '6':
                    padding_mod = padding_mod * 10 + 6;
                    continue;
                case '7':
                    padding_mod = padding_mod * 10 + 7;
                    continue;
                case '8':
                    padding_mod = padding_mod * 10 + 8;
                    continue;
                case '9':
                    padding_mod = padding_mod * 10 + 9;
                    continue;
                default:
                    continue;
            }
        }
        else 
        {
            used_output.putc(str[pos]);
        }

        /* Reinit mods */
        length_mod   = 4;
        padding_mod  = 0;
        upper_mod    = 0;
        pad_char_mod = ' ';
        modifier     = 0;
        
    }
}

/**
 * @brief Prints a formated string.
 * 
 * @details Prints a formated string to the output and managing the formated 
 * string arguments.
 * 
 * @param[in] str The formated string to output.
 * @param[in] args The arguments to use with the formated string.
 */
static void kprint_fmt(const char* str, __builtin_va_list args)
{
    formater(str, args, current_output);
}

/**
 * @brief Prints a formated string to serial output.
 * 
 * @details Prints a formated string to the serial output and managing the 
 * formated string arguments.
 * 
 * @param[in] str The formated string to output.
 * @param[in] args The arguments to use with the formated string.
 */
static void kprint_fmt_serial(const char* str, __builtin_va_list args)
{
    output_t ser_out = {
        .putc = serial_put_char,
        .puts = serial_put_string
    };

    formater(str, args, ser_out);
}

/**
 * @brief Prints the tag for kernel output functions.
 * 
 * @details Prints the tag for kernel output functions.
 * 
 * @param[in] fmt The formated string to print.
 * @param[in] ... The associated arguments to the formated string.
 */
static void tag_printf(const char* fmt, ...)
{
    __builtin_va_list args;

    if(fmt == NULL)
    {
        return;
    }
    /* Prtinf format string */
    __builtin_va_start(args, fmt);
    kprint_fmt(fmt, args);
    __builtin_va_end(args);
}

void kernel_printf(const char* fmt, ...)
{
    __builtin_va_list args;

    if(fmt == NULL)
    {
        return;
    }

    /* Prtinf format string */
    __builtin_va_start(args, fmt);
    current_output.putc = graphic_put_char;
    current_output.puts = graphic_put_string;
    kprint_fmt(fmt, args);
    __builtin_va_end(args);
}

void kernel_error(const char* fmt, ...)
{
    __builtin_va_list args;
    colorscheme_t     buffer;
    colorscheme_t     new_scheme;

    if(fmt == NULL)
    {
        return;
    }

    new_scheme.foreground = FG_RED;
    new_scheme.background = BG_BLACK;
    new_scheme.vga_color  = 1;

    current_output.putc = graphic_put_char;
    current_output.puts = graphic_put_string;

    /* No need to test return value */
    graphic_save_color_scheme(&buffer);

    /* Set REG on BLACK color scheme */
    graphic_set_color_scheme(new_scheme);

    /* Print tag */
    tag_printf("[ERROR] ");

    /* Restore original screen color scheme */
    graphic_set_color_scheme(buffer);

    /* Printf format string */
    __builtin_va_start(args, fmt);
    kprint_fmt(fmt, args);
    __builtin_va_end(args);
}

void kernel_success(const char* fmt, ...)
{
    __builtin_va_list    args;
    colorscheme_t        buffer;
    colorscheme_t        new_scheme;

    if(fmt == NULL)
    {
        return;
    }

    new_scheme.foreground = FG_GREEN;
    new_scheme.background = BG_BLACK;
    new_scheme.vga_color  = 1;

    current_output.putc = graphic_put_char;
    current_output.puts = graphic_put_string;

    /* No need to test return value */
    graphic_save_color_scheme(&buffer);

    /* Set REG on BLACK color scheme */
    graphic_set_color_scheme(new_scheme);

    /* Print tag */
    tag_printf("[OK] ");

    /* Restore original screen color scheme */
    graphic_set_color_scheme(buffer);

    /* Printf format string */
    __builtin_va_start(args, fmt);
    kprint_fmt(fmt, args);
    __builtin_va_end(args);
}

void kernel_info(const char* fmt, ...)
{
    __builtin_va_list    args;
    colorscheme_t        buffer;
    colorscheme_t        new_scheme;

    if(fmt == NULL)
    {
        return;
    }

    new_scheme.foreground = FG_CYAN;
    new_scheme.background = BG_BLACK;
    new_scheme.vga_color  = 1;

    current_output.putc = graphic_put_char;
    current_output.puts = graphic_put_string;

    /* No need to test return value */
    graphic_save_color_scheme(&buffer);

    /* Set REG on BLACK color scheme */
    graphic_set_color_scheme(new_scheme);

    /* Print tag */
    tag_printf("[INFO] ");

    /* Restore original screen color scheme */
    graphic_set_color_scheme(buffer);

    /* Printf format string */
    __builtin_va_start(args, fmt);
    kprint_fmt(fmt, args);
    __builtin_va_end(args);
}

void kernel_debug(const char* fmt, ...)
{
    __builtin_va_list    args;
    colorscheme_t        buffer;
    colorscheme_t        new_scheme;

    if(fmt == NULL)
    {
        return;
    }

    new_scheme.foreground = FG_YELLOW;
    new_scheme.background = BG_BLACK;
    new_scheme.vga_color  = 1;

    current_output.putc = graphic_put_char;
    current_output.puts = graphic_put_string;

    /* No need to test return value */
    graphic_save_color_scheme(&buffer);

    /* Set REG on BLACK color scheme */
    graphic_set_color_scheme(new_scheme);

    /* Print tag */
    tag_printf("[DEBUG] ");

    /* Restore original screen color scheme */
    graphic_set_color_scheme(buffer);

    /* Printf format string */
    __builtin_va_start(args, fmt);
    kprint_fmt(fmt, args);
    __builtin_va_end(args);
}

void kernel_serial_debug(const char* fmt, ...)
{
    __builtin_va_list args;

    if(fmt == NULL)
    {
        return;
    }

    __builtin_va_start(args, fmt);
    kprint_fmt_serial("[DEBUG] ", args);

    kprint_fmt_serial(fmt, args);
    __builtin_va_end(args);

}

void kernel_doprint(const char* str, __builtin_va_list args)
{
    if(str == NULL)
    {
        return;
    }

    current_output.putc = graphic_put_char;
    current_output.puts = graphic_put_string;
    kprint_fmt(str, args);
}
