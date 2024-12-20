#ifndef LOS_KERNEL_ARCH_I386_VGA_H
#define LOS_KERNEL_ARCH_I386_VGA_H

#include <stddef.h>
#include <stdint.h>

static const size_t VGA_WIDTH  = 80;
static const size_t VGA_HEIGHT = 25;
static volatile struct vga_entry *const VGA_MEMORY =
    (struct vga_entry *)0xB8000;

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

struct vga_entry {
        uint8_t c;
        uint8_t colour;
};

inline static uint8_t vga_entry_colour(enum vga_colour fg, enum vga_colour bg,
                                       uint8_t blink)
{
        return (blink << 7) | (bg << 4) | fg;
}

inline static struct vga_entry vga_entry(unsigned char uc, uint8_t colour)
{
        return (struct vga_entry){
            .c      = uc,
            .colour = colour,
        };
}

#endif
