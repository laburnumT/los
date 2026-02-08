#ifndef LOS_KERNEL_ARCH_I386_INTERRUPT_H
#define LOS_KERNEL_ARCH_I386_INTERRUPT_H

#include <stdint.h>

struct interrupt_stack_frame {
        uint32_t ds;
        uint32_t edi, esi, ebp, kern_esp, ebx, edx, ecx, eax;
        uint32_t interrupt;
        uint32_t error_code;
        uint32_t eip, cs, eflags, esp, ss;
} __attribute__((__packed__));

void interrupt_exception_handler(struct interrupt_stack_frame *frame);
void isr_init(void);
void interrupt_set_handler(
    uint8_t n, void (*handler)(struct interrupt_stack_frame *frame));

#endif
