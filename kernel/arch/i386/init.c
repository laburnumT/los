#include "init.h"

#include <_dynarray.h>
#include <_helper.h>
#include <mm/mm.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <kernel/init.h>
#include <kernel/interrupt.h>
#include <kernel/mm.h>
#include <kernel/panic.h>
#include <kernel/syscall.h>
#include <kernel/timer.h>

#include <drivers/ps2.h>

#include "gdt.h"
#include "idt.h"
#include "mm.h"

struct boot_info {
        size_t mod_cnt;
        uintptr_t mod_addr;
};

struct mb_module {
        uintptr_t mod_start;
        uintptr_t mod_end;
        uintptr_t string;
        unsigned char reserved[4];
} __attribute__((packed));

static void get_boot_info_1(uintptr_t mb_info_loc, struct boot_info *bi)
{
        struct multiboot_info_1 *mb_info =
            mm_alloc_addr((void *)mb_info_loc, MM_GP_READ);
        if (mb_info->flags & INIT_MB_MEM) {
                printf(
                    "Mem: %zu-%zu\n", mb_info->mem_lower, mb_info->mem_upper);
        }
        if (mb_info->flags & INIT_MB_BOOT_DISK) {
        }
        if (mb_info->flags & INIT_MB_CMDLINE) {
                char *tmp = mm_alloc_addr(mb_info->cmdline, MM_GP_READ);
                if (*tmp > ' ')
                        printf("Cmdline: %s\n", tmp);
                mm_free_page_active(tmp, 1);
        }
        if (mb_info->flags & INIT_MB_MODS) {
                printf("Found %zu modules\n", mb_info->mods_count);
                bi->mod_cnt  = mb_info->mods_count;
                bi->mod_addr = mb_info->mods_count ? mb_info->mods_addr
                                                   : (uintptr_t)(void *)nullptr;
        }
        if (mb_info->flags & INIT_MB_MMAP) {
                for (uintptr_t offset = 0; offset < mb_info->mmap_length;) {
                        struct multiboot_mmap_entry *mme = mm_alloc_addr(
                            (void *)(mb_info->mmap_addr + offset), MM_GP_READ);
                        if (mme->type == INIT_MB_MEM_AVAILABLE)
                                mm_add_memory(mme->addr, mme->len);

                        offset += mme->size + sizeof(mme->size);
                        mm_free_page_active(mme, 1);
                }
        }
        mm_free_page_active(mb_info, 1);
}

static void get_boot_info(uintptr_t mb_info_loc, uint32_t magic,
                          struct boot_info *bi)
{
        switch (magic) {
        case INIT_MB_1:
                get_boot_info_1(mb_info_loc, bi);
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

static struct module *parse_modules(size_t mod_cnt, uintptr_t mod_addr)
{
        if (!mod_cnt)
                return nullptr;

        struct module *ret = nullptr;

        struct mb_module *modules =
            mm_alloc_addr((void *)mod_addr, MM_GP_READ | MM_GP_KERNEL);

        for (size_t i = 0; i < mod_cnt; ++i) {
                auto mod               = modules[i];
                struct module mod_info = {.type = MOD_BYTES, .addr = nullptr};
                if (mod.string) {
                        char *str = mm_alloc_addr((void *)mod.string,
                                                  MM_GP_READ | MM_GP_KERNEL);
                        defer
                        {
                                mm_free_page_active(str, 1);
                        };
                        if (!*str)
                                continue;
                        size_t dot_pos = 0;
                        for (size_t j = 0; str[j]; ++j) {
                                if (str[j] == '.') {
                                        dot_pos = j;
                                        break;
                                }
                        }
                        char *str_new = strdup(str);
                        defer
                        {
                                free(str_new);
                        };
                        if (dot_pos) {
                                str_new[dot_pos] = '\0';
                                char *type       = str_new + dot_pos + 1;
                                if (!strcmp(type, "romfs")) {
                                        mod_info.type = MOD_ROMFS;
                                }
                        }
                        mod_info.name = strdup(str_new);
                        mod_info.addr = malloc((mod.mod_end - mod.mod_start) *
                                               sizeof(*mod_info.addr));
                        mod_info.len  = mod.mod_end - mod.mod_start;
                        for (uintptr_t pos = 0;
                             mod.mod_start + pos <= mod.mod_end;
                             pos += MM_PAGE_SIZE) {
                                unsigned char *tmp =
                                    mm_alloc_addr((void *)(mod.mod_start + pos),
                                                  MM_GP_RW | MM_GP_KERNEL);
                                memcpy(
                                    mod_info.addr + pos,
                                    tmp,
                                    min(MM_PAGE_SIZE - ((mod.mod_start + pos) %
                                                        MM_PAGE_SIZE),
                                        mod.mod_end - (mod.mod_start + pos)));
                                mm_free_page_active(tmp, 1);
                        }
                }
                los_arr_append(ret, mod_info);
        }

        mm_free_page_active(modules, 1);

        return ret;
}

struct module *init_arch(void *mb_info_loc, uint32_t magic)
{
        puts("Setting up i386 arch");

        mm_init();

        struct boot_info bi = {};
        get_boot_info((uintptr_t)mb_info_loc, magic, &bi);

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

        auto mods = parse_modules(bi.mod_cnt, bi.mod_addr);

        puts("Finished setting up arch");

        return mods;
}
