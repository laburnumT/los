#include "stdtimer.h"

#include <stddef.h>

#include "_helper.h"

void c_sleep(size_t millis);
void timer_sleep(size_t millis);

#ifdef __is_libk
        #include <kernel/timer.h>
#endif

void sleep(size_t millis)
{
        if (__is_defined(__is_libk)) {
                timer_sleep(millis);
                return;
        }
        if (__is_defined(__is_libc)) {
                c_sleep(millis);
                return;
        }
}
