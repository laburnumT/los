#include "gdt.h"

#include <stdint.h>

static struct {
        struct gdt_segment_descriptor null;
        struct gdt_segment_descriptor kernel_mode_code;
        struct gdt_segment_descriptor kernel_mode_data;
        struct gdt_segment_descriptor user_mode_code;
        struct gdt_segment_descriptor user_mode_data;
} __attribute__((__packed__)) gdt;

static struct {
        uint16_t size;
        uintptr_t offset;
} __attribute__((__packed__)) gdtr;

static void gdt_encode(struct gdt_segment_descriptor *d, uint32_t limit,
                       uint32_t base, uint8_t access_byte, uint8_t flags)
{
        // TODO(laburnumT): enforce limit < 0xFFFFF
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
        __asm__("lgdt [%0]\n" ::"m"(gdtr));
}

void gdt_init(void)
{
        gdt_encode(&gdt.null, 0, 0, 0, 0);
        gdt_encode(&gdt.kernel_mode_code, 0xFFFFF, 0, GDT_CODE_PL0, 0xC);
        gdt_encode(&gdt.kernel_mode_data, 0xFFFFF, 0, GDT_DATA_PL0, 0xC);
        gdt_encode(&gdt.user_mode_code, 0xFFFFF, 0, GDT_CODE_PL3, 0xC);
        gdt_encode(&gdt.user_mode_data, 0xFFFFF, 0, GDT_DATA_PL3, 0xC);

        gdt_set(sizeof(gdt) - 1, (uintptr_t)&gdt);
        gdt_reload_segments();
}
