#ifndef LOS_LIB_SCHED_H
#define LOS_LIB_SCHED_H

enum sched_error {
        SCHED_OK,
        SCHED_UNDEFINED,
};

enum sched_error sched_yield(void);

#endif
