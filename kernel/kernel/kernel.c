#include <_dynarray.h>
#include <libelf.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <kernel/fs/vfs.h>
#include <kernel/init.h>
#include <kernel/panic.h>
#include <kernel/sched.h>
#include <kernel/tty.h>

struct ram_ctx {
        unsigned char *start;
        size_t len;
};

static enum vfs_err read(const struct vfs_dev_iface *iface, size_t lba,
                         size_t count, void *buffer)
{
        auto ctx = (struct ram_ctx *)iface->private.ptr;

        size_t offset = lba * iface->block_size;
        size_t size   = count * iface->block_size;

        if (offset + size > ctx->len)
                return -1;
        memcpy(buffer, ctx->start + offset, size);

        return VFS_ERR_OK;
}

void kernel_main(void *boot_info, unsigned int magic)
{
        tty_initialise();
        auto mods           = init_arch(boot_info, magic);
        char *exec_name     = "/bin/lsh";
        unsigned char *exec = nullptr;

        for (size_t i = 0; i < los_arr_len(mods); ++i) {
                if (mods[i].type != MOD_ROMFS)
                        continue;

                struct vfs_dev_iface iface = {
                    .block_size = 1,
                    .private.ptr =
                        &(struct ram_ctx){
                            .start = mods[i].addr,
                            .len   = mods[i].len,
                        },
                    .read = read,
                };
                struct vfs_mount *mount = nullptr;
                if (vfs_mount(VFS_FS_ROMFS, &iface, &mount)) {
                        puts("Failed to mount romfs filesystem");
                        panic();
                }

                struct vfs_file *fp = nullptr;
                if (vfs_open(mount->root, exec_name, &fp)) {
                        printf("Failed to open %s\n", exec_name);
                        panic();
                }
                exec = malloc(fp->node->size);
                if (!exec) {
                        printf("Failed to allocate %zu bytes\n",
                               fp->node->size);
                        panic();
                }
                if (vfs_read(fp, exec, fp->node->size) < 0) {
                        printf("Failed to read %s\n", exec_name);
                        panic();
                }
                if (vfs_close(fp)) {
                        printf("Failed to close %s\n", exec_name);
                        panic();
                }
                break;
        }

        struct libelf_state state1;
        if (libelf_load_elf(exec, &state1) != LIBELF_OK) {
                puts("Failed to make the jump to userspace. Error loading elf");
                panic();
        }
        sched_add_process("lsh1", &state1, 16384, 1);
        struct libelf_state state2;
        if (libelf_load_elf(exec, &state2) != LIBELF_OK) {
                puts("Failed to make the jump to userspace. Error loading elf");
                panic();
        }
        sched_add_process("lsh2", &state2, 16384, 1);

        schedule();
}
