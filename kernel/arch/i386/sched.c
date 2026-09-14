#include "sched.h"

#include <mm/mm.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include <kernel/mm.h>
#include <kernel/sched.h>

#include "context.h"
#include "cpu.h"
#include "gdt.h"
#include "gdt/tss.h"
#include "libelf.h"
#include "mm.h"

void sched_arch_switch_context_helper(void *);

enum {
        KERNEL_STACK_SIZE = 0x1000,
};

void sched_arch_save_frame(struct interrupt_stack_frame *frame)
{
        auto current = sched_get_current();
        if (current)
                current->context = frame;
}

void sched_arch_switch_context(struct los_process *pcurrent)
{
        mm_switch_address_space(pcurrent->state->address_space);
        tss.esp0 = (uintptr_t)pcurrent->kernel_stack_top;
        sched_arch_switch_context_helper(pcurrent->context);
}

static void *sched_alloc_user_stack(struct mm_struct *space, size_t stack_size)
{
        size_t page_count = (stack_size + MM_PAGE_SIZE - 1) / MM_PAGE_SIZE;
        uintptr_t user_virt_addr =
            VIRT_ADDRESS_BASE - (page_count * MM_PAGE_SIZE);

        for (size_t i = 0; i < page_count; i++) {
                uintptr_t phys_frame = mm_get_free_page();

                mm_map_page(space,
                            user_virt_addr + i * MM_PAGE_SIZE,
                            phys_frame,
                            MM_GP_USER | MM_GP_RW);
        }

        return (void *)(user_virt_addr + page_count * MM_PAGE_SIZE);
}

static void sched_free_user_stack(struct mm_struct *space, size_t stack_size)
{
        size_t page_count = (stack_size + MM_PAGE_SIZE - 1) / MM_PAGE_SIZE;
        uintptr_t user_virt_addr =
            VIRT_ADDRESS_BASE - (page_count * MM_PAGE_SIZE);

        for (size_t i = 0; i < page_count; i++)
                mm_free_page(space,
                             (unsigned char *)user_virt_addr + i * MM_PAGE_SIZE,
                             0);
}

void sched_arch_init_process(struct los_process *process,
                             size_t user_stack_size)
{
        size_t kernel_stack_size     = KERNEL_STACK_SIZE;
        process->kernel_stack_bottom = k_malloc(kernel_stack_size, MM_GP_RW);
        process->kernel_stack_top =
            (void *)((uintptr_t)(process->kernel_stack_bottom +
                                 kernel_stack_size) &
                     ~0xF);
        struct interrupt_stack_frame *frame =
            (struct interrupt_stack_frame *)(process->kernel_stack_top -
                                             sizeof(*frame));

        auto user_stack_top = sched_alloc_user_stack(
            process->state->address_space, user_stack_size);
        process->user_stack_size = user_stack_size;

        *frame        = (typeof(*frame)){};
        frame->ds     = offsetof(struct gdt, user_mode_data) | 3;
        frame->ss     = offsetof(struct gdt, user_mode_data) | 3;
        frame->cs     = offsetof(struct gdt, user_mode_code) | 3;
        frame->eip    = process->state->entry;
        frame->esp    = (uintptr_t)user_stack_top;
        frame->eflags = get_eflags() | 0x200;

        process->context = frame;
}

void sched_arch_free_process(struct los_process *process)
{
        if (!process)
                return;
        free(process->kernel_stack_bottom);
        sched_free_user_stack(process->state->address_space,
                              process->user_stack_size);
        mm_unmap_userspace(process->state->address_space);
        free(process->state->address_space);
}
