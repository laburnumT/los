#include <_helper.h>
#include <stdatomic.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include <kernel/fs/romfs.h>
#include <kernel/fs/vfs.h>

enum : size_t {
        HASH_CALC = (size_t)(0.6180339887498948482 * ((double)SIZE_MAX + 1.0)),
        HASH_BITS = sizeof(size_t) * 8,
};

struct vfs_vnode_table {
        size_t cnt, size;
        struct vfs_node **nodes;
};

static size_t scramble(size_t vfs_id, size_t size)
{
        size_t hash  = vfs_id * HASH_CALC;
        size_t shift = HASH_BITS - __builtin_ctzg(size);
        return hash >> shift;
}

struct vfs_node *vfs_node_ref(struct vfs_node *node)
{
        if (!node)
                return nullptr;
        if (atomic_fetch_add(&node->ref_cnt, 1) == 0)
                vfs_lookup_table_add(node->mnt, node);
        return node;
}

struct vfs_node *vfs_node_ref_init(struct vfs_node *node)
{
        if (!node)
                return nullptr;
        atomic_init(&node->ref_cnt, 0);
        return vfs_node_ref(node);
}

void vfs_node_unref(struct vfs_node *node)
{
        if (!node)
                return;
        if (atomic_fetch_sub(&node->ref_cnt, 1) == 1) {
                vfs_lookup_table_remove(node->mnt, node);
                free(node);
        }
}

struct vfs_file *vfs_file_ref(struct vfs_file *file)
{
        if (!file)
                return nullptr;

        atomic_fetch_add(&file->ref_cnt, 1);

        return file;
}

void vfs_file_unref(struct vfs_file *file)
{
        if (!file)
                return;

        if (atomic_fetch_sub(&file->ref_cnt, 1) > 1)
                return;

        if (file->node) {
                vfs_node_unref(file->node);
                file->node = nullptr;
        }
        free(file);
}

struct vfs_node *vfs_lookup_table_find(const struct vfs_mount *mnt,
                                       size_t vfs_id)
{
        auto table  = mnt->table;
        size_t hash = scramble(vfs_id, table->size);
        size_t h    = hash;
        do {
                auto node = table->nodes[h];
                if (!node)
                        return nullptr;
                if (node->vfs_id == vfs_id)
                        return node;
                h = (h + 1) & (table->size - 1);
        } while (h != hash);

        return nullptr;
}

void vfs_lookup_table_add(struct vfs_mount *mnt, struct vfs_node *node)
{
        if (mnt->table->cnt * 2 >= mnt->table->size) {
                auto old_nodes = mnt->table->nodes;
                auto old_size  = mnt->table->size;
                mnt->table->size *= 2;
                mnt->table->cnt = 0;
                mnt->table->nodes =
                    malloc(mnt->table->size * sizeof(*mnt->table->nodes));
                for (size_t i = 0; i < mnt->table->size; ++i)
                        mnt->table->nodes[i] = nullptr;

                for (size_t i = 0; i < old_size; ++i) {
                        if (old_nodes[i])
                                vfs_lookup_table_add(mnt, old_nodes[i]);
                }
                free(old_nodes);
        }
        auto table  = mnt->table;
        size_t hash = scramble(node->vfs_id, table->size);
        size_t h    = hash;
        do {
                if (!table->nodes[h]) {
                        table->nodes[h] = node;
                        ++table->cnt;
                        return;
                }
                if (table->nodes[h]->vfs_id == node->vfs_id)
                        return;
                h = (h + 1) & (table->size - 1);
        } while (h != hash);
}

void vfs_lookup_table_remove(struct vfs_mount *mnt, const struct vfs_node *node)
{
        auto table  = mnt->table;
        size_t hash = scramble(node->vfs_id, table->size);
        size_t h    = hash;
        do {
                if (!table->nodes[h])
                        return;
                if (table->nodes[h]->vfs_id == node->vfs_id) {
                        table->nodes[h] = nullptr;
                        --table->cnt;
                        size_t hole = h;
                        for (size_t i = (h + 1) & (table->size - 1); i != h;
                             i        = (i + 1) & (table->size - 1)) {
                                auto next_node = table->nodes[i];
                                if (!next_node)
                                        break;
                                size_t ideal =
                                    scramble(next_node->vfs_id, table->size);
                                if (is_between_cyclic(ideal, i, hole)) {
                                        table->nodes[hole] = table->nodes[i];
                                        table->nodes[i]    = nullptr;
                                        hole               = i;
                                }
                        }
                        return;
                }
                h = (h + 1) & (table->size - 1);
        } while (h != hash);
}

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
                if (!root->mnt || !root->mnt->root)
                        return VFS_ERR_INVALID_ARG;
                root = root->mnt->root;
                for (; *path == '/'; ++path)
                        ;
        }

        if (!*path) {
                *ret = vfs_node_ref(root);
                return VFS_ERR_OK;
        }

        auto current     = vfs_node_ref(root);
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
                        if (!tmp) {
                                vfs_node_unref(current);
                                return VFS_ERR_NO_MEMORY;
                        }
                        tmp_name = tmp;
                }
                memcpy(tmp_name, path, len);
                tmp_name[len] = '\0';

                path += len;
                for (; *path == '/'; ++path)
                        ;

                if (current->type != VFS_FT_DIR) {
                        vfs_node_unref(current);
                        return VFS_ERR_INVALID_FILE_TYPE;
                }
                if (!current->ops->lookup) {
                        vfs_node_unref(current);
                        return VFS_ERR_OP_NOT_SUPPORTED;
                }
                struct vfs_node *child = nullptr;
                enum vfs_err err =
                    current->ops->lookup(current, tmp_name, &child);
                if (err) {
                        vfs_node_unref(current);
                        return err;
                }
                vfs_node_ref(child);
                vfs_node_unref(current);
                current = child;
        }

        *ret = current;
        return VFS_ERR_OK;
}

enum vfs_err vfs_mount(enum vfs_filesystem fs_type, struct vfs_dev_iface *iface,
                       struct vfs_mount **ret)
{
        enum vfs_err err;
        switch (fs_type) {
        case VFS_FS_ROMFS:
                err = romfs_mount(iface, ret);
                break;
        default:
                return VFS_ERR_INVALID_ARG;
        }
        if (err)
                return err;
        auto mnt         = *ret;
        mnt->table       = malloc(sizeof(*mnt->table));
        mnt->table->size = 1 << 4;
        mnt->table->cnt  = 0;
        mnt->table->nodes =
            malloc(mnt->table->size * sizeof(*mnt->table->nodes));
        for (size_t i = 0; i < mnt->table->size; ++i)
                mnt->table->nodes[i] = nullptr;

        atomic_init(&mnt->ref_cnt, 1);
        vfs_node_ref_init(mnt->root);
        return VFS_ERR_OK;
}

enum vfs_err vfs_unmount(struct vfs_mount *mnt)
{
        if (!mnt)
                return VFS_ERR_INVALID_ARG;

        // mnt->table->cnt == 1 is for mnt->root
        if (atomic_load(&mnt->ref_cnt) > 1 || mnt->table->cnt > 1 ||
            atomic_load(&mnt->root->ref_cnt) > 1)
                return VFS_ERR_BUSY;

        vfs_node_unref(mnt->root);

        free(mnt->table->nodes);
        free(mnt->table);

        free(mnt);

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
                vfs_node_unref(node);
                return VFS_ERR_INVALID_FILE_TYPE;
        }

        struct vfs_file *file = malloc(sizeof(*file));
        if (!file) {
                vfs_node_unref(node);
                return VFS_ERR_NO_MEMORY;
        }

        atomic_init(&file->ref_cnt, 1);
        file->node   = node;
        file->offset = 0;

        *ret = file;
        return VFS_ERR_OK;
}

enum vfs_err vfs_close(struct vfs_file *file)
{
        if (!file)
                return VFS_ERR_INVALID_ARG;

        vfs_file_unref(file);
        return VFS_ERR_OK;
}
