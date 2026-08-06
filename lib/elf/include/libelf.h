#ifndef LOS_LIB_LIBELF_H
#define LOS_LIB_LIBELF_H

#include <stddef.h>
#include <stdint.h>

#include "kernel/mm.h"

enum libelf_error : uint8_t {
        LIBELF_OK,
        LIBELF_INVALID_ELF,
        LIBELF_GENERIC_ERROR,
};

enum : uint8_t {
        LIBELF_MAG_0 = 0x7f,
        LIBELF_MAG_1 = 0x45,
        LIBELF_MAG_2 = 0x4c,
        LIBELF_MAG_3 = 0x46,
};

struct elf_ident {
        uint8_t magic[4];
        enum : uint8_t {
                LIBELF_32_BIT = 1,
                LIBELF_64_BIT = 2,
        } bit_format;
        enum : uint8_t {
                LIBELF_LITTLE_ENDIAN = 1,
                LIBELF_BIG_ENDIAN    = 2,
        } endian;
        enum : uint8_t {
                LIBELF_EIV_CURRENT = 1,
        } version;
        enum : uint8_t {
                LIBELF_OS_ABI_SYSTEM_V = 0,
        } os_abi;
        uint8_t abi_version;
        uint8_t padding[7];
} __attribute__((__packed__));

struct elf32_ehdr {
        struct elf_ident ident;
        enum : uint16_t {
                LIBELF_ET_NONE   = 0x0,
                LIBELF_ET_REL    = 0x1,
                LIBELF_ET_EXEC   = 0x2,
                LIBELF_ET_DYN    = 0x3,
                LIBELF_ET_CORE   = 0x4,
                LIBELF_ET_LOOS   = 0xFE00,
                LIBELF_ET_HIOS   = 0xFEFF,
                LIBELF_ET_LOPROC = 0xFF00,
                LIBELF_ET_HIPROC = 0xFFFF,
        } type;
        enum : uint16_t {
                LIBELF_ISA_NONE              = 0x00,
                LIBELF_ISA_ATT_WE_32100      = 0x01,
                LIBELF_ISA_SPARC             = 0x02,
                LIBELF_ISA_X86               = 0x03,
                LIBELF_ISA_M68K              = 0x04,
                LIBELF_ISA_M88K              = 0x05,
                LIBELF_ISA_INTEL_MCU         = 0x06,
                LIBELF_ISA_INTEL_80860       = 0x07,
                LIBELF_ISA_MIPS              = 0x08,
                LIBELF_ISA_IBM_370           = 0x09,
                LIBELF_ISA_MIPS_RS3000_LE    = 0x0A,
                LIBELF_ISA_HP_PA_RISC        = 0x0F,
                LIBELF_ISA_INTEL_80960       = 0x13,
                LIBELF_ISA_POWERPC           = 0x14,
                LIBELF_ISA_POWERPC_64        = 0x15,
                LIBELF_ISA_S390              = 0x16,
                LIBELF_ISA_IBM_SPU           = 0x17,
                LIBELF_ISA_NEC_V800          = 0x24,
                LIBELF_ISA_FUJITSU_FR20      = 0x25,
                LIBELF_ISA_TRW_RH_32         = 0x26,
                LIBELF_ISA_MOTOROLA_RCE      = 0x27,
                LIBELF_ISA_ARM               = 0x28,
                LIBELF_ISA_DIGITAL_ALPHA     = 0x29,
                LIBELF_ISA_SUPERH            = 0x2A,
                LIBELF_ISA_SPARC_V9          = 0x2B,
                LIBELF_ISA_SIEMENS_TRICORE   = 0x2C,
                LIBELF_ISA_ARGONAUT_RISC     = 0x2D,
                LIBELF_ISA_HITACHI_H8_300    = 0x2E,
                LIBELF_ISA_HITACHI_H8_300H   = 0x2F,
                LIBELF_ISA_HITACHI_H8S       = 0x30,
                LIBELF_ISA_HITACHI_H8_500    = 0x31,
                LIBELF_ISA_IA_64             = 0x32,
                LIBELF_ISA_STANFORD_MIPS_X   = 0x33,
                LIBELF_ISA_MOTOROLA_COLDFIRE = 0x34,
                LIBELF_ISA_MOTOROLA_M68HC12  = 0x35,
                LIBELF_ISA_FUJITSU_MMA       = 0x36,
                LIBELF_ISA_SIEMENS_PCP       = 0x37,
                LIBELF_ISA_SONY_NCPU         = 0x38,
                LIBELF_ISA_DENSO_NDR1        = 0x39,
                LIBELF_ISA_MOTOROLA_STARCORE = 0x3A,
                LIBELF_ISA_TOYOTA_ME16       = 0x3B,
                LIBELF_ISA_STM_ST100         = 0x3C,
                LIBELF_ISA_ALC_TINYJ         = 0x3D,
                LIBELF_ISA_AMD_X86_64        = 0x3E,
                LIBELF_ISA_SONY_DSP          = 0x3F,
                LIBELF_ISA_DEC_PDP_10        = 0x40,
                LIBELF_ISA_DEC_PDP_11        = 0x41,
                LIBELF_ISA_SIEMENS_FX66      = 0x42,
                LIBELF_ISA_STM_ST9_PLUS      = 0x43,
                LIBELF_ISA_STM_ST7           = 0x44,
                LIBELF_ISA_MOTOROLA_MC68HC16 = 0x45,
                LIBELF_ISA_MOTOROLA_MC68HC11 = 0x46,
                LIBELF_ISA_MOTOROLA_MC68HC08 = 0x47,
                LIBELF_ISA_MOTOROLA_MC68HC05 = 0x48,
                LIBELF_ISA_SG_SVX            = 0x49,
                LIBELF_ISA_STM_ST19          = 0x4A,
                LIBELF_ISA_DEC_VAX           = 0x4B,
                LIBELF_ISA_AXIS_32           = 0x4C,
                LIBELF_ISA_INFEON_32         = 0x4D,
                LIBELF_ISA_ELEMENT_14        = 0x4E,
                LIBELF_ISA_LSI_LOGIC         = 0x4F,
                LIBELF_ISA_TMS320C6000       = 0x8C,
                LIBELF_ISA_MCST_ELBRUS_E2K   = 0xAF,
                LIBELF_ISA_ARM_64            = 0xB7,
                LIBELF_ISA_ZILOG_Z80         = 0xDC,
                LIBELF_ISA_RISC_V            = 0xF3,
                LIBELF_ISA_BPF               = 0xF7,
                LIBELF_ISA_WDC_65C816        = 0x101,
                LIBELF_ISA_LOONGARCH         = 0x102,
        } machine;
        enum : uint32_t {
                LIBELF_EV_ORIGINAL = 1,
        } version;
        uintptr_t entry;
        size_t phoff;
        size_t shoff;
        uint32_t flags;
        uint16_t ehsize;
        uint16_t phentsize;
        uint16_t phnum;
        uint16_t shentsize;
        uint16_t shnum;
        uint16_t shstrndx;
} __attribute__((__packed__));

struct elf32_phdr {
        enum : uint32_t {
                LIBELF_PT_NULL    = 0,
                LIBELF_PT_LOAD    = 1,
                LIBELF_PT_DYNAMIC = 2,
                LIBELF_PT_INTERP  = 3,
                LIBELF_PT_NOTE    = 4,
                LIBELF_PT_SHLIB   = 5,
                LIBELF_PT_PHDR    = 6,
                LIBELF_PT_TLS     = 7,
                LIBELF_PT_LOOS    = 0x60000000,
                LIBELF_PT_HIOS    = 0x6FFFFFFF,
                LIBELF_PT_LOPROC  = 0x70000000,
                LIBELF_PT_HIPROC  = 0x7FFFFFFF,
        } type;
        size_t offset;
        uintptr_t vaddr;
        uintptr_t paddr;
        size_t filesz;
        size_t memsz;
        enum : uint32_t {
                LIBELF_PF_X = 1,
                LIBELF_PF_W = 2,
                LIBELF_PF_R = 4,
        } flags;
        uint32_t align;
} __attribute__((__packed__));

struct elf32_shdr {
        char *name;
        enum : uint32_t {
                LIBELF_SHT_NULL          = 0x0,
                LIBELF_SHT_PROGBITS      = 0x1,
                LIBELF_SHT_SYMTAB        = 0x2,
                LIBELF_SHT_STRTAB        = 0x3,
                LIBELF_SHT_RELA          = 0x4,
                LIBELF_SHT_HASH          = 0x5,
                LIBELF_SHT_DYNAMIC       = 0x6,
                LIBELF_SHT_NOTE          = 0x7,
                LIBELF_SHT_NOBITS        = 0x8,
                LIBELF_SHT_REL           = 0x9,
                LIBELF_SHT_SHLIB         = 0xA,
                LIBELF_SHT_DYNSYM        = 0xB,
                LIBELF_SHT_INIT_ARRAY    = 0xE,
                LIBELF_SHT_FINI_ARRAY    = 0xF,
                LIBELF_SHT_PREINIT_ARRAY = 0x10,
                LIBELF_SHT_GROUP         = 0x11,
                LIBELF_SHT_SYMTAB_SHNDX  = 0x12,
                LIBELF_SHT_NUM           = 0x13,
                LIBELF_SHT_LOOS          = 0x60000000,
        } type;
        enum : uint32_t {
                LIBELF_SHF_WRITE            = 0x1 << 0,
                LIBELF_SHF_ALLOC            = 0x1 << 1,
                LIBELF_SHF_EXECINSTR        = 0x1 << 2,
                LIBELF_SHF_MERGE            = 0x1 << 4,
                LIBELF_SHF_STRINGS          = 0x1 << 5,
                LIBELF_SHF_INFO_LINK        = 0x1 << 6,
                LIBELF_SHF_LINK_ORDER       = 0x1 << 7,
                LIBELF_SHF_OS_NONCONFORMING = 0x1 << 8,
                LIBELF_SHF_GROUP            = 0x1 << 9,
                LIBELF_SHF_TLS              = 0x1 << 10,
                LIBELF_SHF_MASKOS           = 0x0FF00000,
                LIBELF_SHF_MASKPROC         = 0xF0000000,
                LIBELF_SHF_ORDERED          = 0x4000000,
                LIBELF_SHF_EXCLUDE          = 0x8000000,
        } flags;
        uintptr_t addr;
        size_t offset;
        size_t size;
        size_t link;
        uint32_t info;
        uint32_t addralign;
        size_t entsize;
} __attribute__((__packed__));

struct libelf_state {
        uintptr_t entry;
        struct mm_struct *address_space;
};

enum libelf_error libelf_load_elf(void *file, struct libelf_state *state);

#endif
