#include <stdint.h>

#include <drivers/port.h>

void outb(uint16_t port, uint8_t data)
{
        __asm__ volatile("out dx, al\n" : : "Nd"(port), "a"(data) : "memory");
}

uint8_t inb(uint16_t port)
{
        uint8_t data;
        __asm__ volatile("in al, dx\n" : "=a"(data) : "Nd"(port) : "memory");
        return data;
}
