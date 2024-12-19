#ifndef LOS_KERNEL_ARCH_I386_VGA_H
#define LOS_KERNEL_ARCH_I386_VGA_H

#include <stdint.h>

enum vga_colour {
        VGA_COLOR_BLACK         = 0,
        VGA_COLOR_BLUE          = 1,
        VGA_COLOR_GREEN         = 2,
        VGA_COLOR_CYAN          = 3,
        VGA_COLOR_RED           = 4,
        VGA_COLOR_MAGENTA       = 5,
        VGA_COLOR_BROWN         = 6,
        VGA_COLOR_LIGHT_GREY    = 7,
        VGA_COLOR_DARK_GREY     = 8,
        VGA_COLOR_LIGHT_BLUE    = 9,
        VGA_COLOR_LIGHT_GREEN   = 10,
        VGA_COLOR_LIGHT_CYAN    = 11,
        VGA_COLOR_LIGHT_RED     = 12,
        VGA_COLOR_LIGHT_MAGENTA = 13,
        VGA_COLOR_LIGHT_BROWN   = 14,
        VGA_COLOR_WHITE         = 15,
};

struct vga_entry_colour {
        uint8_t foreground : 4;
        uint8_t background : 3;
        uint8_t blink : 1;
};

struct vga_entry {
        uint8_t c;
        struct vga_entry_colour colour;
};

inline static struct vga_entry_colour
vga_entry_colour(enum vga_colour fg, enum vga_colour bg, uint8_t blink)
{
        return (struct vga_entry_colour){
            .foreground = fg, .background = bg, .blink = blink};
}

inline static struct vga_entry vga_entry(unsigned char uc,
                                         struct vga_entry_colour color)
{
        return (struct vga_entry){
            .c      = uc,
            .colour = color,
        };
}

#endif
