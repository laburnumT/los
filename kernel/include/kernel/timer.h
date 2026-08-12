#ifndef LOS_KERNEL_TIMER_H
#define LOS_KERNEL_TIMER_H

#include <stddef.h>
#include <stdint.h>

enum {
        LOS_TIMER_FREQUENCY = 1000,
};

void timer_init(size_t frequency);
void timer_sleep(size_t millis);

#endif
