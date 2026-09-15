#ifndef LOS_KERNEL_INTERRUPT_H
#define LOS_KERNEL_INTERRUPT_H

#include <stdint.h>

void interrupt_enable(void);
void interrupt_disable(void);
uint8_t interrupt_check_enabled(void);

#endif
