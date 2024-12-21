#include <kernel/init.h>

#include <stdint.h>
#include <stdio.h>

#include "gdt.h"

static struct global_descriptor_table {
        struct gdt_segment_descriptor null;
        struct gdt_segment_descriptor kernel_mode_code;
        struct gdt_segment_descriptor kernel_mode_data;
        struct gdt_segment_descriptor user_mode_code;
        struct gdt_segment_descriptor user_mode_data;
} __attribute__((__packed__)) gdt;

void init_arch(void)
{
        puts("Setting up i386 arch");

        puts("Setting up GDT");
        gdt_encode(&gdt.null, 0, 0, 0, 0);
        gdt_encode(&gdt.kernel_mode_code, 0xFFFFF, 0, GDT_CODE_PL0, 0xC);
        gdt_encode(&gdt.kernel_mode_data, 0xFFFFF, 0, GDT_DATA_PL0, 0xC);
        gdt_encode(&gdt.user_mode_code, 0xFFFFF, 0, GDT_CODE_PL3, 0xC);
        gdt_encode(&gdt.user_mode_data, 0xFFFFF, 0, GDT_DATA_PL3, 0xC);

        gdt_set(sizeof(gdt) - 1, (uintptr_t)&gdt);
        gdt_reload_segments();

        puts("Finished setting up arch");
}
