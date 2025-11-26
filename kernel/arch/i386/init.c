#include <kernel/init.h>

#include <stdint.h>
#include <stdio.h>

#include "gdt.h"
#include "idt.h"
#include "kernel/interrupt.h"
#include "mm.h"
#include "ps2.h"

void init_arch(void)
{
        puts("Setting up i386 arch");

        mm_init();

        interrupt_disable();
        gdt_init();
        puts("Set up GDT");
        idt_init();
        puts("Set up IDT");
        if (ps2_init() == 0)
                puts("Set up PS/2");

        puts("Finished setting up arch");
}
