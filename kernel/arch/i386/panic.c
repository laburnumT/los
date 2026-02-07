#include <stddef.h>

#include <kernel/interrupt.h>

[[noreturn]] void panic(void)
{
        interrupt_disable();
        __asm__("hlt");
        unreachable();
}
