#include "interrupt.h"

#include <stdio.h>

#include <kernel/interrupt.h>

static void (*isr_handlers[256])(struct interrupt_stack_frame *frame) = {};

void interrupt_exception_handler(struct interrupt_stack_frame *frame)
{
        if (isr_handlers[frame->interrupt]) {
                isr_handlers[frame->interrupt](frame);
                return;
        }

        if (frame->interrupt < 32)
                printf("Unhandled exception: %u\n", frame->interrupt);
        else
                printf("Unhandled interrupt: %u\n", frame->interrupt - 32);

        __asm__("hlt");
}

void isr_init(void)
{
}
