#ifndef LOS_KERNEL_INIT_H
#define LOS_KERNEL_INIT_H

#include <stddef.h>
#include <stdint.h>

void init_arch(void *boot_info, uint32_t magic);

void jump_to_userspace(unsigned char *exec, size_t stack_size);

#endif
