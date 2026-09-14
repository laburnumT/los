#ifndef LOS_KERNEL_ARCH_I386_INIT_H
#define LOS_KERNEL_ARCH_I386_INIT_H

#include <stdint.h>

enum {
        INIT_MB_1         = 0x2BADB002,
        INIT_MB_2         = 0x36D76289,
        INIT_MB_MEM       = 0x1 << 0,
        INIT_MB_BOOT_DISK = 0x1 << 1,
        INIT_MB_CMDLINE   = 0x1 << 2,
        INIT_MB_MODS      = 0x1 << 3,
        INIT_MB_MMAP      = 0x1 << 6,
};

struct multiboot_info_1 {
        /* Multiboot info version number */
        uint32_t flags;

        /* Available memory from BIOS */
        uint32_t mem_lower;
        uint32_t mem_upper;

        /* "root" partition */
        uint32_t boot_device;

        /* Kernel command line */
        char *cmdline;

        /* Boot-Module list */
        uint32_t mods_count;
        uint32_t mods_addr;

        union {
                struct {
                        uint32_t tabsize;
                        uint32_t strsize;
                        uint32_t addr;
                        uint32_t reserved;
                } b4;
                struct {
                        uint32_t num;
                        uint32_t size;
                        uint32_t addr;
                        uint32_t shndx;
                } b5;
        };

        /* Memory Mapping buffer */
        uint32_t mmap_length;
        uintptr_t mmap_addr;

        /* Drive Info buffer */
        uint32_t drives_length;
        uint32_t drives_addr;

        /* ROM configuration table */
        uint32_t config_table;

        /* Boot Loader Name */
        uint32_t boot_loader_name;

        /* APM table */
        uint32_t apm_table;

        /* Video */
        uint32_t vbe_control_info;
        uint32_t vbe_mode_info;
        uint16_t vbe_mode;
        uint16_t vbe_interface_seg;
        uint16_t vbe_interface_off;
        uint16_t vbe_interface_len;

        uint64_t framebuffer_addr;
        uint32_t framebuffer_pitch;
        uint32_t framebuffer_width;
        uint32_t framebuffer_height;
        uint8_t framebuffer_bpp;
        uint8_t framebuffer_type;
        uint8_t color_info[5];
} __attribute__((__packed__));

struct multiboot_mmap_entry {
        uint32_t size;
        union {
                uint64_t addr;
                struct {
                        uint32_t addr_low;
                        uint32_t addr_high;
                } __attribute__((__packed__));
        } __attribute__((__packed__));
        union {
                uint64_t len;
                struct {
                        uint32_t len_low;
                        uint32_t len_high;
                } __attribute__((__packed__));
        } __attribute__((__packed__));
        enum : uint32_t {
                INIT_MB_MEM_AVAILABLE = 1,
                INIT_MB_MEM_INIT_MB_MEM_RESERVED,
                INIT_MB_MEM_ACPI_RECLAIMABLE,
                INIT_MB_MEM_NVS,
                INIT_MB_MEM_BADRAM,
        } type;
} __attribute__((__packed__));

#endif
