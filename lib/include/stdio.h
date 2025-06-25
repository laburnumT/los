#ifndef LOS_LIB_STDIO_H
#define LOS_LIB_STDIO_H

#include <stdarg.h>

#define EOF (-1)

int putchar(int c);
int puts(const char *s);
int vprintf(const char *restrict format, va_list arg);
int printf(const char *restrict format, ...);
int getchar(void);

#endif
