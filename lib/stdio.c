#include <stdio.h>

#ifdef __is_libk
        #include <kernel/tty.h>
#endif

int putchar(int c)
{
#ifdef __is_libk
        tty_write(&(char){(char)c}, 1);
#else
        #error NOT IMPLEMENTED
#endif
        return c;
}

int puts(const char *s)
{
        for (; *s; ++s) {
                if (putchar(*s) != *s)
                        return EOF;
        }

        return 0;
}
