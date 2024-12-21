#include <kernel/init.h>

#include <stdint.h>
#include <stdio.h>

#include "gdt.h"

void init_arch(void)
{
        puts("Setting up i386 arch");

        puts("Setting up GDT");
        gdt_init();

        puts("Finished setting up arch");
}
