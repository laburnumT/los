#ifndef LOS_KERNEL_MM_H
#define LOS_KERNEL_MM_H

#include <_helper.h>
#include <mm/mm.h>
#include <stddef.h>
#include <stdint.h>


void mm_init(void);
void mm_add_memory(uint64_t addr, uint64_t size);

void *mm_alloc_pages(size_t cnt, enum mm_gp attributes);

inline static void *mm_alloc_page(enum mm_gp attributes)
{
        return mm_alloc_pages(1, attributes);
}

#endif
