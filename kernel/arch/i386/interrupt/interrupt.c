#include "interrupt.h"

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

#include "idt.h"
#include "pic.h"
#include <kernel/panic.h>

static void (*isr_handlers[256])(struct interrupt_stack_frame *frame) = {};

static void interrupt_cpu_13(struct interrupt_stack_frame *frame)
{
        uint32_t error_code = frame->error_code;
        printf("13: %u\n", error_code);
        if (error_code == 0)
                panic();
        printf("External: %u\n", error_code & 0x1);
        char *str;
        switch (error_code & 0x3) {
        case 0:
                str = "GDT";
                break;
        case 1:
        case 3:
                str = "IDT";
                break;
        case 2:
                str = "LDT";
                break;
        default:
                unreachable();
        }
        printf("Index in %s\n", str);
        printf("Index: %p\n", (void *)(error_code & 0xFFF8));
        panic();
}

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

        panic();
}

void interrupt_set_handler(uint8_t n,
                           void (*handler)(struct interrupt_stack_frame *frame))
{
        isr_handlers[n] = handler;
        if (n >= PIC_OFFSET)
                pic_clear_mask(n - PIC_OFFSET);
}

void isr_init(void)
{
        interrupt_set_handler(13, interrupt_cpu_13);
        interrupt_set_handler(PIC_OFFSET + 0, interrupt_irq_0);
}
