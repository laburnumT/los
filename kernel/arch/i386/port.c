#include "port.h"

#include <stdint.h>

void outb(uint16_t port, uint8_t data)
{
        __asm__("out dx, al\n" : : "d"(port), "a"(data));
}

uint8_t inb(uint16_t port)
{
        uint8_t data;
        __asm__("in al, dx\n" : "=a"(data) : "d"(port));
        return data;
}
