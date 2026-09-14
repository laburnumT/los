#ifndef LOS_KERNEL_ARCH_I386_PIC_H
#define LOS_KERNEL_ARCH_I386_PIC_H

#include <stdint.h>

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
