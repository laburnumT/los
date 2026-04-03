#include "mm.h"

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <kernel/mm.h>
#include <kernel/panic.h>

enum : uint32_t {
        MM_ADDR_MASK       = 0xFFFFF000,
        MM_PAGES_INIT_SIZE = 2,
};

enum : uint16_t {
        MM_PAGE_SIZE = 0x1000,
};

/* PTE
 * 0-7   ptde_attributes
 * 8     Global
 * 9-11  Ignored
 * 12-31 Physical address
 */
struct page_table_entry {
        union {
                struct {
                        enum ptde_attributes attributes;
                        uint8_t global_addr;
                        uint16_t addr;
                } __attribute__((__packed__));
                uint32_t val;
        };
} __attribute__((__packed__));

/*
 * 0-7   ptde_attributes
 * 8-11  Ignored
 * 12-31 Physical address
 */
struct page_directory_entry {
        union {
                struct {
                        enum ptde_attributes attributes;
                        uint8_t addr_12_15;
                        uint16_t addr_16_31;
                } __attribute__((__packed__));
                uint32_t val;
        };
} __attribute__((__packed__));

static struct mm_available_pages {
        size_t size;
        size_t pos;
        uintptr_t addr[];
} available_pages_init = {
    .size = MM_PAGES_INIT_SIZE,
    .pos  = 0,
    .addr = {0, 0},
};

static struct mm_available_pages *available_pages = &available_pages_init;

extern struct page_directory_entry page_directory[1024];
extern struct page_table_entry page_table1[1024];
extern char scratch_space[4096 * 2];
extern const uintptr_t VIRT_ADDRESS_BASE;

static struct page_table_entry *page_tables_virt[1024];

static void *mm_alloc_page_from_page_table1(enum ptde_attributes attributes);

static struct page_table_entry create_pte(uintptr_t addr, uint8_t global,
                                          uint8_t attributes)
{
        return (struct page_table_entry){
            .val = attributes + ((global & 0x1) << 8) + (addr & MM_ADDR_MASK)};
}

static struct page_directory_entry create_pde(size_t idx,
                                              enum ptde_attributes attributes)
{
        struct page_table_entry *pt =
            mm_alloc_page_from_page_table1(PTDE_PRESENT | PTDE_WRITE);
        memset(pt, 0, sizeof(*pt));
        page_tables_virt[idx]           = pt;
        struct page_directory_entry pde = {
            .val = mm_get_phys_addr((uintptr_t)pt) & MM_ADDR_MASK};
        pde.attributes = attributes;
        return pde;
}

static size_t get_pde_index_from_address(uintptr_t addr)
{
        return addr >> (32 - 10);
}

static size_t get_pte_index_from_address(uintptr_t addr)
{
        return addr >> (32 - 20) & 0x03FF;
}

static struct page_directory_entry *get_pde_from_address(uintptr_t addr)
{
        return page_directory + get_pde_index_from_address(addr);
}

static struct page_table_entry *get_pte_from_address(uintptr_t addr)
{
        struct page_directory_entry *pde = get_pde_from_address(addr);
        if (!(pde->attributes & PTDE_PRESENT)) {
                printf("%p not present\n", addr);
                panic();
        }
        size_t index = get_pte_index_from_address(addr);
        struct page_table_entry *pte =
            (struct page_table_entry *)((pde->val & MM_ADDR_MASK) +
                                        VIRT_ADDRESS_BASE) +
            index;
        if (!(pte->attributes & PTDE_PRESENT)) {
                printf("%p not present\n", addr);
                panic();
        }

        return pte;
}

static uintptr_t get_virt_address(size_t pd_idx, size_t pt_idx)
{
        return (pd_idx << (32 - 10)) + (pt_idx << (32 - 20));
}

static void mm_add_page(uint64_t addr)
{
        if (available_pages->pos == available_pages->size) {
                if (available_pages == &available_pages_init)
                        return;

                available_pages->size *= 2;
                struct mm_available_pages *tmp = realloc(
                    available_pages,
                    sizeof(*available_pages) +
                        available_pages->size * sizeof(*available_pages->addr));
                if (tmp == nullptr)
                        return;
                available_pages = tmp;
        }

        available_pages->addr[available_pages->pos++] = addr;
}

static uint64_t mm_get_free_page(void)
{
        if (available_pages->pos == 0) {
                printf("Out of memory\n");
                panic();
        }
        return available_pages->addr[--available_pages->pos];
}

static uint8_t is_subset_rough(enum ptde_attributes bigger,
                               enum ptde_attributes smaller)
{
        const enum ptde_attributes mask =
            PTDE_PRESENT | PTDE_WRITE | PTDE_USER_MODE;
        bigger &= mask;
        smaller &= mask;
        return (bigger & smaller) == smaller;
}

static void *mm_alloc_page_from_page_table1(enum ptde_attributes attributes)
{
        for (size_t i = 0; i < sizeof(page_table1) / sizeof(*page_table1);
             ++i) {
                if (!(page_table1[i].attributes & PTDE_PRESENT)) {
                        const uintptr_t page = mm_get_free_page();
                        page_table1[i] =
                            create_pte(page, 0, PTDE_PRESENT | attributes);
                        return (void *)(get_virt_address(768, i));
                }
        }
        printf("OUT OF MEMORY\n");
        panic();
}

static enum ptde_attributes convert_attributes(enum mm_gp attr)
{
        enum ptde_attributes ret = 0;
        if (attr & MM_GP_WRITE || attr & MM_GP_RW)
                ret |= PTDE_WRITE;
        if (attr & MM_GP_USER)
                ret |= PTDE_USER_MODE;
        return ret;
}

void mm_add_memory(uint64_t addr, uint64_t size)
{
        size_t pages = size / (uint64_t)MM_PAGE_SIZE;
        for (size_t i = 0; i < pages; ++i)
                mm_add_page(addr + (uint64_t)i * MM_PAGE_SIZE);
}

uintptr_t mm_get_phys_addr(uintptr_t addr)
{
        const struct page_table_entry *pte = get_pte_from_address(addr);
        return pte->val & MM_ADDR_MASK;
}

void mm_update_attributes(void *addr, enum mm_update_attribute_type update,
                          enum ptde_attributes attributes)
{
        struct page_table_entry *pte = get_pte_from_address((uintptr_t)addr);

        switch (update) {
        case MM_REPLACE:
                pte->attributes = attributes;
                break;
        case MM_ADD:
                pte->attributes |= attributes;
                break;
        case MM_TOGGLE:
                pte->attributes ^= attributes;
                break;
        case MM_REMOVE:
                pte->attributes &= ~attributes;
                break;
        }

        pte->attributes |= PTDE_PRESENT;
        __asm__ volatile("invlpg [%0]" ::"r"(addr) : "memory");
}

static size_t find_consecutive_entries(size_t pd_idx,
                                       struct page_table_entry pt[1024],
                                       size_t cnt)
{
        for (size_t idx = pd_idx ? 0 : 1; idx < 1024 - cnt + 1; ++idx) {
                uint8_t found = 1;
                for (size_t i = 0; i < cnt && idx + i < 1024; ++i) {
                        if (pt[idx + i].attributes & PTDE_PRESENT) {
                                found = 0;
                                break;
                        }
                }
                if (found)
                        return idx;
        }
        return 1024;
}

void *mm_alloc_pages(enum mm_gp attributes, size_t cnt)
{
        enum ptde_attributes attr =
            convert_attributes(attributes) | PTDE_PRESENT;

        for (size_t pd_idx = 0;
             pd_idx < sizeof(page_directory) / sizeof(*page_directory);
             ++pd_idx) {
                struct page_directory_entry *pde = page_directory + pd_idx;
                if (!(pde->attributes & PTDE_PRESENT)) {
                        page_directory[pd_idx] = create_pde(pd_idx, attr);
                }
                if (!is_subset_rough(pde->attributes, attr))
                        continue;
                struct page_table_entry *pt = page_tables_virt[pd_idx];
                size_t pt_idx = find_consecutive_entries(pd_idx, pt, cnt);
                if (pt_idx == 1024)
                        continue;

                for (size_t i = 0; i < cnt; ++i) {
                        uint64_t phys_addr = mm_get_free_page();
                        pt[pt_idx + i]     = create_pte(
                            (uintptr_t)phys_addr & MM_ADDR_MASK, 0, attr);
                }
                return (void *)(get_virt_address(pd_idx, pt_idx));
        }
        printf("OUT OF MEMORY\n");
        panic();
}

void *mm_alloc_addr(void *phys_addr, enum mm_gp attributes)
{
        enum ptde_attributes attr = convert_attributes(attributes);
        attr |= PTDE_PRESENT;
        for (size_t pd_idx = 0;
             pd_idx < sizeof(page_directory) / sizeof(*page_directory);
             ++pd_idx) {
                struct page_directory_entry *pde = page_directory + pd_idx;
                if (!(pde->attributes & PTDE_PRESENT)) {
                        page_directory[pd_idx] = create_pde(pd_idx, attr);
                }
                if (!is_subset_rough(pde->attributes, attr))
                        continue;
                struct page_table_entry *pt = page_tables_virt[pd_idx];
                for (size_t pt_idx = 0; pt_idx < 1024; ++pt_idx) {
                        if (!(pt[pt_idx].attributes & PTDE_PRESENT)) {
                                pt[pt_idx] = create_pte((uintptr_t)phys_addr &
                                                            MM_ADDR_MASK,
                                                        0,
                                                        attr);
                                return (
                                    void *)(get_virt_address(pd_idx, pt_idx) +
                                            ((uintptr_t)phys_addr %
                                             MM_PAGE_SIZE));
                        }
                }
        }
        printf("OUT OF MEMORY\n");
        panic();
}

void mm_free_page(void *addr, uint8_t phys_alloc)
{
        struct page_table_entry *pte = get_pte_from_address((uintptr_t)addr);
        if (!phys_alloc)
                mm_add_page(pte->addr & MM_ADDR_MASK);
        pte->val = 0;
        __asm__ volatile("invlpg [%0]" ::"r"(addr) : "memory");
}

void mm_get_page_info(void *addr)
{
        printf("Addr: %p\n", addr);
        printf("PTE:\n");
        const struct page_table_entry *pte =
            get_pte_from_address((uintptr_t)addr);
        printf("  Physical address: %p\n", pte->val & MM_ADDR_MASK);
        printf("  ATTRIBUTES:\n");
        printf("    Write: %u\n", pte->attributes & PTDE_WRITE ? 1 : 0);
        printf("    User: %u\n", pte->attributes & PTDE_USER_MODE ? 1 : 0);
        printf("    Accessed: %u\n", pte->attributes & PTDE_ACCESSED ? 1 : 0);
        printf("    Dirty: %u\n", pte->attributes & PTE_DIRTY ? 1 : 0);
        size_t index                          = (uintptr_t)addr >> (32 - 10);
        const struct page_directory_entry pde = page_directory[index];
        if (pde.attributes & PTDE_PRESENT) {
                printf("PTE: %p\n", pde.val & MM_ADDR_MASK);
                printf("  ATTRIBUTES:\n");
                printf("    Write: %u\n", pde.attributes & PTDE_WRITE ? 1 : 0);
                printf("    User: %u\n",
                       pde.attributes & PTDE_USER_MODE ? 1 : 0);
                printf("    Accessed: %u\n",
                       pde.attributes & PTDE_ACCESSED ? 1 : 0);
        }
}

void mm_init(void)
{
        mm_add_memory((uint64_t)mm_get_phys_addr((uintptr_t)scratch_space),
                      sizeof(scratch_space));
        page_tables_virt[768] = page_table1;

        available_pages =
            malloc(sizeof(*available_pages) +
                   available_pages->size * sizeof(*available_pages->addr));
        memcpy(available_pages,
               &available_pages_init,
               sizeof(available_pages_init) +
                   available_pages_init.size *
                       sizeof(*available_pages_init.addr));
        if (!available_pages) {
                puts("Error setting up memory");
                panic();
        }
}
