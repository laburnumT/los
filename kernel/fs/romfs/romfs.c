#include <_helper.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <kernel/fs/romfs.h>
#include <kernel/fs/vfs.h>

enum {
        ROMFS_OFFSET_TYPE = 0b111,
        ROMFS_OFFSET_MASK = ~0xF,
};

enum romfs_file_type : uint8_t {
        ROMFS_FT_HARD_LINK,
        ROMFS_FT_DIR,
        ROMFS_FT_REGULAR_FILE,
        ROMFS_FT_SYMLINK,
        ROMFS_FT_BLOCK_DEV,
        ROMFS_FT_CHAR_DEV,
        ROMFS_FT_SOCKET,
        ROMFS_FT_FIFO,
};

struct romfs_fs_header {
        unsigned char magic[8];
        uint32_t full_size;
        uint32_t checksum;
        char volume_name[]; // Null terminated and padded to 16 bytes
} __attribute__((packed));

struct romfs_file_header {
        uint32_t next_offset;
        uint32_t info;
        uint32_t size;
        uint32_t checksum;
        char file_name[]; // Null terminated and padded to 16 bytes
} __attribute__((packed));

static int romfs_read(struct vfs_node *node, size_t offset, size_t size,
                      void *buffer);
static enum vfs_err romfs_lookup(struct vfs_node *node, const char *name,
                                 struct vfs_node **ret);

#ifdef LOS_ENDIAN_SMALL
static uint32_t read_big_endian(uint32_t x)
{
        return ((x & 0x000000ff) << 24) | ((x & 0x0000ff00) << 8) |
               ((x & 0x00ff0000) >> 8) | ((x & 0xff000000) >> 24);
}
#else
static uint32_t read_big_endian(uint32_t x)
{
        return x;
}
#endif

static uint8_t is_romfs_magic(const unsigned char *data)
{
        static const unsigned char magic[] = "-rom1fs-";
        for (size_t i = 0; i < sizeof(magic); ++i) {
                if (data[i] != magic[i])
                        return 0;
        }
        return 1;
}

static uint32_t get_checksum(unsigned char *data, uint32_t size)
{
        auto words        = (uint32_t *)data;
        uint32_t checksum = 0;
        for (size_t i = 0; i < size / sizeof(*words); ++i) {
                if (i == 3)
                        continue;
                checksum += read_big_endian(words[i]);
        }
        return -checksum;
}

static const struct vfs_ops romfs_file_ops = {.read = romfs_read};
static const struct vfs_ops romfs_dir_ops  = {.lookup = romfs_lookup};

static enum vfs_err romfs_read_name(struct vfs_dev_iface *iface, size_t offset,
                                    char **buffer)
{
        size_t len  = 16;
        size_t size = 16;
        char *buf   = malloc(len * sizeof(*buf));
        if (!buf)
                return VFS_ERR_NO_MEMORY;

        for (;;) {
                auto err = vfs_iface_read_bytes(
                    iface, offset + len - 16, 16, buf + len - 16);
                if (err) {
                        free(buf);
                        return err;
                }
                if (!buf[len - 1])
                        break;
                len += 16;
                if (len < size)
                        continue;
                size *= 2;
                char *tmp = realloc(buf, size * sizeof(*buf));
                if (!tmp) {
                        free(buf);
                        return VFS_ERR_NO_MEMORY;
                }
                buf = tmp;
        }
        *buffer = buf;
        return VFS_ERR_OK;
}

static int romfs_read(struct vfs_node *node, size_t offset, size_t size,
                      void *buffer)
{
        if (!node || !buffer)
                return VFS_ERR_INVALID_ARG;
        if (offset >= node->size)
                return 0;

        size = min(size, node->size - offset);

        auto iface         = node->mount->iface;
        auto file_offset   = node->private.offset;
        size_t true_offset = file_offset + offset;

        auto err = vfs_iface_read_bytes(iface, true_offset, size, buffer);
        if (err)
                return err;

        return (int)size;
}

static enum vfs_err romfs_lookup(struct vfs_node *node, const char *name,
                                 struct vfs_node **ret)
{
        if (!node || !name || !ret)
                return VFS_ERR_INVALID_ARG;
        if (node->type != VFS_FT_DIR)
                return VFS_ERR_INVALID_FILE_TYPE;

        auto iface = node->mount->iface;
        struct romfs_file_header dir_hdr;
        auto err = vfs_iface_read_bytes(
            iface, node->private.offset, sizeof(dir_hdr), &dir_hdr);
        if (err)
                return err;

        struct romfs_file_header hdr;
        for (size_t offset = read_big_endian(dir_hdr.info); offset;
             offset = read_big_endian(hdr.next_offset) & ROMFS_OFFSET_MASK) {
                auto err =
                    vfs_iface_read_bytes(iface, offset, sizeof(hdr), &hdr);
                if (err)
                        return err;
                char *file_name = nullptr;
                err = romfs_read_name(iface, offset + sizeof(hdr), &file_name);
                if (err)
                        return err;
                defer
                {
                        free(file_name);
                };
                if (strcmp(file_name, name))
                        continue;

                size_t data_offset =
                    (offset + sizeof(hdr) + strlen(name) + 1 + 15) / 16 * 16;

                {
                        unsigned char *data_checksum =
                            malloc(data_offset - offset);
                        if (!data_checksum)
                                return VFS_ERR_NO_MEMORY;
                        defer
                        {
                                free(data_checksum);
                        };
                        err = vfs_iface_read_bytes(
                            iface, offset, data_offset - offset, data_checksum);
                        if (err)
                                return err;
                        if (get_checksum(data_checksum, data_offset - offset) !=
                            read_big_endian(hdr.checksum))
                                return VFS_ERR_CHECKSUM;
                }

                struct vfs_node *new_node = malloc(sizeof(*new_node));
                if (!new_node)
                        return VFS_ERR_NO_MEMORY;

                enum romfs_file_type type =
                    read_big_endian(hdr.next_offset) & ROMFS_OFFSET_TYPE;
                new_node->mount = node->mount;
                new_node->size  = read_big_endian(hdr.size);

                switch (type) {
                case ROMFS_FT_DIR:
                        new_node->type           = VFS_FT_DIR;
                        new_node->ops            = &romfs_dir_ops;
                        new_node->private.offset = offset;
                        break;
                case ROMFS_FT_REGULAR_FILE:
                        new_node->type           = VFS_FT_FILE;
                        new_node->ops            = &romfs_file_ops;
                        new_node->private.offset = data_offset;
                        break;
                case ROMFS_FT_HARD_LINK:
                case ROMFS_FT_SYMLINK:
                case ROMFS_FT_BLOCK_DEV:
                case ROMFS_FT_CHAR_DEV:
                case ROMFS_FT_SOCKET:
                case ROMFS_FT_FIFO:
                        free(new_node);
                        printf("Unhandled ROMFS type: %u\n", type);
                        return VFS_ERR_GENERIC;
                }

                *ret = new_node;
                return VFS_ERR_OK;
        }
        return VFS_ERR_NOT_FOUND;
}

uint8_t romfs_is_valid(struct vfs_dev_iface *iface)
{
        struct romfs_fs_header fs_header;
        if (vfs_iface_read_bytes(iface, 0, sizeof(fs_header), &fs_header))
                return 0;

        if (!is_romfs_magic(fs_header.magic))
                return 0;

        auto size = read_big_endian(fs_header.full_size);
        size      = min(size, 512);

        unsigned char data_checksum[512];
        if (vfs_iface_read_bytes(iface, 0, size, data_checksum))
                return 0;

        if (get_checksum(data_checksum, size) !=
            read_big_endian(fs_header.checksum))
                return 0;

        return 1;
}

enum vfs_err romfs_mount(struct vfs_dev_iface *iface, struct vfs_mount **ret)
{
        if (!iface || !ret)
                return VFS_ERR_INVALID_ARG;
        if (!romfs_is_valid(iface))
                return VFS_ERR_CORRUPT;

        struct vfs_mount *mount = malloc(sizeof(*mount));
        if (!mount)
                return VFS_ERR_NO_MEMORY;

        mount->fs_type = VFS_FS_ROMFS;
        mount->iface   = iface;

        struct romfs_fs_header fs_header;
        auto err =
            vfs_iface_read_bytes(iface, 0, sizeof(fs_header), &fs_header);
        if (err) {
                free(mount);
                return err;
        }

        char *volume_name = nullptr;
        err = romfs_read_name(iface, sizeof(fs_header), &volume_name);
        if (err) {
                free(mount);
                return err;
        }

        size_t root_offset =
            (sizeof(fs_header) + strlen(volume_name) + 1 + 15) / 16 * 16;

        free(volume_name);

        struct vfs_node *root = malloc(sizeof(*root));
        if (!root) {
                free(mount);
                return VFS_ERR_NO_MEMORY;
        }

        root->type           = VFS_FT_DIR;
        root->mount          = mount;
        root->ops            = &romfs_dir_ops;
        root->private.offset = root_offset;
        root->size           = 0;

        mount->root = root;

        *ret = mount;
        return VFS_ERR_OK;
}
