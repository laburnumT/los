#include <stddef.h>
#include <stdint.h>

#include <kernel/init.h>
#include <kernel/tty.h>

void kernel_main(void *boot_info, unsigned int magic)
{
        tty_initialise();
        init_arch(boot_info, magic);

        static unsigned char code[] = {
#embed "../../build/bin/lsh/lsh"
        };

        jump_to_userspace(code, 16384);
}
