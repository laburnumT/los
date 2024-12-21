#include <stdio.h>

#include <kernel/init.h>
#include <kernel/tty.h>

void kernel_main(void)
{
        tty_initialise();
        init_arch();
        puts("Hello, kernel World!\nNew\tline");
        printf("> ");
}
