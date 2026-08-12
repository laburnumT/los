#include "init.h"

#include <libelf.h>
#include <mm/mm.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

#include <kernel/interrupt.h>
#include <kernel/mm.h>
#include <kernel/panic.h>
#include <kernel/syscall.h>
#include <kernel/timer.h>

#include "gdt.h"
#include "idt.h"
#include "ps2.h"
#include "stdlib.h"

static void get_boot_info_1(uintptr_t mb_info_loc)
{
        struct multiboot_info_1 *mb_info =
            mm_alloc_addr((void *)mb_info_loc, MM_GP_READ);
        if (mb_info->flags & INIT_MB_MEM) {
                printf(
                    "Mem: %zu-%zu\n", mb_info->mem_lower, mb_info->mem_upper);
        }
        if (mb_info->flags & INIT_MB_BOOT_DISK) {
                printf("Boot disk stuff I guess\n");
        }
        if (mb_info->flags & INIT_MB_CMDLINE) {
                char *tmp = mm_alloc_addr(mb_info->cmdline, MM_GP_READ);
                if (*tmp > ' ')
                        printf("Cmdline: %s\n", tmp);
                mm_free_page(tmp, 1);
        }
        if (mb_info->flags & INIT_MB_MMAP) {
                for (uintptr_t offset = 0; offset < mb_info->mmap_length;) {
                        struct multiboot_mmap_entry *mme = mm_alloc_addr(
                            (void *)(mb_info->mmap_addr + offset), MM_GP_READ);
                        if (mme->type == INIT_MB_MEM_AVAILABLE) {
                                mm_add_memory(mme->addr, mme->len);
                        }
                        offset += mme->size + sizeof(mme->size);
                        mm_free_page(mme, 1);
                }
        }
        mm_free_page(mb_info, 1);
}

static void get_boot_info(uintptr_t mb_info_loc, uint32_t magic)
{
        switch (magic) {
        case INIT_MB_1:
                get_boot_info_1(mb_info_loc);
                break;
        case INIT_MB_2:
                printf("Multiboot 2 is not supported\n");
                panic();
                break;
        default:
                printf("0x%X not a valid grub magic number\n", magic);
                panic();
        }
}

void jump_to_userspace(unsigned char *exec, size_t stack_size)
{
        struct libelf_state state;
        libelf_load_elf(exec, &state);

        mm_switch_address_space(state.address_space);

        unsigned char *user_stack = k_malloc(stack_size, MM_GP_USER | MM_GP_RW);

        jump_ring((void *)state.entry,
                  (void *)((uintptr_t)(user_stack + stack_size) & ~0xF));
}

void init_arch(uintptr_t mb_info_loc, uint32_t magic)
{
        puts("Setting up i386 arch");

        mm_init();

        get_boot_info(mb_info_loc, magic);

        interrupt_disable();
        gdt_init();
        puts("Set up GDT");
        idt_init();
        puts("Set up IDT");
        syscall_init();
        puts("Set up syscall");
        interrupt_enable();

        if (ps2_init() == 0)
                puts("Set up PS/2");

        timer_init(LOS_TIMER_FREQUENCY);
        puts("Set up timer");

        puts("Finished setting up arch");
}
