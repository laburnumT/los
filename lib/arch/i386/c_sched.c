#include <sched.h>

enum sched_error sched_yield(void)
{
        __asm__("mov eax, 4\n"
                "int 0x80\n" ::
                    : "eax");
        return SCHED_OK;
}
