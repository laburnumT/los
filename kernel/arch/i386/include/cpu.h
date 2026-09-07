#ifndef LOS_KERNEL_ARCH_I386_CPU_H
#define LOS_KERNEL_ARCH_I386_CPU_H

#include <stdint.h>

uint32_t get_eflags(void);
uint32_t get_cr3(void);

#endif
