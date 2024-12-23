#include "interrupt.h"

#include <stdio.h>

#include "idt.h"
#include "pic.h"

static void (*isr_handlers[256])(struct interrupt_stack_frame *frame) = {};

static void interrupt_irq_0(struct interrupt_stack_frame *frame)
{
        (void)frame;
        pic_send_eoi(0);
}

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
        isr_handlers[PIC_OFFSET + 0] = interrupt_irq_0;
}
