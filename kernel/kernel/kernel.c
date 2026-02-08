#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

#include <kernel/init.h>
#include <kernel/tty.h>

void run_shell(void)
{
        // malloc/free test
        struct {
                size_t size, loc;
                char *buff;
        } b = {.size = 5, .loc = 0, .buff = malloc(5 * sizeof(*b.buff))};
        printf("> ");
        for (;;) {
                int c = getchar();
                if (c == -1)
                        continue;
                if (b.size == b.loc) {
                        b.size *= 2;
                        b.buff = realloc(b.buff, b.size);
                }
                b.buff[b.loc++] = (char)c;
                putchar(c);
                if (c == '\n') {
                        printf("> ");
                        b.loc = 0;
                }
        }
}

void kernel_main(void *boot_info, unsigned int magic)
{
        tty_initialise();
        init_arch(boot_info, magic);

        run_shell();
}
