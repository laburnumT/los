#include <_dynqueue.h>
#include <libelf.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <kernel/panic.h>
#include <kernel/sched.h>

static struct los_process **pqueue  = nullptr;
static struct los_process *pcurrent = nullptr;

struct los_process *sched_get_current(void)
{
        return pcurrent;
}

void sched_add_process(const char *name, struct libelf_state *state,
                       size_t user_stack_size, uint8_t can_preempt)
{
        static size_t pid_cnt           = 1;
        struct los_process *new_process = malloc(sizeof(*new_process));
        new_process->pid                = pid_cnt;
        new_process->name               = strdup(name);
        new_process->can_preempt        = can_preempt;
        new_process->state              = state;

        sched_arch_init_process(new_process, user_stack_size);

        new_process->status = LOS_P_NEW;

        pqueue = los_queue_push(pqueue, new_process);
}

static void free_process(struct los_process *process)
{
        if (!process)
                return;
        sched_arch_free_process(process);
        free(process->name);
        free(process);
}

void sched_remove_process(int status)
{
        auto process    = pcurrent;
        process->status = LOS_P_DEAD;
        free_process(process);
        pcurrent = nullptr;
        schedule();
}

void schedule(void)
{
        if (!pcurrent && !los_queue_size(pqueue)) {
                puts("Cannot schedule as there are no programs to schedule");
                panic();
        }

        if (!los_queue_size(pqueue))
                return;
        if (pcurrent && !pcurrent->can_preempt)
                return;
        auto process = los_queue_pop(pqueue);
        if (process->status == LOS_P_DEAD) {
                free_process(process);
                return;
        }
        if (pcurrent)
                los_queue_push(pqueue, pcurrent);

        pcurrent = process;

        sched_arch_switch_context(pcurrent);
}
