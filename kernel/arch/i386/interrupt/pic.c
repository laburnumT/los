#include "pic.h"

#include <stdint.h>

#include "port.h"

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
        outb(PIC1_DATA, 0x1 << follow_id); //
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
