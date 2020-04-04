#include <interrupt/interrupts.h>
#include <core/panic.h>
#include <io/kernel_output.h>
#include <cpu.h>
#include <vesa.h>
#include <Tests/test_bank.h>

#if VESA_TEXT_TEST == 1
unsigned int rgb(double hue)
{
    int h = (int)(hue * 256 * 6);
    int x = h % 0x100;

    int r = 0, g = 0, b = 0;
    switch (h / 256)
    {
    case 0: r = 255; g = x;       break;
    case 1: g = 255; r = 255 - x; break;
    case 2: g = 255; b = x;       break;
    case 3: b = 255; g = 255 - x; break;
    case 4: b = 255; r = x;       break;
    case 5: r = 255; b = 255 - x; break;
    }

    return r + (g << 8) + (b << 16);
}

void vesa_text_test(void)
{    
    vesa_put_string("[TESTMODE]");
    for(uint16_t i = 32; i < 127; ++i)
    {
        vesa_put_char((char)i);
    }

    vesa_put_string("\n[TESTMODE]");
    for(uint16_t i = 0; i < 256; ++i)
    {
        colorscheme_t color = {
            .vga_color = 1,
            .foreground = i & 0x0F,
            .background = i & 0xF0
        };
        vesa_set_color_scheme(color);
        vesa_put_char('A');
    }

    uint32_t x = 0;
    uint32_t y = 180;
    uint32_t width = vesa_get_screen_width();
    double range = vesa_get_screen_width();
    for(uint32_t j = 0; j < 256; ++j)
    {
        for (double i = 0; i < range; i++)
        {
            unsigned int color = rgb(i / range);
            vesa_draw_pixel(x, y, j, color & 0xFF, (color & 0xFF00) >> 8, (color & 0xFF0000) >> 16);
            if(++x == width)
            {
                ++y;
                x = 0;
            }
        }
    }

    for(uint32_t j = 0; j < 256; ++j)
    {
        for (double i = 0; i < range; i++)
        {
            unsigned int color = rgb(i / range);
            uint32_t r = color & 0xFF;
            uint32_t g = (color & 0xFF00) >> 8;
            uint32_t b = (color & 0xFF0000) >> 16;

            r = (r + j > 255) ? 255 : r + j;
            g = (g + j > 255) ? 255 : g + j;
            b = (b + j > 255) ? 255 : b + j;
            vesa_draw_pixel(x, y, 0xFF, r, g, b);
            if(++x == width)
            {
                ++y;
                x = 0;
            }
        }
    }

    /* Kill QEMU */
    cpu_outw(0x2000, 0x604);
    cpu_outw(0x2000, 0xB004);
    while(1)
    {
        __asm__ ("hlt");
    }
}
#else
void vesa_text_test(void)
{

} 
#endif