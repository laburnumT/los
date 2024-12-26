#ifndef LOS_KERNEL_ARCH_I386_PIC_H
#define LOS_KERNEL_ARCH_I386_PIC_H

#include <stdint.h>

enum : uint16_t {
        PIC1_COMMAND = 0x20,
        PIC1_DATA    = 0x21,
        PIC2_COMMAND = 0xA0,
        PIC2_DATA    = 0xA1,
};

enum : uint8_t {
        PIC_EOI = 0x20,
};

enum : uint8_t {
        PIC_ICW1_INIT            = 0x1 << 4, // Required
        PIC_ICW1_LEVEL_TRIGGERED = 0x1 << 3, // 1 = level, 0 = edge
        PIC_ICW1_INTERVAL4       = 0x1 << 2, // 1 = 4, 0 = 8
        PIC_ICW1_SINGLE          = 0x1 << 1, // 1 = single, 0 = cascade
        PIC_ICW1_ICW4            = 0x1 << 0, // 1 = icw4 needed, 0 = not needed
};

enum : uint8_t {
        PIC_ICW4_SF_NESTED     = 0x1 << 4, // 1 = special_nested_mode, 0 = not
        PIC_ICW4_BUFFER_MAIN   = (0x1 << 3) + (0x1 << 2), // 3 = buffer main
        PIC_ICW4_BUFFER_FOLLOW = 0x1 << 2, // 2 = buffer follow, 0/1 = no buffer
        PIC_ICW4_AUTO_EOI      = 0x1 << 1, // 1 = auto eoi, 0 = normal eoi
        PIC_ICW4_8086          = 0x1 << 0, // 1 = 8086 mode, 0 mcs 80 mode
};

enum pic_get_mode : uint8_t {
        PIC_IRR = 0x0a,
        PIC_ISR = 0x0b,
};

void pic_disable(void);
void pic_send_eoi(uint8_t irq);
void pic_remap(uintptr_t offset1, uintptr_t offset2);
void pic_set_mask(uint8_t irq);
void pic_clear_mask(uint8_t irq);
uint16_t pic_get(enum pic_get_mode mode);

#endif
