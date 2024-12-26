#include <kernel/init.h>

#include <stdint.h>
#include <stdio.h>

#include "gdt.h"
#include "idt.h"
#include "ps2.h"
#include <kernel/interrupt.h>

void init_arch(void)
{
        puts("Setting up i386 arch");

        interrupt_disable();
        gdt_init();
        puts("Set up GDT");
        idt_init();
        puts("Set up IDT");
        if (ps2_init() == 0)
                puts("Set up PS/2");

        puts("Finished setting up arch");
}
