#ifndef LOS_LIB_STDIO_H
#define LOS_LIB_STDIO_H

#define EOF (-1)

int putchar(int c);
int puts(const char *s);
int printf(const char *restrict format, ...);
int getchar(void);

#endif
