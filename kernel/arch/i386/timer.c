#include <stddef.h>
#include <stdint.h>

#include <kernel/timer.h>

#include "context.h"
#include "idt.h"
#include "interrupt.h"
#include "pic.h"
#include "pit.h"

static volatile size_t LOS_TIMER = 0;

static void interrupt_irq_0(struct interrupt_stack_frame *frame)
{
        (void)frame;
        ++LOS_TIMER;
        pic_send_eoi(0);
}

void timer_sleep(size_t millis)
{
        size_t end = LOS_TIMER + millis;
        for (; LOS_TIMER < end;) {
                __asm__("hlt");
        }
}

void timer_init(uint32_t frequency)
{
        pit_init(frequency);
        interrupt_set_handler(PIC_OFFSET + 0, interrupt_irq_0);
}
