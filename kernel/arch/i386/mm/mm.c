#include "mm.h"

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <kernel/mm.h>
#include <kernel/panic.h>

#include "cpu.h"
#include "mm/mm.h"

enum : uint32_t {
        MM_ADDR_MASK             = 0xFFFFF000,
        MM_PAGES_INIT_SIZE       = 2,
        MM_PAGE_TABLES_VIRT_BASE = 0xFFC00000,
        MM_PAGE_DIRECTORY_VIRT   = 0xFFFFF000,
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

extern char scratch_space[4096 * MM_PAGES_INIT_SIZE];

struct mm_struct {
        uintptr_t cr3;
        struct page_directory_entry *pd;
};

static struct mm_struct *active_address_space = &(struct mm_struct){};

static struct page_table_entry create_pte(uintptr_t addr, uint8_t global,
                                          uint8_t attributes)
{
        return (struct page_table_entry){
            .val = attributes + ((global & 0x1) << 8) + (addr & MM_ADDR_MASK)};
}

static uintptr_t get_virt_address(size_t pd_idx, size_t pt_idx)
{
        return (pd_idx << (32 - 10)) + (pt_idx << (32 - 20));
}

static struct page_table_entry *get_pt_virt(size_t pd_idx)
{
        return (struct page_table_entry *)(pd_idx * MM_PAGE_SIZE +
                                           MM_PAGE_TABLES_VIRT_BASE);
}

static struct page_directory_entry *get_pd_virt(void)
{
        return (struct page_directory_entry *)MM_PAGE_DIRECTORY_VIRT;
}

static void flush_tlb(void *addr)
{
        __asm__ volatile("invlpg [%0]" ::"r"(addr) : "memory");
}

static struct page_directory_entry
create_pde_phys(uintptr_t addr, enum ptde_attributes attributes)
{
        struct page_directory_entry pde = {.val = addr & MM_ADDR_MASK};
        pde.attributes                  = attributes;
        return pde;
}

static void create_pde(size_t idx, enum ptde_attributes attributes)
{
        uintptr_t phys_addr             = mm_get_free_page();
        struct page_directory_entry *pd = get_pd_virt();
        pd[idx].val                     = phys_addr & MM_ADDR_MASK;
        pd[idx].attributes              = attributes;
        struct page_table_entry *pt     = get_pt_virt(idx);
        flush_tlb(pt);
        memset(pt, 0, MM_PAGE_SIZE);
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
        return get_pd_virt() + get_pde_index_from_address(addr);
}

static struct page_table_entry *get_pte_from_address(uintptr_t addr)
{
        struct page_directory_entry *pde = get_pde_from_address(addr);
        if (!(pde->attributes & PTDE_PRESENT)) {
                printf("%p not present\n", addr);
                panic();
        }
        size_t pd_idx                = get_pde_index_from_address(addr);
        size_t pt_idx                = get_pte_index_from_address(addr);
        struct page_table_entry *pte = get_pt_virt(pd_idx) + pt_idx;
        if (!(pte->attributes & PTDE_PRESENT)) {
                printf("%p not present\n", addr);
                panic();
        }

        return pte;
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

static uint8_t is_subset_rough(enum ptde_attributes bigger,
                               enum ptde_attributes smaller)
{
        const enum ptde_attributes mask =
            PTDE_PRESENT | PTDE_WRITE | PTDE_USER_MODE;
        bigger &= mask;
        smaller &= mask;
        return (bigger & smaller) == smaller;
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

uint64_t mm_get_free_page(void)
{
        if (available_pages->pos == 0) {
                printf("Out of memory\n");
                panic();
        }
        return available_pages->addr[--available_pages->pos];
}

struct mm_struct *mm_create_new_mm(void)
{
        struct mm_struct *ret = malloc(sizeof(*ret));
        if (!ret)
                return nullptr;

        struct page_directory_entry *pd_virt =
            mm_alloc_page(MM_GP_RW | MM_GP_KERNEL);
        if (!pd_virt) {
                free(ret);
                return nullptr;
        }
        uintptr_t pd_phys = mm_get_phys_addr((uintptr_t)pd_virt);

        ret->cr3 = pd_phys;
        ret->pd  = pd_virt;
        memset(ret->pd, 0, MM_PAGE_SIZE);

        for (size_t i = 768; i < 1024; ++i)
                ret->pd[i] = get_pd_virt()[i];

        ret->pd[1023] = (struct page_directory_entry){
            .val = (ret->cr3 & MM_ADDR_MASK) | PTDE_PRESENT | PTDE_WRITE};

        return ret;
}

void mm_switch_address_space(struct mm_struct *space)
{
        if (!space)
                return;
        active_address_space = space;
        __asm__ volatile("mov cr3, %0\n" ::"r"(space->cr3) : "memory");
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
        flush_tlb(addr);
}

// TODO(laburnumT): Use error enum
int mm_map_page(struct mm_struct *space, uintptr_t vaddr, uintptr_t paddr,
                enum mm_gp attributes)
{
        if (!space)
                return 1;
        enum ptde_attributes attr =
            convert_attributes(attributes) | PTDE_PRESENT;

        size_t pd_idx = get_pde_index_from_address(vaddr);
        size_t pt_idx = get_pte_index_from_address(vaddr);

        struct page_directory_entry *pde = &space->pd[pd_idx];
        if (!(pde->attributes & PTDE_PRESENT)) {
                uintptr_t page = mm_get_free_page();
                void *virt_page =
                    mm_alloc_addr((void *)page, MM_GP_RW | MM_GP_KERNEL);
                memset(virt_page, 0, MM_PAGE_SIZE);
                mm_free_page(virt_page, 1);
                *pde = create_pde_phys(page, attr);
        }
        uintptr_t pt_paddr = pde->val & MM_ADDR_MASK;
        struct page_table_entry *pt =
            mm_alloc_addr((void *)pt_paddr, MM_GP_RW | MM_GP_KERNEL);
        pt[pt_idx].val        = paddr & MM_ADDR_MASK;
        pt[pt_idx].attributes = attr;
        mm_free_page(pt, 1);
        flush_tlb((void *)vaddr);

        return 0;
}

int mm_map_range(struct mm_struct *space, uintptr_t vaddr, uintptr_t paddr,
                 size_t cnt, enum mm_gp attributes)
{
        for (size_t i = 0; i < cnt;
             ++i, vaddr += MM_PAGE_SIZE, paddr += MM_PAGE_SIZE) {
                int err = mm_map_page(space, vaddr, paddr, attributes);
                if (err)
                        return err;
        }
        return 0;
}

void *mm_alloc_pages(size_t cnt, enum mm_gp attributes)
{
        if (cnt > available_pages->pos || !cnt)
                return nullptr;

        enum ptde_attributes attr =
            convert_attributes(attributes) | PTDE_PRESENT;

        struct {
                size_t start, stop;
        } range = {
            .start = attr & PTDE_USER_MODE ? 1 : 768,
            .stop  = attr & PTDE_USER_MODE ? 768 : 1023,
        };

        size_t found = 0;
        size_t starting_pd_idx, starting_pt_idx;
        for (size_t pd_idx = range.start; pd_idx < range.stop; ++pd_idx) {
                if (!found)
                        starting_pd_idx = pd_idx;

                struct page_directory_entry *pde = get_pd_virt() + pd_idx;
                if (!(pde->attributes & PTDE_PRESENT))
                        create_pde(pd_idx, attr);

                if (!is_subset_rough(pde->attributes, attr)) {
                        found = 0;
                        continue;
                }
                struct page_table_entry *pt = get_pt_virt(pd_idx);

                for (size_t pt_idx = pd_idx ? 0 : 1;
                     pt_idx < 1024 && found < cnt;
                     ++pt_idx) {
                        if (pt[pt_idx].attributes & PTDE_PRESENT) {
                                found = 0;
                                continue;
                        }
                        if (!found)
                                starting_pt_idx = pt_idx;
                        ++found;
                }

                if (found != cnt)
                        continue;

                for (size_t i      = 0,
                            pd_idx = starting_pd_idx,
                            pt_idx = starting_pt_idx;
                     i < cnt;
                     ++i) {
                        uint64_t phys_addr = mm_get_free_page();
                        auto pt            = get_pt_virt(pd_idx);
                        pt[pt_idx]         = create_pte(
                            (uintptr_t)phys_addr & MM_ADDR_MASK, 0, attr);
                        ++pt_idx;
                        if (pt_idx == 1024) {
                                ++pd_idx;
                                pt_idx = 0;
                        }
                }

                return (
                    void *)(get_virt_address(starting_pd_idx, starting_pt_idx));
        }
        printf("OUT OF MEMORY\n");
        panic();
}

void *mm_alloc_addr(void *phys_addr, enum mm_gp attributes)
{
        enum ptde_attributes attr = convert_attributes(attributes);
        attr |= PTDE_PRESENT;

        struct {
                size_t start, stop;
        } range = {
            .start = attr & PTDE_USER_MODE ? 1 : 768,
            .stop  = attr & PTDE_USER_MODE ? 768 : 1023,
        };
        for (size_t pd_idx = range.start; pd_idx < range.stop; ++pd_idx) {
                struct page_directory_entry *pde = get_pd_virt() + pd_idx;
                if (!(pde->attributes & PTDE_PRESENT))
                        create_pde(pd_idx, attr);

                if (!is_subset_rough(pde->attributes, attr))
                        continue;
                struct page_table_entry *pt = get_pt_virt(pd_idx);
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
        flush_tlb(addr);
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
        const struct page_directory_entry pde = get_pd_virt()[index];
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
        active_address_space->cr3 = get_cr3();
        active_address_space->pd =
            (struct page_directory_entry *)MM_PAGE_DIRECTORY_VIRT;
        mm_add_memory((uint64_t)mm_get_phys_addr((uintptr_t)scratch_space),
                      sizeof(scratch_space));

        available_pages =
            malloc(sizeof(*available_pages) +
                   available_pages->size * sizeof(*available_pages->addr));
        if (!available_pages) {
                puts("Error setting up memory");
                panic();
        }
        memcpy(available_pages,
               &available_pages_init,
               sizeof(available_pages_init) +
                   available_pages_init.size *
                       sizeof(*available_pages_init.addr));
}
