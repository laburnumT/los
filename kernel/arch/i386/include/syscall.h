#ifndef LOS_KERNEL_ARCH_I386_SYSCALL_H
#define LOS_KERNEL_ARCH_I386_SYSCALL_H

#include <stdint.h>

enum : uint32_t {
        LOS_SYS_WRITE_CHAR  = 0,
        LOS_SYS_READ_CHAR   = 1,
        LOS_SYS_ALLOC_PAGES = 2,
        LOS_SYS_SLEEP       = 3,
        LOS_SYS_SCHED_YIELD = 4,
        LOS_SYS_EXIT        = 5,
};

#endif
