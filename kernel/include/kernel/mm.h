#ifndef LOS_KERNEL_MM_H
#define LOS_KERNEL_MM_H

#include <stddef.h>
#include <stdint.h>

#include "_helper.h"

enum FLAG_ENUM mm_prot : uint8_t {
        MM_PROT_EMPTY = 0,
        MM_PROT_WRITE = 0x1 << 0,
};

void mm_init(void);
void mm_add_memory(uint64_t addr, uint64_t size);

void *mm_alloc_pages(enum mm_prot attributes, size_t cnt);

inline static void *mm_alloc_page(enum mm_prot attributes)
{
        return mm_alloc_pages(attributes, 1);
}

#endif
