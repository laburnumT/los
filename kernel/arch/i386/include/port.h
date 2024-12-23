#ifndef LOS_KERNEL_ARCH_I386_PORT_H
#define LOS_KERNEL_ARCH_I386_PORT_H

#include <stdint.h>

void outb(uint16_t port, uint8_t data);
uint8_t inb(uint16_t port);

#endif
