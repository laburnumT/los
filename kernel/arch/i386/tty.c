#include <kernel/tty.h>

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include <string.h>

#include "vga.h"

static const size_t VGA_WIDTH             = 80;
static const size_t VGA_HEIGHT            = 25;
static struct vga_entry *const VGA_MEMORY = (struct vga_entry *)0xB8000;

static size_t terminal_row;
static size_t terminal_column;
static struct vga_entry_colour terminal_colour;
static struct vga_entry *terminal_buffer;

void tty_initialize(void)
{
        terminal_row    = 0;
        terminal_column = 0;
        terminal_colour =
            vga_entry_colour(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK, 0);
        terminal_buffer = VGA_MEMORY;
        for (size_t y = 0; y < VGA_HEIGHT; y++) {
                for (size_t x = 0; x < VGA_WIDTH; x++) {
                        const size_t index = y * VGA_WIDTH + x;
                        terminal_buffer[index] =
                            vga_entry(' ', terminal_colour);
                }
        }
}

static void tty_putentryat(unsigned char c, struct vga_entry_colour colour,
                           size_t x, size_t y)
{
        const size_t index     = y * VGA_WIDTH + x;
        terminal_buffer[index] = vga_entry(c, colour);
}

void tty_putchar(char c)
{
        switch (c) {
        case '\n':
                ++terminal_row;
                terminal_column = 0;
                break;
        case '\t':
                terminal_column += 4;
                break;
        default:
                tty_putentryat((unsigned char)c,
                               terminal_colour,
                               terminal_column,
                               terminal_row);
                ++terminal_column;
                break;
        }
        if (terminal_column == VGA_WIDTH) {
                terminal_column = 0;
                ++terminal_row;
        }
        if (terminal_row == VGA_HEIGHT) {
                --terminal_row;
                for (size_t row = 1; row < VGA_HEIGHT; ++row) {
                        const size_t index_dest = (row - 1) * VGA_WIDTH;
                        const size_t index_src  = row * VGA_WIDTH;
                        memcpy(terminal_buffer + index_dest,
                               terminal_buffer + index_src,
                               VGA_WIDTH);
                }
                for (size_t x = 0; x < VGA_WIDTH; x++) {
                        const size_t index = (VGA_HEIGHT - 1) * VGA_WIDTH + x;
                        terminal_buffer[index] =
                            vga_entry(' ', terminal_colour);
                }
        }
}

void tty_write(const char *data, size_t size)
{
        for (size_t i = 0; i < size; ++i)
                tty_putchar(data[i]);
}

void tty_writestring(const char *data)
{
        tty_write(data, strlen(data));
}
