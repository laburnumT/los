#include <stdio.h>

#include <kernel/init.h>
#include <kernel/tty.h>

void kernel_main(void *boot_info, unsigned int magic)
{
        tty_initialise();
        init_arch(boot_info, magic);
        puts("Hello, kernel World!\nNew\tline");
        printf("> ");
}
