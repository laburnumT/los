#include <stdint.h>
#include <stdio.h>

#include <drivers/pit.h>
#include <drivers/port.h>

enum { PIT_FREQUENCY = 1193182 };

enum pit_port : uint8_t {
        PIT_CHANNEL_0_PORT = 0x40,
        PIT_CHANNEL_1_PORT = 0x41,
        PIT_CHANNEL_2_PORT = 0x42,
        PIT_COMMAND_PORT   = 0x43,
};

enum : uint8_t {
        PIT_BCD_MODE = 1 << 0,

        PIT_OPERATING_MODE_0  = 0 << 1,
        PIT_OPERATING_MODE_1  = (1 << 0) << 1,
        PIT_OPERATING_MODE_2A = (1 << 1) << 1,
        PIT_OPERATING_MODE_2B = ((1 << 1) | (1 << 2)) << 1,
        PIT_OPERATING_MODE_3A = ((1 << 0) | (1 << 1)) << 1,
        PIT_OPERATING_MODE_3B = ((1 << 0) | (1 << 1) | (1 << 2)) << 1,
        PIT_OPERATING_MODE_4  = (1 << 2) << 1,
        PIT_OPERATING_MODE_5  = ((1 << 0) | (1 << 2)) << 1,

        PIT_ACCESS_MODE_LATCH    = 0 << 4,
        PIT_ACCESS_MODE_LOBYTE   = (1 << 0) << 4,
        PIT_ACCESS_MODE_HIBYTE   = (1 << 1) << 4,
        PIT_ACCESS_MODE_LOHIBYTE = ((1 << 0) | (1 << 1)) << 4,

        PIT_CHANNEL_0         = 0 << 6,
        PIT_CHANNEL_1         = (1 << 0) << 6,
        PIT_CHANNEL_2         = (1 << 1) << 6,
        PIT_CHANNEL_READ_BACK = ((1 << 0) | (1 << 1)) << 6,
};

enum : uint8_t {
        PIT_COMMAND_BCD_MODE       = 1 << 0,
        PIT_COMMAND_OPERATING_MODE = (1 << 1) | (1 << 2) | (1 << 3),
        PIT_COMMAND_ACCESS_MODE    = (1 << 4) | (1 << 5),
        PIT_COMMAND_SELECT_CHANNEL = (1 << 6) | (1 << 7),
};

void pit_init(uint32_t frequency)
{
        uint32_t pre_divisor = PIT_FREQUENCY / frequency;
        uint16_t divisor;

        if (pre_divisor <= UINT16_MAX && pre_divisor) {
                divisor = pre_divisor;
        } else if (!pre_divisor) {
                printf(
                    "Warning: frequency of %u HZ is faster than is possible. Using %u HZ\n",
                    PIT_FREQUENCY);
                divisor = 1;
        } else {
                printf(
                    "Warning: frequency of %u HZ is slower than is possible. Using ~18 HZ\n",
                    frequency);
                divisor = 0;
        }

        outb(PIT_COMMAND_PORT,
             PIT_CHANNEL_0 | PIT_ACCESS_MODE_LOHIBYTE | PIT_OPERATING_MODE_2A);
        outb(PIT_CHANNEL_0_PORT, divisor & 0xFF);
        outb(PIT_CHANNEL_0_PORT, (divisor >> 8) & 0xFF);
}
