#ifndef LOS_KERNEL_TTY_H
#define LOS_KERNEL_TTY_H

#include <stddef.h>
#include <stdint.h>

void tty_initialize(void);
void tty_putchar(char c);
void tty_write(const char *data, size_t size);
void tty_writestring(const char *data);
void tty_cursor_enable(uint8_t start, uint8_t end);
void tty_cursor_disable(void);
void tty_cursor_update(size_t x, size_t y);

#endif
