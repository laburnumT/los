#ifndef LOS_KERNEL_SCHED_H
#define LOS_KERNEL_SCHED_H

#include <libelf.h>
#include <stddef.h>
#include <stdint.h>

struct los_process {
        size_t pid;
        char *name;
        void *context;
        unsigned char *kernel_stack_bottom;
        unsigned char *kernel_stack_top;
        size_t user_stack_size;
        struct libelf_state *state;
        enum : uint8_t {
                LOS_P_RUNNING,
                LOS_P_IDLE,
                LOS_P_NEW,
                LOS_P_DEAD,
        } status;
        uint8_t can_preempt;
};

struct los_process *sched_get_current(void);
void sched_add_process(const char *name, struct libelf_state *state,
                       size_t user_stack_size, uint8_t can_preempt);
void sched_remove_process(int status);
void schedule(void);

void sched_arch_init_process(struct los_process *process,
                             size_t user_stack_size);
void sched_arch_switch_context(struct los_process *pcurrent);
void sched_arch_free_process(struct los_process *process);

#endif
