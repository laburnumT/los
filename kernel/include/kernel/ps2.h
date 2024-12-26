#ifndef LOS_KERNEL_PS2_H
#define LOS_KERNEL_PS2_H

#include <stdint.h>

void ps2_send_byte(uint8_t byte, uint8_t port);
uint8_t ps2_receive_byte(void);

uint8_t ps2_receive_key(void);

#endif
