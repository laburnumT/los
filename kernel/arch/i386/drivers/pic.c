#include <stdint.h>

#include <drivers/pic.h>
#include <drivers/port.h>

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

void pic_disable(void)
{
        outb(PIC1_DATA, 0xFF);
        outb(PIC2_DATA, 0xFF);
}

void pic_send_eoi(uint8_t irq)
{
        if (irq >= 8)
                outb(PIC2_COMMAND, PIC_EOI);
        outb(PIC1_COMMAND, PIC_EOI);
}

void pic_remap(uintptr_t offset1, uintptr_t offset2)
{
        const uint8_t save1 = inb(PIC1_DATA);
        const uint8_t save2 = inb(PIC2_DATA);

        // ICW1: Init in cascade
        outb(PIC1_COMMAND, PIC_ICW1_INIT | PIC_ICW1_ICW4);
        outb(PIC2_COMMAND, PIC_ICW1_INIT | PIC_ICW1_ICW4);

        // ICW2: Set offsets
        outb(PIC1_DATA, offset1);
        outb(PIC2_DATA, offset2);

        // ICW3:
        const uint8_t follow_id = 2;
        outb(PIC1_DATA, 0x1 << follow_id);
        outb(PIC2_DATA, follow_id);

        // ICW4: 8086 mode
        outb(PIC1_DATA, PIC_ICW4_8086);
        outb(PIC2_DATA, PIC_ICW4_8086);

        outb(PIC1_DATA, save1);
        outb(PIC2_DATA, save2);
}

static uint16_t pic_mask_prework(uint8_t *irq)
{
        uint16_t port;

        if (*irq < 8) {
                port = PIC1_DATA;
        } else {
                port = PIC2_DATA;
                *irq -= 8;
        }
        return port;
}

void pic_set_mask(uint8_t irq)
{
        uint16_t port = pic_mask_prework(&irq);
        outb(port, inb(port) | (1 << irq));
}

void pic_clear_mask(uint8_t irq)
{
        uint16_t port = pic_mask_prework(&irq);
        outb(port, inb(port) & ~(1 << irq));
}

uint16_t pic_get(enum pic_get_mode mode)
{
        outb(PIC1_COMMAND, mode);
        outb(PIC2_COMMAND, mode);
        return (inb(PIC2_COMMAND) << 8) | inb(PIC1_COMMAND);
}
