#ifndef LOS_KERNEL_PS2_H
#define LOS_KERNEL_PS2_H

#include <stdint.h>

void ps2_send_byte(uint8_t byte, uint8_t port);
uint8_t ps2_receive_byte(void);

enum keyboard_code ps2_receive_key(void);
uint8_t ps2_scan_code_get(void);
void ps2_scan_code_set(uint8_t set);

#endif
