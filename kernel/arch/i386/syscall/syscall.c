#include "syscall.h"

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

#include <kernel/interrupt.h>
#include <kernel/mm.h>
#include <kernel/syscall.h>

#include "gdt.h"
#include "idt.h"
#include "interrupt.h"
#include "kernel/timer.h"

static uint32_t _write_char(uint32_t c, [[maybe_unused]] uint32_t arg2,
                            [[maybe_unused]] uint32_t arg3,
                            [[maybe_unused]] uint32_t arg4,
                            [[maybe_unused]] uint32_t arg5,
                            [[maybe_unused]] uint32_t arg6)
{
        return putchar((char)c);
}

static uint32_t
_read_char([[maybe_unused]] uint32_t arg1, [[maybe_unused]] uint32_t arg2,
           [[maybe_unused]] uint32_t arg3, [[maybe_unused]] uint32_t arg4,
           [[maybe_unused]] uint32_t arg5, [[maybe_unused]] uint32_t arg6)
{
        return getchar();
}

static uint32_t _alloc(uint32_t cnt, uint32_t attributes,
                       [[maybe_unused]] uint32_t arg3,
                       [[maybe_unused]] uint32_t arg4,
                       [[maybe_unused]] uint32_t arg5,
                       [[maybe_unused]] uint32_t arg6)
{
        return (uint32_t)mm_alloc_pages(cnt, attributes);
}

static uint32_t _sleep(uint32_t millis, [[maybe_unused]] uint32_t arg2,
                       [[maybe_unused]] uint32_t arg3,
                       [[maybe_unused]] uint32_t arg4,
                       [[maybe_unused]] uint32_t arg5,
                       [[maybe_unused]] uint32_t arg6)
{
        timer_sleep(millis);
        return 0;
}

static uint32_t (*syscall_funcs[])(uint32_t arg1, uint32_t arg2, uint32_t arg3,
                                   uint32_t arg4, uint32_t arg5,
                                   uint32_t arg6) = {
    [LOS_SYS_WRITE_CHAR]  = _write_char,
    [LOS_SYS_READ_CHAR]   = _read_char,
    [LOS_SYS_ALLOC_PAGES] = _alloc,
    [LOS_SYS_SLEEP]       = _sleep,
};

static void interrupt_irq_128(struct interrupt_stack_frame *frame)
{
        interrupt_enable();
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
