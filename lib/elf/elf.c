#include <_helper.h>
#include <libelf.h>
#include <mm/mm.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <kernel/mm.h>

static enum libelf_error validate_header(const struct elf32_ehdr *hdr)
{
        if (!hdr)
                return LIBELF_INVALID_ELF;

        if (hdr->ident.magic[0] != LIBELF_MAG_0 ||
            hdr->ident.magic[1] != LIBELF_MAG_1 ||
            hdr->ident.magic[2] != LIBELF_MAG_2 ||
            hdr->ident.magic[3] != LIBELF_MAG_3)
                return LIBELF_INVALID_ELF;

        switch (hdr->ident.bit_format) {
        case LIBELF_32_BIT:
                if (hdr->phentsize != sizeof(struct elf32_phdr))
                        return LIBELF_INVALID_ELF;
                break;
        case LIBELF_64_BIT:
        default:
                return LIBELF_INVALID_ELF;
        }

        switch (hdr->ident.endian) {
        case LIBELF_LITTLE_ENDIAN:
                break;
        case LIBELF_BIG_ENDIAN:
        default:
                return LIBELF_INVALID_ELF;
        }

        switch (hdr->ident.version) {
        case LIBELF_EIV_CURRENT:
                break;
        default:
                return LIBELF_INVALID_ELF;
        }

        switch (hdr->type) {
        case LIBELF_ET_EXEC:
                break;
        case LIBELF_ET_DYN:
        case LIBELF_ET_NONE:
        case LIBELF_ET_REL:
        case LIBELF_ET_CORE:
        case LIBELF_ET_LOOS:
        case LIBELF_ET_HIOS:
        case LIBELF_ET_LOPROC:
        case LIBELF_ET_HIPROC:
        default:
                return LIBELF_INVALID_ELF;
        }

        switch (hdr->machine) {
        case LIBELF_ISA_X86:
                break;
        case LIBELF_ISA_NONE:
        case LIBELF_ISA_ATT_WE_32100:
        case LIBELF_ISA_SPARC:
        case LIBELF_ISA_M68K:
        case LIBELF_ISA_M88K:
        case LIBELF_ISA_INTEL_MCU:
        case LIBELF_ISA_INTEL_80860:
        case LIBELF_ISA_MIPS:
        case LIBELF_ISA_IBM_370:
        case LIBELF_ISA_MIPS_RS3000_LE:
        case LIBELF_ISA_HP_PA_RISC:
        case LIBELF_ISA_INTEL_80960:
        case LIBELF_ISA_POWERPC:
        case LIBELF_ISA_POWERPC_64:
        case LIBELF_ISA_S390:
        case LIBELF_ISA_IBM_SPU:
        case LIBELF_ISA_NEC_V800:
        case LIBELF_ISA_FUJITSU_FR20:
        case LIBELF_ISA_TRW_RH_32:
        case LIBELF_ISA_MOTOROLA_RCE:
        case LIBELF_ISA_ARM:
        case LIBELF_ISA_DIGITAL_ALPHA:
        case LIBELF_ISA_SUPERH:
        case LIBELF_ISA_SPARC_V9:
        case LIBELF_ISA_SIEMENS_TRICORE:
        case LIBELF_ISA_ARGONAUT_RISC:
        case LIBELF_ISA_HITACHI_H8_300:
        case LIBELF_ISA_HITACHI_H8_300H:
        case LIBELF_ISA_HITACHI_H8S:
        case LIBELF_ISA_HITACHI_H8_500:
        case LIBELF_ISA_IA_64:
        case LIBELF_ISA_STANFORD_MIPS_X:
        case LIBELF_ISA_MOTOROLA_COLDFIRE:
        case LIBELF_ISA_MOTOROLA_M68HC12:
        case LIBELF_ISA_FUJITSU_MMA:
        case LIBELF_ISA_SIEMENS_PCP:
        case LIBELF_ISA_SONY_NCPU:
        case LIBELF_ISA_DENSO_NDR1:
        case LIBELF_ISA_MOTOROLA_STARCORE:
        case LIBELF_ISA_TOYOTA_ME16:
        case LIBELF_ISA_STM_ST100:
        case LIBELF_ISA_ALC_TINYJ:
        case LIBELF_ISA_AMD_X86_64:
        case LIBELF_ISA_SONY_DSP:
        case LIBELF_ISA_DEC_PDP_10:
        case LIBELF_ISA_DEC_PDP_11:
        case LIBELF_ISA_SIEMENS_FX66:
        case LIBELF_ISA_STM_ST9_PLUS:
        case LIBELF_ISA_STM_ST7:
        case LIBELF_ISA_MOTOROLA_MC68HC16:
        case LIBELF_ISA_MOTOROLA_MC68HC11:
        case LIBELF_ISA_MOTOROLA_MC68HC08:
        case LIBELF_ISA_MOTOROLA_MC68HC05:
        case LIBELF_ISA_SG_SVX:
        case LIBELF_ISA_STM_ST19:
        case LIBELF_ISA_DEC_VAX:
        case LIBELF_ISA_AXIS_32:
        case LIBELF_ISA_INFEON_32:
        case LIBELF_ISA_ELEMENT_14:
        case LIBELF_ISA_LSI_LOGIC:
        case LIBELF_ISA_TMS320C6000:
        case LIBELF_ISA_MCST_ELBRUS_E2K:
        case LIBELF_ISA_ARM_64:
        case LIBELF_ISA_ZILOG_Z80:
        case LIBELF_ISA_RISC_V:
        case LIBELF_ISA_BPF:
        case LIBELF_ISA_WDC_65C816:
        case LIBELF_ISA_LOONGARCH:
        default:
                return LIBELF_INVALID_ELF;
        }

        if (hdr->version != LIBELF_EV_ORIGINAL)
                return LIBELF_INVALID_ELF;

        return LIBELF_OK;
}

static enum libelf_error map_elf(const char *addr, struct libelf_state *state)
{
        auto ehdr = (const struct elf32_ehdr *)addr;
        auto phdr = (const struct elf32_phdr *)(addr + ehdr->phoff);

        state->address_space = mm_create_new_mm();

        for (size_t i = 0; i < ehdr->phnum; ++i) {
                auto ph = phdr[i];
                if (ph.type != LIBELF_PT_LOAD)
                        continue;
                uintptr_t vaddr_start = ph.vaddr & ~0xFFF;
                uintptr_t vaddr_end   = (ph.vaddr + ph.memsz + 0xFFF) & ~0xFFF;

                for (uintptr_t vaddr = vaddr_start; vaddr < vaddr_end;
                     vaddr += 0x1000) {
                        uintptr_t paddr = mm_get_free_page();

                        char *vpaddr = mm_alloc_addr((void *)paddr,
                                                     MM_GP_KERNEL | MM_GP_RW);
                        memset(vpaddr, 0, 0x1000);

                        uintptr_t copy_start = max(vaddr, ph.vaddr);
                        uintptr_t copy_end =
                            min(vaddr + 0x1000, ph.vaddr + ph.filesz);
                        if (copy_start < copy_end) {
                                size_t copy_size   = copy_end - copy_start;
                                size_t page_offset = copy_start - vaddr;
                                size_t file_offset =
                                    ph.offset + (copy_start - ph.vaddr);

                                memcpy(vpaddr + page_offset,
                                       addr + file_offset,
                                       copy_size);
                        }
                        mm_free_page_active(vpaddr, 1);
                        mm_map_page(state->address_space,
                                    vaddr,
                                    paddr,
                                    MM_GP_RW | MM_GP_USER);
                }
        }

        state->entry = ehdr->entry;

        return LIBELF_OK;
}

enum libelf_error libelf_load_elf(void *file, struct libelf_state *state)
{
        auto hdr = (struct elf32_ehdr *)file;
        if (validate_header(hdr) != LIBELF_OK)
                return LIBELF_INVALID_ELF;

        switch (hdr->type) {
        case LIBELF_ET_EXEC:
                state->entry = hdr->entry;
                break;
        default:
                return LIBELF_INVALID_ELF;
        }

        return map_elf((char *)hdr, state);
}
