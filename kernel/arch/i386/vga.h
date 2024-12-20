#ifndef LOS_KERNEL_ARCH_I386_VGA_H
#define LOS_KERNEL_ARCH_I386_VGA_H

#include <stddef.h>
#include <stdint.h>

static const size_t VGA_WIDTH  = 80;
static const size_t VGA_HEIGHT = 25;
static volatile struct vga_entry *const VGA_MEMORY =
    (struct vga_entry *)0xB8000;
static const uint16_t VGA_ADDRESS_REGISTER = 0x3D4;
static const uint16_t VGA_DATA_REGISTER    = 0x3D5;

enum vga_register_index : uint8_t {
        VGA_HORIZONTAL_TOTAL_REGISTER          = 0x00,
        VGA_END_HORIZONTAL_DISPLAY_REGISTER    = 0x01,
        VGA_START_HORIZONTAL_BLANKING_REGISTER = 0x02,
        VGA_END_HORIZONTAL_BLANKING_REGISTER   = 0x03,
        VGA_CURSOR_START_REGISTER              = 0x0A,
        VGA_CURSOR_END_REGISTER                = 0x0B,
        VGA_CURSOR_LOCATION_HIGH_REGISTER      = 0x0E,
        VGA_CURSOR_LOCATION_LOW_REGISTER       = 0x0F,
};

enum vga_colour : uint8_t {
        VGA_COLOUR_BLACK         = 0,
        VGA_COLOUR_BLUE          = 1,
        VGA_COLOUR_GREEN         = 2,
        VGA_COLOUR_CYAN          = 3,
        VGA_COLOUR_RED           = 4,
        VGA_COLOUR_MAGENTA       = 5,
        VGA_COLOUR_BROWN         = 6,
        VGA_COLOUR_LIGHT_GREY    = 7,
        VGA_COLOUR_DARK_GREY     = 8,
        VGA_COLOUR_LIGHT_BLUE    = 9,
        VGA_COLOUR_LIGHT_GREEN   = 10,
        VGA_COLOUR_LIGHT_CYAN    = 11,
        VGA_COLOUR_LIGHT_RED     = 12,
        VGA_COLOUR_LIGHT_MAGENTA = 13,
        VGA_COLOUR_LIGHT_BROWN   = 14,
        VGA_COLOUR_WHITE         = 15,
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
