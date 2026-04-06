#include <kernel/keyboard.h>
#include <kernel/tty.h>

int k_putchar(int c)
{
        tty_putchar((char)c);
        return c;
}

int k_getchar(void)
{
        return keyboard_getchar();
}
