#ifndef LOS_KERNEL_FS_ROMFS_H
#define LOS_KERNEL_FS_ROMFS_H

#include <stdint.h>

#include <kernel/fs/vfs.h>

enum vfs_err romfs_mount(struct vfs_dev_iface *iface, struct vfs_mount **ret);
uint8_t romfs_is_valid(struct vfs_dev_iface *iface);

#endif
