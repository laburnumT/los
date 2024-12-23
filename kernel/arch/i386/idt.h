#ifndef LOS_KERNEL_ARCH_I386_IDT_H
#define LOS_KERNEL_ARCH_I386_IDT_H

#include <stdint.h>

enum idt_gate_type : uint8_t {
        IDT_TASK         = 0x5,
        IDT_INTERRUPT_16 = 0x6,
        IDT_TRAP_16      = 0x7,
        IDT_INTERRUPT_32 = 0xE,
        IDT_TRAP_32      = 0xF,
};

enum : uintptr_t { PIC_OFFSET = 0x20 };

void idt_init(void);
void idt_interrupt_enable(void);
void idt_interrupt_disable(void);

#endif
