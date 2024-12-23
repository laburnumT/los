#include <kernel/init.h>

#include <stdint.h>
#include <stdio.h>

#include "gdt.h"
#include "idt.h"
#include "kernel/interrupt.h"

void init_arch(void)
{
        puts("Setting up i386 arch");

        puts("Setting up GDT");
        interrupt_disable();
        gdt_init();
        puts("Setting up IDT");
        idt_init();

        puts("Finished setting up arch");
}
