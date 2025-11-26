#include "gdt.h"

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

#include "boot.h"
#include "kernel/panic.h"
#include "tss.h"

enum : uint8_t {
        GDT_PRESENT                 = 0x1 << 7,
        GDT_DESCRIPTOR_PRIV_LEVEL_3 = (0x1 << 6) + (0x1 << 5),
        GDT_DESCRIPTOR_PRIV_LEVEL_2 = 0x1 << 6,
        GDT_DESCRIPTOR_PRIV_LEVEL_1 = 0x1 << 5,
        GDT_DESCRIPTOR_PRIV_LEVEL_0 = 0x0,
        GDT_CODE_DATA               = 0x1 << 4,
        GDT_EXECUTABLE              = 0x1 << 3,
        GDT_DIRECTION               = 0x1 << 2,
        GDT_READ_WRITE              = 0x1 << 1,
        GDT_ACCESSED                = 0x1 << 0,
};

enum : uint8_t {
        GDT_FLAG_GRANULARITY = 0x1 << 3,
        GDT_FLAG_SIZE        = 0x1 << 2,
        GDT_FLAG_LONG_MODE   = 0x1 << 1,
};

enum : uint8_t {
        GDT_CODE_PL0 = GDT_PRESENT | GDT_DESCRIPTOR_PRIV_LEVEL_0 |
                       GDT_CODE_DATA | GDT_EXECUTABLE | GDT_READ_WRITE,
        GDT_DATA_PL0 = GDT_PRESENT | GDT_DESCRIPTOR_PRIV_LEVEL_0 |
                       GDT_CODE_DATA | GDT_READ_WRITE,
        GDT_CODE_PL3 = GDT_PRESENT | GDT_DESCRIPTOR_PRIV_LEVEL_3 |
                       GDT_CODE_DATA | GDT_EXECUTABLE | GDT_READ_WRITE,
        GDT_DATA_PL3 = GDT_PRESENT | GDT_DESCRIPTOR_PRIV_LEVEL_3 |
                       GDT_CODE_DATA | GDT_READ_WRITE,
        GDT_TSS = GDT_PRESENT | GDT_EXECUTABLE | GDT_ACCESSED,
};

static struct gdt gdt;

static struct {
        uint16_t size;
        uintptr_t offset;
} __attribute__((__packed__)) gdtr;

static void gdt_encode(struct gdt_segment_descriptor *d, uint32_t limit,
                       uint32_t base, uint8_t access_byte, uint8_t flags)
{
        if (limit > 0xFFFFF) {
                printf("Limit of 0x%X is larger than 0xFFFFF\n", limit);
                panic();
        }

        d->limit       = limit & 0xFFFF;
        d->limit_flags = (limit >> 16) & 0x0F;

        d->base1 = base & 0xFFFF;
        d->base2 = (base >> 16) & 0xFF;
        d->base3 = (base >> 24) & 0xFF;

        d->access_byte = access_byte;

        d->limit_flags |= flags << 4;
}

static void gdt_set(uint16_t size, uintptr_t offset)
{
        gdtr.size   = size;
        gdtr.offset = offset;
        __asm__ volatile("lgdt [%0]\n" ::"m"(gdtr) : "memory");
}

void gdt_init(void)
{
        gdt_encode(&gdt.null, 0, 0, 0, 0);
        gdt_encode(&gdt.kernel_mode_code,
                   0xFFFFF,
                   0,
                   GDT_CODE_PL0,
                   GDT_FLAG_GRANULARITY | GDT_FLAG_SIZE);
        gdt_encode(&gdt.kernel_mode_data,
                   0xFFFFF,
                   0,
                   GDT_DATA_PL0,
                   GDT_FLAG_GRANULARITY | GDT_FLAG_SIZE);
        gdt_encode(&gdt.user_mode_code,
                   0xFFFFF,
                   0,
                   GDT_CODE_PL3,
                   GDT_FLAG_GRANULARITY | GDT_FLAG_SIZE);
        gdt_encode(&gdt.user_mode_data,
                   0xFFFFF,
                   0,
                   GDT_DATA_PL3,
                   GDT_FLAG_GRANULARITY | GDT_FLAG_SIZE);
        gdt_encode(&gdt.tss, sizeof(tss) - 1, (uintptr_t)&tss, GDT_TSS, 0);

        gdt_set(sizeof(gdt) - 1, (uintptr_t)&gdt);
        gdt_reload_segments();

        tss      = (typeof(tss)){};
        tss.ss0  = offsetof(typeof(gdt), kernel_mode_data);
        tss.esp0 = (uintptr_t)&stack_top;
        __asm__ volatile("ltr %0\n" ::"r"(offsetof(typeof(gdt), tss))
                         : "memory");
}
