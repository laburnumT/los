#ifndef LOS_KERNEL_ARCH_I386_CONTEXT_H
#define LOS_KERNEL_ARCH_I386_CONTEXT_H

#include <stdint.h>

struct interrupt_stack_frame {
        uint32_t ds;
        uint32_t edi, esi, ebp, kern_esp, ebx, edx, ecx, eax;
        uint32_t interrupt;
        uint32_t error_code;
        uint32_t eip, cs, eflags, esp, ss;
} __attribute__((__packed__));

#endif
