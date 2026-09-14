#ifndef LOS_KERNEL_ARCH_I386_SCHED_H
#define LOS_KERNEL_ARCH_I386_SCHED_H

#include "context.h"

void sched_arch_save_frame(struct interrupt_stack_frame *frame);

#endif
