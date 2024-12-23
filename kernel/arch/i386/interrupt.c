#include "interrupt.h"

#include <kernel/interrupt.h>

void interrupt_exception_handler(struct interrupt_stack_frame *frame)
{
        __asm__("hlt");
}
