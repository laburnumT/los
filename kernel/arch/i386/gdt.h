#ifndef LOS_KERNEL_ARCH_I386_GDT_H
#define LOS_KERNEL_ARCH_I386_GDT_H

#include <stdint.h>

enum gdt_access_byte : uint8_t {
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

enum gdt_access_byte_defaults : uint8_t {
        GDT_CODE_PL0 = GDT_PRESENT | GDT_DESCRIPTOR_PRIV_LEVEL_0 |
                       GDT_CODE_DATA | GDT_EXECUTABLE | GDT_READ_WRITE,
        GDT_DATA_PL0 = GDT_PRESENT | GDT_DESCRIPTOR_PRIV_LEVEL_0 |
                       GDT_CODE_DATA | GDT_READ_WRITE,
        GDT_CODE_PL3 = GDT_PRESENT | GDT_DESCRIPTOR_PRIV_LEVEL_3 |
                       GDT_CODE_DATA | GDT_EXECUTABLE | GDT_READ_WRITE,
        GDT_DATA_PL3 = GDT_PRESENT | GDT_DESCRIPTOR_PRIV_LEVEL_3 |
                       GDT_CODE_DATA | GDT_READ_WRITE,
};

struct gdt_segment_descriptor {
        uint16_t limit;
        uint16_t base1;
        uint8_t base2;
        uint8_t access_byte;
        uint8_t limit_flags;
        uint8_t base3;
} __attribute__((__packed__));

void gdt_encode(struct gdt_segment_descriptor *d,
                                       uint32_t limit, uint32_t base,
                                       uint8_t access_byte, uint8_t flags);
// Set in gdt_asm.s
void gdt_set(uint16_t size, uintptr_t offset);
void gdt_reload_segments(void);

#endif
