#ifndef LOS_KERNEL_TTY_H
#define LOS_KERNEL_TTY_H

#include <stddef.h>

void tty_initialize(void);
void tty_putchar(char c);
void tty_write(const char *data, size_t size);
void tty_writestring(const char *data);

#endif
