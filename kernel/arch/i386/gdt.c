#include "gdt.h"

#include <stdint.h>

void gdt_encode(struct gdt_segment_descriptor *d, uint32_t limit, uint32_t base,
                uint8_t access_byte, uint8_t flags)
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
