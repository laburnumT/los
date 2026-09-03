#ifndef LOS_KERNEL_ARCH_I386_INTERRUPT_H
#define LOS_KERNEL_ARCH_I386_INTERRUPT_H

#include <stdint.h>

#include "context.h"

void interrupt_exception_handler(struct interrupt_stack_frame *frame);
void isr_init(void);
void interrupt_set_handler(
    uint8_t n, void (*handler)(struct interrupt_stack_frame *frame));

#endif
