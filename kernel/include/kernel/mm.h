#ifndef LOS_KERNEL_MM_H
#define LOS_KERNEL_MM_H

#include <stddef.h>
#include <stdint.h>

#include "_helper.h"

enum FLAG_ENUM mm_gp : uint8_t {
        MM_GP_READ   = 0x1 << 0,
        MM_GP_WRITE  = 0x1 << 1,
        MM_GP_RW     = MM_GP_READ | MM_GP_WRITE,
        MM_GP_KERNEL = 0x1 << 2,
        MM_GP_USER   = 0x1 << 3,
};

void mm_init(void);
void mm_add_memory(uint64_t addr, uint64_t size);

void *mm_alloc_pages(enum mm_gp attributes, size_t cnt);

inline static void *mm_alloc_page(enum mm_gp attributes)
{
        return mm_alloc_pages(attributes, 1);
}

#endif
