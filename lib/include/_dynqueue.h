#ifndef LOS_LIB__DYNQUEUE_H
#define LOS_LIB__DYNQUEUE_H

#include <stddef.h>
#include <stdlib.h>
#include <string.h>

enum { LOS_QUEUE_INIT_SIZE = 10 };

struct los_queue_header {
        size_t head, tail, size;
};

#define los_queue_size(queue)                                                  \
        ({                                                                     \
                auto header = (struct los_queue_header *)(queue) - 1;          \
                header->head <= header->tail                                   \
                    ? header->tail - header->head                              \
                    : header->size - header->head - header->tail;              \
        })

inline static struct los_queue_header *
los_queue_resize_helper(struct los_queue_header *header, size_t elem_size)
{
        if ((header->tail + 1) % header->size != header->head) {
                return header;
        }
        struct los_queue_header *ret =
            malloc(sizeof(*header) + header->size * 2 * elem_size);
        ret->size       = header->size * 2;
        ret->head       = 0;
        ret->tail       = 0;
        void *old_queue = ret + 1;
        void *new_queue = header + 1;
        for (size_t i = header->head; i != header->tail;
             i        = (i + 1) % header->size)
                memcpy(new_queue + ret->tail++, old_queue + i, elem_size);

        free(header);
        return ret;
}

#define los_queue_push(queue, elem)                                            \
        ({                                                                     \
                auto q = (queue);                                              \
                auto e = (elem);                                               \
                if (!q) {                                                      \
                        struct los_queue_header *header =                      \
                            malloc(sizeof(*header) +                           \
                                   LOS_QUEUE_INIT_SIZE * sizeof(*q));          \
                        header->head = 0;                                      \
                        header->tail = 0;                                      \
                        header->size = LOS_QUEUE_INIT_SIZE;                    \
                        q            = (void *)(header + 1);                   \
                        (queue)      = q;                                      \
                }                                                              \
                auto header     = (struct los_queue_header *)q - 1;            \
                header          = los_queue_resize_helper(header, sizeof(e));  \
                q               = (void *)(header + 1);                        \
                q[header->tail] = e;                                           \
                header->tail    = (header->tail + 1) % header->size;           \
                q;                                                             \
        })

#define los_queue_pop(queue)                                                   \
        ({                                                                     \
                auto q         = (queue);                                      \
                typeof(*q) ret = {};                                           \
                if (q && los_queue_size(q) > 0) {                              \
                        auto header = (struct los_queue_header *)q - 1;        \
                        ret         = q[header->head++];                       \
                }                                                              \
                ret;                                                           \
        })

#define los_queue_free(queue)                                                  \
        do {                                                                   \
                auto q = (queue);                                              \
                if (q)                                                         \
                        free(q);                                               \
        } while (0)

#endif
