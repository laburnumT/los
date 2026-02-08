#ifndef LOS_KERNEL_ARCH_I386_VM_H
#define LOS_KERNEL_ARCH_I386_VM_H

#include <stddef.h>
#include <stdint.h>

#include <kernel/mm.h>

#include "_helper.h"

enum FLAG_ENUM ptde_attributes : uint8_t {
        PTDE_PRESENT       = 0x1 << 0,
        PTDE_WRITE         = 0x1 << 1,
        PTDE_USER_MODE     = 0x1 << 2,
        PTDE_WRITE_THROUGH = 0x1 << 3,
        PTDE_CACHE_DISABLE = 0x1 << 4,
        PTDE_ACCESSED      = 0x1 << 5,
        PTE_DIRTY          = 0x1 << 6,
        PTE_PAT            = 0x1 << 7,
        PDE_PAGE_SIZE      = 0x1 << 7,
};

enum mm_update_attribute_type : uint8_t {
        MM_REPLACE,
        MM_ADD,
        MM_TOGGLE,
        MM_REMOVE,
};

uintptr_t mm_get_phys_addr(uintptr_t addr);
void mm_update_attributes(void *addr, enum mm_update_attribute_type update,
                          enum ptde_attributes attributes);

void *mm_fpa_alloc_addr(void *phys_addr, enum mm_prot attributes);
void mm_pfa_dealloc(void *addr, uint8_t phys_alloc);

void mm_get_page_info(void *addr);

void *map_memory(uintptr_t info_phys_addr, size_t size);
void unmap_memory(void *virt_addr);

#endif
