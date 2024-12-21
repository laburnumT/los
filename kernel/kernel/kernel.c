#include <stdio.h>

#include <kernel/tty.h>

void kernel_main(void)
{
        tty_initialise();
        puts("Hello, kernel World!\nNew\tline");
}
