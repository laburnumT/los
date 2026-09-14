#ifndef LOS_LIB_DYNARRAY_H
#define LOS_LIB_DYNARRAY_H

#include <stddef.h>
#include <stdlib.h>

enum { LOS_ARR_INIT_SIZE = 10 };

struct los_arr_header {
        size_t cnt, size;
};

#define los_arr_len(arr) ((arr) ? ((struct los_arr_header *)(arr) - 1)->cnt : 0)

#define los_arr_append(arr, elem)                                              \
        do {                                                                   \
                if (!(arr)) {                                                  \
                        struct los_arr_header *header =                        \
                            malloc(sizeof(*header) +                           \
                                   LOS_ARR_INIT_SIZE * sizeof(*(arr)));        \
                        header->cnt  = 0;                                      \
                        header->size = LOS_ARR_INIT_SIZE;                      \
                        (arr)        = (void *)(header + 1);                   \
                }                                                              \
                auto header = (struct los_arr_header *)(arr) - 1;              \
                if (header->cnt >= header->size) {                             \
                        header->size *= 2;                                     \
                        header = realloc(header,                               \
                                         sizeof(*header) +                     \
                                             header->size * sizeof(*(arr)));   \
                        (arr)  = (void *)(header + 1);                         \
                }                                                              \
                (arr)[header->cnt++] = (elem);                                 \
        } while (0)

#define los_arr_resize(arr, size)                                                  \
        do {                                                                       \
                if (!(arr)) {                                                      \
                        struct los_arr_header *header =                            \
                            malloc(sizeof(*header) + (size) * sizeof(*(arr)));     \
                        header->cnt  = 0;                                          \
                        header->size = (size);                                     \
                        (arr)        = (void *)(header + 1);                       \
                }                                                                  \
                auto header = (struct los_arr_header *)(arr) - 1;                  \
                if (header->size != (size)) {                                      \
                        header->size = (size);                                     \
                        header       = realloc(header,                             \
                                         sizeof(*header) +                   \
                                             header->size * sizeof(*(arr))); \
                        (arr)        = (void *)(header + 1);                       \
                }                                                                  \
        } while (0)

#define los_arr_free(arr) free((struct los_arr_header *)(arr) - 1)

#endif
