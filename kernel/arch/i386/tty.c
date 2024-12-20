#include <kernel/tty.h>

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include <string.h>

#include "port.h"
#include "vga.h"

static size_t terminal_row;
static size_t terminal_column;
static uint8_t terminal_colour;
static volatile struct vga_entry *terminal_buffer;

static void tty_clear(void)
{
        for (size_t y = 0; y < VGA_HEIGHT; y++) {
                for (size_t x = 0; x < VGA_WIDTH; x++) {
                        const size_t index = y * VGA_WIDTH + x;
                        terminal_buffer[index] =
                            vga_entry(' ', terminal_colour);
                }
        }
}

void tty_initialize(void)
{
        terminal_row    = 0;
        terminal_column = 0;
        terminal_colour =
            vga_entry_colour(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK, 0);
        terminal_buffer = VGA_MEMORY;
        tty_clear();
        tty_cursor_enable(0, 15);
        tty_cursor_update(0, 0);
}

void tty_cursor_enable(uint8_t start, uint8_t end)
{
        outb(VGA_ADDRESS_REGISTER, VGA_CURSOR_START_REGISTER);
        outb(VGA_DATA_REGISTER, start);

        outb(VGA_ADDRESS_REGISTER, VGA_CURSOR_END_REGISTER);
        outb(VGA_DATA_REGISTER, end);
}

void tty_cursor_disable(void)
{
        outb(VGA_ADDRESS_REGISTER, VGA_CURSOR_START_REGISTER);
        outb(VGA_DATA_REGISTER, 0x20);
}

uint16_t tty_cursor_get(void)
{
        uint16_t pos = 0;
        outb(VGA_ADDRESS_REGISTER, VGA_CURSOR_LOCATION_LOW_REGISTER);
        pos = inb(VGA_DATA_REGISTER);
        outb(VGA_ADDRESS_REGISTER, VGA_CURSOR_LOCATION_HIGH_REGISTER);
        pos |= ((uint16_t)inb(VGA_DATA_REGISTER)) << 8;
        return pos;
}

void tty_cursor_update(size_t x, size_t y)
{
        const size_t index = y * VGA_WIDTH + x;

        outb(VGA_ADDRESS_REGISTER, VGA_CURSOR_LOCATION_LOW_REGISTER);
        outb(VGA_DATA_REGISTER, (uint8_t)(index & 0xFF));
        outb(VGA_ADDRESS_REGISTER, VGA_CURSOR_LOCATION_HIGH_REGISTER);
        outb(VGA_DATA_REGISTER, (uint8_t)((index >> 8) & 0xFF));
}

static void tty_putentryat(unsigned char c, uint8_t colour, size_t x, size_t y)
{
        const size_t index     = y * VGA_WIDTH + x;
        terminal_buffer[index] = vga_entry(c, colour);
}

static void tty_scroll(void)
{
        if (terminal_column == VGA_WIDTH) {
                terminal_column = 0;
                ++terminal_row;
        }
        if (terminal_row == VGA_HEIGHT) {
                --terminal_row;
                for (size_t row = 1; row < VGA_HEIGHT; ++row) {
                        for (size_t col = 0; col < VGA_WIDTH; ++col) {
                                const size_t index_dest =
                                    (row - 1) * VGA_WIDTH + col;
                                const size_t index_src = row * VGA_WIDTH + col;
                                terminal_buffer[index_dest] =
                                    terminal_buffer[index_src];
                        }
                }
                for (size_t x = 0; x < VGA_WIDTH; x++) {
                        const size_t index = (VGA_HEIGHT - 1) * VGA_WIDTH + x;
                        terminal_buffer[index] =
                            vga_entry(' ', terminal_colour);
                }
        }
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
        tty_scroll();
        tty_cursor_update(terminal_column, terminal_row);
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
