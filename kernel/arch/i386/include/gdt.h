#ifndef LOS_KERNEL_ARCH_I386_GDT_H
#define LOS_KERNEL_ARCH_I386_GDT_H

#include <stdint.h>

void gdt_init(void);

// Set in gdt_asm.s
void gdt_reload_segments(void);

#endif
