#ifndef LOS_KERNEL_INIT_H
#define LOS_KERNEL_INIT_H

#include <stddef.h>
#include <stdint.h>

struct module {
        enum : uint8_t {
                MOD_BYTES,
                MOD_ROMFS,
        } type;
        char *name;
        unsigned char *addr;
        size_t len;
};

struct module *init_arch(void *boot_info, uint32_t magic);

void jump_to_userspace(unsigned char *exec, size_t stack_size);

#endif
