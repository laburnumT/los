#ifndef LOS_KERNEL_MM_H
#define LOS_KERNEL_MM_H

#include <mm/mm.h>
#include <stddef.h>
#include <stdint.h>


void mm_init(void);
void mm_add_memory(uint64_t addr, uint64_t size);

uint64_t mm_get_free_page(void);
void *mm_alloc_addr(void *phys_addr, enum mm_gp attributes);
void mm_free_page_active(void *addr, uint8_t phys_alloc);
void *mm_alloc_pages(size_t cnt, enum mm_gp attributes);

inline static void *mm_alloc_page(enum mm_gp attributes)
{
        return mm_alloc_pages(1, attributes);
}

struct mm_struct *mm_create_new_mm(void);
void mm_switch_address_space(struct mm_struct *space);
struct mm_struct *mm_get_active_address_space(void);

int mm_map_page(struct mm_struct *space, uintptr_t virt_addr,
                uintptr_t phys_addr, enum mm_gp attributes);
int mm_map_range(struct mm_struct *space, uintptr_t vaddr, uintptr_t paddr,
                 size_t cnt, enum mm_gp attributes);

#endif
