#include "idt.h"

#include <stdint.h>

#include "interrupt.h"
#include "pic.h"
#include <kernel/interrupt.h>

static struct {
        uint16_t size;
        uintptr_t offset;
} __attribute__((__packed__)) idtr;

static struct idt_entry {
        uint16_t offset1;
        uint16_t segment_selector;
        uint8_t reserved;
        uint8_t attributes;
        uint16_t offset2;
} __attribute__((__packed__)) idt[256];

extern void (*isr_table[256])(void);

static void idt_encode(struct idt_entry *i, uintptr_t offset,
                       uint16_t segment_selector, enum idt_gate_type gate_type,
                       uint8_t priv_level, uint8_t present)
{
        i->offset1 = offset & 0xFFFF;
        i->offset2 = (offset >> 16) & 0xFFFF;

        i->segment_selector = segment_selector;

        i->reserved = 0;

        i->attributes = (present << 7) | (priv_level << 5) | gate_type;
}

static void idt_set(uint16_t size, uintptr_t offset)
{
        idtr.size   = size;
        idtr.offset = offset;
        __asm__ volatile("lidt [%0]\n" ::"m"(idtr));
}

void idt_init(void)
{
        for (uint16_t i = 0; i < 256; ++i) {
                idt_encode(idt + i,
                           (uintptr_t)isr_table[i],
                           0x08,
                           IDT_INTERRUPT_32,
                           0,
                           1);
        }

        // Disable all interrupts
        for (uint16_t i = 0; i < (uint16_t)(256 - PIC_OFFSET); ++i)
                pic_set_mask(i);

        idt_set(sizeof(idt) - 1, (uintptr_t)idt);

        pic_remap(PIC_OFFSET, PIC_OFFSET + 8);

        isr_init();

        interrupt_enable();
}
