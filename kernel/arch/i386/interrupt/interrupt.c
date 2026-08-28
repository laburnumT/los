#include "interrupt.h"

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

#include <kernel/panic.h>

#include "idt.h"
#include "mm.h"
#include "pic.h"

static void (*isr_handlers[256])(struct interrupt_stack_frame *frame) = {};

static void interrupt_cpu_13(struct interrupt_stack_frame *frame)
{
        uint32_t error_code = frame->error_code;
        printf("13: %u\n", error_code);
        if (error_code == 0)
                panic();
        printf("External: %u\n", error_code & 0x1);
        char *str;
        switch (error_code & 0x6) {
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

static void interrupt_cpu_14(struct interrupt_stack_frame *frame)
{
        uint32_t error_code = frame->error_code;
        uint32_t error_address;
        __asm__("mov %0, cr2\n" : "=r"(error_address)::);

        printf("Page fault error: %u\n", error_code);
        printf("Page present  : %u\n", error_code & 0x1);
        printf("Access type   : %c\n", error_code & (0x1 << 1) ? 'W' : 'R');
        printf("User level 3  : %u\n", !!(error_code & (0x1 << 2)));
        printf("Reserved write: %u\n", !!(error_code & (0x1 << 3)));
        printf("Instr fetch   : %u\n", !!(error_code & (0x1 << 4)));
        printf("Prot key viol : %u\n", !!(error_code & (0x1 << 5)));
        printf("Shadow stack  : %u\n", !!(error_code & (0x1 << 6)));
        printf("SGX violation : %u\n", !!(error_code & (0x1 << 15)));
        printf("Error address : 0x%p\n", error_address);

        if (error_code & 0x1)
                mm_get_page_info((void *)error_address);

        panic();
}

void interrupt_exception_handler(struct interrupt_stack_frame *frame)
{
        if (isr_handlers[frame->interrupt]) {
                isr_handlers[frame->interrupt](frame);
                return;
        }
        printf("isr %u not present\n", frame->interrupt);

        if (frame->interrupt < PIC_OFFSET)
                printf("Unhandled exception: %u\n", frame->interrupt);
        else
                printf("Unhandled interrupt: %u\n",
                       frame->interrupt - PIC_OFFSET);

        panic();
}

void interrupt_set_handler(uint8_t n,
                           void (*handler)(struct interrupt_stack_frame *frame))
{
        isr_handlers[n] = handler;
        if (n >= PIC_OFFSET) {
                if (n - PIC_OFFSET >= 8)
                        pic_clear_mask(2);
                pic_clear_mask(n - PIC_OFFSET);
        }
}

void isr_init(void)
{
        interrupt_set_handler(13, interrupt_cpu_13);
        interrupt_set_handler(14, interrupt_cpu_14);
}
