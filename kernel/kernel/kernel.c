#include <stdio.h>

#include <kernel/tty.h>

void kernel_main(void) {
	tty_initialize();
	puts("Hello, kernel World!\nNew\tline\n");
}
