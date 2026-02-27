#ifndef LOS_KERNEL_ARCH_I386_GDT_H
#define LOS_KERNEL_ARCH_I386_GDT_H

#include <stdint.h>

struct gdt_segment_descriptor {
        uint16_t limit;
        uint16_t base1;
        uint8_t base2;
        uint8_t access_byte;
        uint8_t limit_flags;
        uint8_t base3;
} __attribute__((__packed__));

struct gdt {
        struct gdt_segment_descriptor null;
        struct gdt_segment_descriptor kernel_mode_code;
        struct gdt_segment_descriptor kernel_mode_data;
        struct gdt_segment_descriptor user_mode_code;
        struct gdt_segment_descriptor user_mode_data;
        struct gdt_segment_descriptor tss;
} __attribute__((__packed__));

void gdt_init(void);

// Set in gdt_asm.s
void gdt_reload_segments(void);

// Set in jump_ring.s
void jump_ring(void *location, void *user_stack);

#endif
