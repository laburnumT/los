#include <kernel/pcspeaker.h>


void play_sound(uint32_t freq) {
    uint32_t divisor = 1193180 / freq;

    // Set speaker to square wave mode (only available mode)
    outb(0x43, 0xB6);

    // send divisor (low byte, high byte)
    outb(0x42, (uint8_t)(divisor & 0xFF));
    outb(0x42, (uint8_t)((divisor >> 8) & 0xFF));

    // enable speaker
    outb(0x61, 3);
}

void stop_sound() {
    //only the last two bits are used
    // bit 0: speaker enable
    // bit 1: tone activation?????
    outb(0x61, 0);
}