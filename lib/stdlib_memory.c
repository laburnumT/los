#include "stdlib.h"

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "_helper.h"

#ifdef __is_libk
        #include <kernel/mm.h>
#endif

struct mm_header {
        struct mm_header *next;
#ifdef __is_libk
        enum mm_gp flags;
#endif
        size_t size;
        char memory[];
};

static struct mm_header free_base = {
    .next = &free_base,
    .size = 0,
};

static struct mm_header *get_header(void *ptr)
{
        return (struct mm_header *)((char *)ptr - sizeof(struct mm_header));
}

#ifdef __is_libk
static void set_flags(struct mm_header *h, enum mm_gp flags)
{
        h->flags = flags;
}
static uint8_t flags_match(struct mm_header *h, enum mm_gp flags)
{
        return h->flags == flags;
}
#else
static void set_flags(struct mm_header *h, enum mm_get flags)
{
        (void)h;
        (void)flags;
}
static uint8_t flags_match(struct mm_header *h, enum mm_get flags)
{
        return 1;
}
#endif

static void get_mem(size_t size, enum mm_gp flags)
{
        size += sizeof(struct mm_header);
        if (size < 0x1000)
                size = 0x1000;

        size_t pages          = (size - 1) / 0x1000 + 1;
        struct mm_header *ret = mm_alloc_pages(flags, pages);
        set_flags(ret, flags);
        ret->size = size - sizeof(*ret);
        free(ret->memory);
}

static void *_malloc(size_t size, enum mm_gp flags)
{
        if (!size)
                return nullptr;

        for (struct mm_header *h = free_base.next, *prev = &free_base;;
             prev = h, h = h->next) {
                if (h->size >= size && flags_match(h, flags)) {
                        if (h->size == size) {
                                prev->next = h->next;
                                return h->memory;
                        }
                        if (h->size >= size + sizeof(*h)) {
                                h->size -= size + sizeof(*h);
                                h       = (typeof(h))(h->memory + h->size);
                                h->size = size;
                                return h->memory;
                        }
                }
                if (h->next == &free_base) {
                        get_mem(size, flags);
                        h = &free_base;
                }
        }
}

void *k_malloc(size_t size, enum mm_gp flags)
{
        return _malloc(size, flags);
}

void *malloc(size_t size)
{
        enum mm_gp flags = MM_GP_RW;
        if (__is_defined(__is_libk))
                return _malloc(size, flags | MM_GP_KERNEL);
        if (__is_defined(__is_libc))
                return _malloc(size, flags | MM_GP_USER);
}

// TODO(laburnumT): Don't find new space if current space can be expanded
static void *_realloc(void *ptr, size_t size, enum mm_gp flags)
{
        if (!ptr)
                return malloc(size);

        struct mm_header *h = get_header(ptr);
        if (h->size >= size)
                return nullptr;

        void *new_ptr = _malloc(size, flags);
        if (!new_ptr)
                return nullptr;

        memcpy(new_ptr, h->memory, h->size);
        free(h->memory);
        return new_ptr;
}

void *k_realloc(void *ptr, size_t size, enum mm_gp flags)
{
        return _realloc(ptr, size, flags);
}

void *realloc(void *ptr, size_t size)
{
        enum mm_gp flags = MM_GP_RW;
        if (__is_defined(__is_libk))
                return _realloc(ptr, size, flags | MM_GP_KERNEL);
        if (__is_defined(__is_libc))
                return _realloc(ptr, size, flags | MM_GP_USER);
}

void free(void *ptr)
{
        struct mm_header *h = get_header(ptr);
        size_t h_size       = h->size + sizeof(*h);
        uintptr_t addr      = (uintptr_t)h;
        if (free_base.next == &free_base) {
                free_base.next = h;
                h->next        = &free_base;
                return;
        }
        for (struct mm_header *header = free_base.next, *prev = &free_base;
             header != &free_base;
             prev = header, header = header->next) {
                // Left insert
                if (addr + h_size < (uintptr_t)header) {
                        prev->next = h;
                        h->next    = header;
                        return;
                }

                // Left join
                if (addr + h_size == (uintptr_t)header) {
                        h->next    = header->next;
                        prev->next = h;
                        h->size += header->size + sizeof(*header);
                        return;
                }

                // Right join
                if ((uintptr_t)header->memory + header->size == addr) {
                        header->size += h_size;
                        // Left join
                        auto next = header->next;
                        if ((uintptr_t)header->memory + header->size ==
                            (uintptr_t)next) {
                                header->size += next->size + sizeof(*next);
                                header->next = next->next;
                        }
                        return;
                }

                // Right insert
                if (header->next == &free_base) {
                        header->next = h;
                        h->next      = &free_base;
                        return;
                }
        }
        puts("Error freeing memory");
}
