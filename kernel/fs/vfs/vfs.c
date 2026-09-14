#include <stdlib.h>
#include <string.h>

#include <kernel/fs/romfs.h>
#include <kernel/fs/vfs.h>

enum vfs_err vfs_iface_read_bytes(struct vfs_dev_iface *iface, size_t offset,
                                  size_t size, void *buffer)
{
        size_t lba           = offset / iface->block_size;
        size_t offset_in_lba = offset % iface->block_size;

        size_t block_cnt =
            (offset_in_lba + size + iface->block_size - 1) / iface->block_size;

        unsigned char *tmp_blocks = malloc(block_cnt * iface->block_size);
        if (!tmp_blocks)
                return VFS_ERR_NO_MEMORY;
        defer
        {
                free(tmp_blocks);
        };

        auto err = iface->read(iface, lba, block_cnt, tmp_blocks);
        if (err)
                return err;
        memcpy(buffer, tmp_blocks + offset_in_lba, size);

        return VFS_ERR_OK;
}

enum vfs_err vfs_lookup(struct vfs_node *root, const char *path,
                        struct vfs_node **ret)
{
        if (!root || !path || !ret)
                return VFS_ERR_INVALID_ARG;
        if (root->type != VFS_FT_DIR)
                return VFS_ERR_INVALID_FILE_TYPE;

        if (*path == '/') {
                root = root->mount->root;
                for (; *path == '/'; ++path)
                        ;
        }

        if (!*path) {
                *ret = root;
                return VFS_ERR_OK;
        }

        auto current     = root;
        size_t name_size = 16;
        char *tmp_name   = malloc(name_size * sizeof(*tmp_name));
        defer
        {
                free(tmp_name);
        };
        for (size_t len = 0; *path;) {
                len = 0;
                for (; path[len] && path[len] != '/'; ++len)
                        ;

                if (len + 1 >= name_size) {
                        name_size *= 2;
                        char *tmp =
                            realloc(tmp_name, name_size * sizeof(*tmp_name));
                        if (!tmp)
                                return VFS_ERR_NO_MEMORY;
                        tmp_name = tmp;
                }
                memcpy(tmp_name, path, len);
                tmp_name[len] = '\0';

                path += len;
                for (; *path == '/'; ++path)
                        ;

                if (current->type != VFS_FT_DIR) {
                        if (current != root)
                                free(current);
                        return VFS_ERR_INVALID_FILE_TYPE;
                }
                if (!current->ops->lookup) {
                        if (current != root)
                                free(current);
                        return VFS_ERR_OP_NOT_SUPPORTED;
                }
                struct vfs_node *child = nullptr;
                enum vfs_err err =
                    current->ops->lookup(current, tmp_name, &child);
                if (err) {
                        if (current != root)
                                free(current);
                        return err;
                }
                if (current != root)
                        free(current);
                current = child;
        }

        *ret = current;
        return VFS_ERR_OK;
}

enum vfs_err vfs_mount(enum vfs_filesystem fs_type, struct vfs_dev_iface *iface,
                       struct vfs_mount **ret)
{
        switch (fs_type) {
        case VFS_FS_ROMFS:
                return romfs_mount(iface, ret);
        default:
                return VFS_ERR_INVALID_ARG;
        }
}

enum vfs_err vfs_unmount(struct vfs_mount *mount)
{
        if (!mount)
                return VFS_ERR_INVALID_ARG;
        if (mount->root) {
                free(mount->root);
                mount->root = nullptr;
        }
        free(mount);

        return VFS_ERR_OK;
}

int vfs_read(struct vfs_file *file, void *buffer, size_t count)
{
        if (!file || !file->node)
                return VFS_ERR_INVALID_ARG;

        if (!file->node->ops->read)
                return VFS_ERR_OP_NOT_SUPPORTED;

        int bytes_read =
            file->node->ops->read(file->node, file->offset, count, buffer);
        if (bytes_read > 0)
                file->offset += bytes_read;

        return bytes_read;
}

enum vfs_err vfs_open(struct vfs_node *root, const char *path,
                      struct vfs_file **ret)
{
        if (!root || !path || !ret)
                return VFS_ERR_INVALID_ARG;

        struct vfs_node *node = nullptr;
        enum vfs_err err      = vfs_lookup(root, path, &node);
        if (err)
                return err;
        if (node->type != VFS_FT_FILE) {
                if (node != root)
                        free(node);
                return VFS_ERR_INVALID_FILE_TYPE;
        }

        struct vfs_file *file = malloc(sizeof(*file));
        if (!file)
                return VFS_ERR_NO_MEMORY;

        file->node   = node;
        file->offset = 0;

        *ret = file;
        return VFS_ERR_OK;
}

enum vfs_err vfs_close(struct vfs_file *file)
{
        if (!file)
                return VFS_ERR_INVALID_ARG;
        free(file->node);
        free(file);
        return VFS_ERR_OK;
}
