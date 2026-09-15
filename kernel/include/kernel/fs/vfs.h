#ifndef LOS_KERNEL_FS_VFS_H
#define LOS_KERNEL_FS_VFS_H

#include <stddef.h>
#include <stdint.h>

enum vfs_filetype : uint8_t {
        VFS_FT_FILE,
        VFS_FT_DIR,
        VFS_FT_HARD_LINK,
        VFS_FT_SYM_LINK,
};

enum vfs_filesystem : uint8_t {
        VFS_FS_ROMFS,
};

enum vfs_err {
        VFS_ERR_OK                = 0,
        VFS_ERR_GENERIC           = -1,
        VFS_ERR_INVALID_ARG       = -2,
        VFS_ERR_INVALID_FILE_TYPE = -3,
        VFS_ERR_NO_MEMORY         = -4,
        VFS_ERR_OP_NOT_SUPPORTED  = -5,
        VFS_ERR_NOT_FOUND         = -6,
        VFS_ERR_CORRUPT           = -7,
        VFS_ERR_CHECKSUM          = -8,
};

struct vfs_dev_iface {
        size_t block_size;
        union {
                void *ptr;
                size_t offset;
        } private;
        enum vfs_err (*read)(const struct vfs_dev_iface *iface, size_t lba,
                             size_t count, void *buffer);
};

struct vfs_mount {
        enum vfs_filesystem fs_type;
        struct vfs_node *root;
        struct vfs_dev_iface *iface;
};

struct vfs_node {
        enum vfs_filetype type;
        size_t size;
        const struct vfs_ops *ops;
        struct vfs_mount *mount;
        union {
                void *ptr;
                size_t offset;
        } private;
};

struct vfs_ops {
        int (*read)(struct vfs_node *node, size_t offset, size_t size,
                    void *buffer);
        enum vfs_err (*lookup)(struct vfs_node *node, const char *name,
                               struct vfs_node **ret);
};

struct vfs_file {
        struct vfs_node *node;
        size_t offset;
};

enum vfs_err vfs_mount(enum vfs_filesystem fs_type, struct vfs_dev_iface *iface,
                       struct vfs_mount **ret);
enum vfs_err vfs_unmount(struct vfs_mount *mount);
enum vfs_err vfs_open(struct vfs_node *root, const char *path,
                      struct vfs_file **ret);
enum vfs_err vfs_close(struct vfs_file *file);

int vfs_read(struct vfs_file *file, void *buffer, size_t count);
enum vfs_err vfs_lookup(struct vfs_node *root, const char *path,
                        struct vfs_node **ret);

enum vfs_err vfs_iface_read_bytes(struct vfs_dev_iface *iface, size_t offset,
                                  size_t size, void *buffer);

#endif
