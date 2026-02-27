#ifndef LOS_LIB_MM_H
#define LOS_LIB_MM_H

#include <_helper.h>
#include <stdint.h>

enum FLAG_ENUM mm_gp : uint8_t {
        MM_GP_READ   = 0x1 << 0,
        MM_GP_WRITE  = 0x1 << 1,
        MM_GP_RW     = MM_GP_READ | MM_GP_WRITE,
        MM_GP_KERNEL = 0x1 << 2,
        MM_GP_USER   = 0x1 << 3,
};

#endif
