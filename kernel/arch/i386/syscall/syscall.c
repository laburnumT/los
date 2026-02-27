#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

#include <kernel/syscall.h>

#include "gdt.h"
#include "idt.h"
#include "interrupt.h"

static uint32_t _write_char(uint32_t arg1, [[maybe_unused]] uint32_t arg2,
                            [[maybe_unused]] uint32_t arg3,
                            [[maybe_unused]] uint32_t arg4,
                            [[maybe_unused]] uint32_t arg5,
                            [[maybe_unused]] uint32_t arg6)
{
        return putchar((char)arg1);
}

static uint32_t (*syscall_funcs[])(uint32_t arg1, uint32_t arg2, uint32_t arg3,
                                   uint32_t arg4, uint32_t arg5,
                                   uint32_t arg6) = {[0] = _write_char};

static void interrupt_irq_128(struct interrupt_stack_frame *frame)
{
        if (frame->eax >= sizeof(syscall_funcs) / sizeof(*syscall_funcs))
                return;

        frame->eax = syscall_funcs[frame->eax](frame->ebx,
                                               frame->ecx,
                                               frame->edx,
                                               frame->esi,
                                               frame->edi,
                                               frame->ebp);
}

void syscall_init(void)
{
        idt_update_entry(128,
                         offsetof(struct gdt, kernel_mode_code),
                         IDT_INTERRUPT_32,
                         3,
                         1);
        interrupt_set_handler(128, interrupt_irq_128);
}
