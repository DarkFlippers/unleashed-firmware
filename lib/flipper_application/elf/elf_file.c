#include <elf.h>
#include "elf_file.h"
#include "elf_file_i.h"
#include "elf_api_interface.h"

#define TAG "elf"

#define ELF_NAME_BUFFER_LEN 32
#define SECTION_OFFSET(e, n) (e->section_table + n * sizeof(Elf32_Shdr))
#define IS_FLAGS_SET(v, m) ((v & m) == m)
#define RESOLVER_THREAD_YIELD_STEP 30

// #define ELF_DEBUG_LOG 1

#ifndef ELF_DEBUG_LOG
#undef FURI_LOG_D
#define FURI_LOG_D(...)
#endif

#define TRAMPOLINE_CODE_SIZE 6

/**
ldr r12, [pc, #2]
bx r12
*/
const uint8_t trampoline_code_little_endian[TRAMPOLINE_CODE_SIZE] =
    {0xdf, 0xf8, 0x02, 0xc0, 0x60, 0x47};

typedef struct {
    uint8_t code[TRAMPOLINE_CODE_SIZE];
    uint32_t addr;
} __attribute__((packed)) JMPTrampoline;

/**************************************************************************************************/
/********************************************* Caches *********************************************/
/**************************************************************************************************/

static bool address_cache_get(AddressCache_t cache, int symEntry, Elf32_Addr* symAddr) {
    Elf32_Addr* addr = AddressCache_get(cache, symEntry);
    if(addr) {
        *symAddr = *addr;
        return true;
    } else {
        return false;
    }
}

static void address_cache_put(AddressCache_t cache, int symEntry, Elf32_Addr symAddr) {
    AddressCache_set_at(cache, symEntry, symAddr);
}

/**************************************************************************************************/
/********************************************** ELF ***********************************************/
/**************************************************************************************************/

static ELFSection* elf_file_get_section(ELFFile* elf, const char* name) {
    return ELFSectionDict_get(elf->sections, name);
}

static void elf_file_put_section(ELFFile* elf, const char* name, ELFSection* section) {
    ELFSectionDict_set_at(elf->sections, strdup(name), *section);
}

static bool elf_read_string_from_offset(ELFFile* elf, off_t offset, string_t name) {
    bool result = false;

    off_t old = storage_file_tell(elf->fd);

    do {
        if(!storage_file_seek(elf->fd, offset, true)) break;

        char buffer[ELF_NAME_BUFFER_LEN + 1];
        buffer[ELF_NAME_BUFFER_LEN] = 0;

        while(true) {
            uint16_t read = storage_file_read(elf->fd, buffer, ELF_NAME_BUFFER_LEN);
            string_cat_str(name, buffer);
            if(strlen(buffer) < ELF_NAME_BUFFER_LEN) {
                result = true;
                break;
            }

            if(storage_file_get_error(elf->fd) != FSE_OK || read == 0) break;
        }

    } while(false);
    storage_file_seek(elf->fd, old, true);

    return result;
}

static bool elf_read_section_name(ELFFile* elf, off_t offset, string_t name) {
    return elf_read_string_from_offset(elf, elf->section_table_strings + offset, name);
}

static bool elf_read_symbol_name(ELFFile* elf, off_t offset, string_t name) {
    return elf_read_string_from_offset(elf, elf->symbol_table_strings + offset, name);
}

static bool elf_read_section_header(ELFFile* elf, size_t section_idx, Elf32_Shdr* section_header) {
    off_t offset = SECTION_OFFSET(elf, section_idx);
    return storage_file_seek(elf->fd, offset, true) &&
           storage_file_read(elf->fd, section_header, sizeof(Elf32_Shdr)) == sizeof(Elf32_Shdr);
}

static bool
    elf_read_section(ELFFile* elf, size_t section_idx, Elf32_Shdr* section_header, string_t name) {
    if(!elf_read_section_header(elf, section_idx, section_header)) {
        return false;
    }

    if(section_header->sh_name && !elf_read_section_name(elf, section_header->sh_name, name)) {
        return false;
    }

    return true;
}

static bool elf_read_symbol(ELFFile* elf, int n, Elf32_Sym* sym, string_t name) {
    bool success = false;
    off_t old = storage_file_tell(elf->fd);
    off_t pos = elf->symbol_table + n * sizeof(Elf32_Sym);
    if(storage_file_seek(elf->fd, pos, true) &&
       storage_file_read(elf->fd, sym, sizeof(Elf32_Sym)) == sizeof(Elf32_Sym)) {
        if(sym->st_name)
            success = elf_read_symbol_name(elf, sym->st_name, name);
        else {
            Elf32_Shdr shdr;
            success = elf_read_section(elf, sym->st_shndx, &shdr, name);
        }
    }
    storage_file_seek(elf->fd, old, true);
    return success;
}

static ELFSection* elf_section_of(ELFFile* elf, int index) {
    ELFSectionDict_it_t it;
    for(ELFSectionDict_it(it, elf->sections); !ELFSectionDict_end_p(it); ELFSectionDict_next(it)) {
        ELFSectionDict_itref_t* itref = ELFSectionDict_ref(it);
        if(itref->value.sec_idx == index) {
            return &itref->value;
        }
    }

    return NULL;
}

static Elf32_Addr elf_address_of(ELFFile* elf, Elf32_Sym* sym, const char* sName) {
    if(sym->st_shndx == SHN_UNDEF) {
        Elf32_Addr addr = 0;
        if(elf->api_interface->resolver_callback(sName, &addr)) {
            return addr;
        }
    } else {
        ELFSection* symSec = elf_section_of(elf, sym->st_shndx);
        if(symSec) {
            return ((Elf32_Addr)symSec->data) + sym->st_value;
        }
    }
    FURI_LOG_D(TAG, "  Can not find address for symbol %s", sName);
    return ELF_INVALID_ADDRESS;
}

__attribute__((unused)) static const char* elf_reloc_type_to_str(int symt) {
#define STRCASE(name) \
    case name:        \
        return #name;
    switch(symt) {
        STRCASE(R_ARM_NONE)
        STRCASE(R_ARM_TARGET1)
        STRCASE(R_ARM_ABS32)
        STRCASE(R_ARM_THM_PC22)
        STRCASE(R_ARM_THM_JUMP24)
    default:
        return "R_<unknow>";
    }
#undef STRCASE
}

static JMPTrampoline* elf_create_trampoline(Elf32_Addr addr) {
    JMPTrampoline* trampoline = malloc(sizeof(JMPTrampoline));
    memcpy(trampoline->code, trampoline_code_little_endian, TRAMPOLINE_CODE_SIZE);
    trampoline->addr = addr;
    return trampoline;
}

static void elf_relocate_jmp_call(ELFFile* elf, Elf32_Addr relAddr, int type, Elf32_Addr symAddr) {
    int offset, hi, lo, s, j1, j2, i1, i2, imm10, imm11;
    int to_thumb, is_call, blx_bit = 1 << 12;

    /* Get initial offset */
    hi = ((uint16_t*)relAddr)[0];
    lo = ((uint16_t*)relAddr)[1];
    s = (hi >> 10) & 1;
    j1 = (lo >> 13) & 1;
    j2 = (lo >> 11) & 1;
    i1 = (j1 ^ s) ^ 1;
    i2 = (j2 ^ s) ^ 1;
    imm10 = hi & 0x3ff;
    imm11 = lo & 0x7ff;
    offset = (s << 24) | (i1 << 23) | (i2 << 22) | (imm10 << 12) | (imm11 << 1);
    if(offset & 0x01000000) offset -= 0x02000000;

    to_thumb = symAddr & 1;
    is_call = (type == R_ARM_THM_PC22);

    /* Store offset */
    int offset_copy = offset;

    /* Compute final offset */
    offset += symAddr - relAddr;
    if(!to_thumb && is_call) {
        blx_bit = 0; /* bl -> blx */
        offset = (offset + 3) & -4; /* Compute offset from aligned PC */
    }

    /* Check that relocation is possible
    * offset must not be out of range
    * if target is to be entered in arm mode:
        - bit 1 must not set
        - instruction must be a call (bl) or a jump to PLT */
    if(!to_thumb || offset >= 0x1000000 || offset < -0x1000000) {
        if(to_thumb || (symAddr & 2) || (!is_call)) {
            FURI_LOG_D(
                TAG,
                "can't relocate value at %x, %s, doing trampoline",
                relAddr,
                elf_reloc_type_to_str(type));

            Elf32_Addr addr;
            if(!address_cache_get(elf->trampoline_cache, symAddr, &addr)) {
                addr = (Elf32_Addr)elf_create_trampoline(symAddr);
                address_cache_put(elf->trampoline_cache, symAddr, addr);
            }

            offset = offset_copy;
            offset += (int)addr - relAddr;
            if(!to_thumb && is_call) {
                blx_bit = 0; /* bl -> blx */
                offset = (offset + 3) & -4; /* Compute offset from aligned PC */
            }
        }
    }

    /* Compute and store final offset */
    s = (offset >> 24) & 1;
    i1 = (offset >> 23) & 1;
    i2 = (offset >> 22) & 1;
    j1 = s ^ (i1 ^ 1);
    j2 = s ^ (i2 ^ 1);
    imm10 = (offset >> 12) & 0x3ff;
    imm11 = (offset >> 1) & 0x7ff;
    (*(uint16_t*)relAddr) = (uint16_t)((hi & 0xf800) | (s << 10) | imm10);
    (*(uint16_t*)(relAddr + 2)) =
        (uint16_t)((lo & 0xc000) | (j1 << 13) | blx_bit | (j2 << 11) | imm11);
}

static void elf_relocate_mov(Elf32_Addr relAddr, int type, Elf32_Addr symAddr) {
    uint16_t upper_insn = ((uint16_t*)relAddr)[0];
    uint16_t lower_insn = ((uint16_t*)relAddr)[1];

    /* MOV*<C> <Rd>,#<imm16>
     *
     * i = upper[10]
     * imm4 = upper[3:0]
     * imm3 = lower[14:12]
     * imm8 = lower[7:0]
     *
     * imm16 = imm4:i:imm3:imm8
     */
    uint32_t i = (upper_insn >> 10) & 1; /* upper[10] */
    uint32_t imm4 = upper_insn & 0x000F; /* upper[3:0] */
    uint32_t imm3 = (lower_insn >> 12) & 0x7; /* lower[14:12] */
    uint32_t imm8 = lower_insn & 0x00FF; /* lower[7:0] */

    int32_t addend = (imm4 << 12) | (i << 11) | (imm3 << 8) | imm8; /* imm16 */

    uint32_t addr = (symAddr + addend);
    if (type == R_ARM_THM_MOVT_ABS) {
        addr >>= 16; /* upper 16 bits */
    } else {
        addr &= 0x0000FFFF; /* lower 16 bits */
    }

    /* Re-encode */
    ((uint16_t*)relAddr)[0] = (upper_insn & 0xFBF0)
                            | (((addr >> 11) & 1) << 10) /* i */
                            | ((addr >> 12) & 0x000F); /* imm4 */
    ((uint16_t*)relAddr)[1] = (lower_insn & 0x8F00)
                            | (((addr >> 8) & 0x7) << 12) /* imm3 */
                            | (addr & 0x00FF); /* imm8 */
}

static bool elf_relocate_symbol(ELFFile* elf, Elf32_Addr relAddr, int type, Elf32_Addr symAddr) {
    switch(type) {
    case R_ARM_TARGET1:
    case R_ARM_ABS32:
        *((uint32_t*)relAddr) += symAddr;
        FURI_LOG_D(TAG, "  R_ARM_ABS32 relocated is 0x%08X", (unsigned int)*((uint32_t*)relAddr));
        break;
    case R_ARM_THM_PC22:
    case R_ARM_THM_JUMP24:
        elf_relocate_jmp_call(elf, relAddr, type, symAddr);
        FURI_LOG_D(
            TAG, "  R_ARM_THM_CALL/JMP relocated is 0x%08X", (unsigned int)*((uint32_t*)relAddr));
        break;
    case R_ARM_THM_MOVW_ABS_NC:
    case R_ARM_THM_MOVT_ABS:
        elf_relocate_mov(relAddr, type, symAddr);
        FURI_LOG_D(TAG, "  R_ARM_THM_MOVW_ABS_NC/MOVT_ABS relocated is 0x%08X", (unsigned int)*((uint32_t*)relAddr));
        break;
    default:
        FURI_LOG_E(TAG, "  Undefined relocation %d", type);
        return false;
    }
    return true;
}

static bool elf_relocate(ELFFile* elf, Elf32_Shdr* h, ELFSection* s) {
    if(s->data) {
        Elf32_Rel rel;
        size_t relEntries = h->sh_size / sizeof(rel);
        size_t relCount;
        (void)storage_file_seek(elf->fd, h->sh_offset, true);
        FURI_LOG_D(TAG, " Offset   Info     Type             Name");

        int relocate_result = true;
        string_t symbol_name;
        string_init(symbol_name);

        for(relCount = 0; relCount < relEntries; relCount++) {
            if(relCount % RESOLVER_THREAD_YIELD_STEP == 0) {
                FURI_LOG_D(TAG, "  reloc YIELD");
                furi_delay_tick(1);
            }

            if(storage_file_read(elf->fd, &rel, sizeof(Elf32_Rel)) != sizeof(Elf32_Rel)) {
                FURI_LOG_E(TAG, "  reloc read fail");
                string_clear(symbol_name);
                return false;
            }

            Elf32_Addr symAddr;

            int symEntry = ELF32_R_SYM(rel.r_info);
            int relType = ELF32_R_TYPE(rel.r_info);
            Elf32_Addr relAddr = ((Elf32_Addr)s->data) + rel.r_offset;

            if(!address_cache_get(elf->relocation_cache, symEntry, &symAddr)) {
                Elf32_Sym sym;
                string_reset(symbol_name);
                if(!elf_read_symbol(elf, symEntry, &sym, symbol_name)) {
                    FURI_LOG_E(TAG, "  symbol read fail");
                    string_clear(symbol_name);
                    return false;
                }

                FURI_LOG_D(
                    TAG,
                    " %08X %08X %-16s %s",
                    (unsigned int)rel.r_offset,
                    (unsigned int)rel.r_info,
                    elf_reloc_type_to_str(relType),
                    string_get_cstr(symbol_name));

                symAddr = elf_address_of(elf, &sym, string_get_cstr(symbol_name));
                address_cache_put(elf->relocation_cache, symEntry, symAddr);
            }

            if(symAddr != ELF_INVALID_ADDRESS) {
                FURI_LOG_D(
                    TAG,
                    "  symAddr=%08X relAddr=%08X",
                    (unsigned int)symAddr,
                    (unsigned int)relAddr);
                if(!elf_relocate_symbol(elf, relAddr, relType, symAddr)) {
                    relocate_result = false;
                }
            } else {
                FURI_LOG_E(TAG, "  No symbol address of %s", string_get_cstr(symbol_name));
                relocate_result = false;
            }
        }
        string_clear(symbol_name);

        return relocate_result;
    } else {
        FURI_LOG_D(TAG, "Section not loaded");
    }

    return false;
}

/**************************************************************************************************/
/********************************************* MISC ***********************************************/
/**************************************************************************************************/

static bool cstr_prefix(const char* prefix, const char* string) {
    return strncmp(prefix, string, strlen(prefix)) == 0;
}

/**************************************************************************************************/
/************************************ Internal FAP interfaces *************************************/
/**************************************************************************************************/
typedef enum {
    SectionTypeERROR = 0,
    SectionTypeUnused = 1 << 0,
    SectionTypeData = 1 << 1,
    SectionTypeRelData = 1 << 2,
    SectionTypeSymTab = 1 << 3,
    SectionTypeStrTab = 1 << 4,
    SectionTypeManifest = 1 << 5,
    SectionTypeDebugLink = 1 << 6,

    SectionTypeValid = SectionTypeSymTab | SectionTypeStrTab | SectionTypeManifest,
} SectionType;

static bool elf_load_metadata(
    ELFFile* elf,
    Elf32_Shdr* section_header,
    FlipperApplicationManifest* manifest) {
    if(section_header->sh_size < sizeof(FlipperApplicationManifest)) {
        return false;
    }

    if(manifest == NULL) {
        return true;
    }

    return storage_file_seek(elf->fd, section_header->sh_offset, true) &&
           storage_file_read(elf->fd, manifest, section_header->sh_size) ==
               section_header->sh_size;
}

static bool elf_load_debug_link(ELFFile* elf, Elf32_Shdr* section_header) {
    elf->debug_link_info.debug_link_size = section_header->sh_size;
    elf->debug_link_info.debug_link = malloc(section_header->sh_size);

    return storage_file_seek(elf->fd, section_header->sh_offset, true) &&
           storage_file_read(elf->fd, elf->debug_link_info.debug_link, section_header->sh_size) ==
               section_header->sh_size;
}

static SectionType elf_preload_section(
    ELFFile* elf,
    size_t section_idx,
    Elf32_Shdr* section_header,
    string_t name_string,
    FlipperApplicationManifest* manifest) {
    const char* name = string_get_cstr(name_string);

    const struct {
        const char* prefix;
        SectionType type;
    } lookup_sections[] = {
        {".text", SectionTypeData},
        {".rodata", SectionTypeData},
        {".data", SectionTypeData},
        {".bss", SectionTypeData},
        {".preinit_array", SectionTypeData},
        {".init_array", SectionTypeData},
        {".fini_array", SectionTypeData},
        {".rel.text", SectionTypeRelData},
        {".rel.rodata", SectionTypeRelData},
        {".rel.data", SectionTypeRelData},
        {".rel.preinit_array", SectionTypeRelData},
        {".rel.init_array", SectionTypeRelData},
        {".rel.fini_array", SectionTypeRelData},
    };

    for(size_t i = 0; i < COUNT_OF(lookup_sections); i++) {
        if(cstr_prefix(lookup_sections[i].prefix, name)) {
            FURI_LOG_D(TAG, "Found section %s", lookup_sections[i].prefix);

            if(lookup_sections[i].type == SectionTypeRelData) {
                name = name + strlen(".rel");
            }

            ELFSection* section_p = elf_file_get_section(elf, name);
            if(!section_p) {
                ELFSection section = {
                    .data = NULL,
                    .sec_idx = 0,
                    .rel_sec_idx = 0,
                    .size = 0,
                };

                elf_file_put_section(elf, name, &section);
                section_p = elf_file_get_section(elf, name);
            }

            if(lookup_sections[i].type == SectionTypeRelData) {
                section_p->rel_sec_idx = section_idx;
            } else {
                section_p->sec_idx = section_idx;
            }

            return lookup_sections[i].type;
        }
    }

    if(strcmp(name, ".symtab") == 0) {
        FURI_LOG_D(TAG, "Found .symtab section");
        elf->symbol_table = section_header->sh_offset;
        elf->symbol_count = section_header->sh_size / sizeof(Elf32_Sym);
        return SectionTypeSymTab;
    } else if(strcmp(name, ".strtab") == 0) {
        FURI_LOG_D(TAG, "Found .strtab section");
        elf->symbol_table_strings = section_header->sh_offset;
        return SectionTypeStrTab;
    } else if(strcmp(name, ".fapmeta") == 0) {
        FURI_LOG_D(TAG, "Found .fapmeta section");
        if(elf_load_metadata(elf, section_header, manifest)) {
            return SectionTypeManifest;
        } else {
            return SectionTypeERROR;
        }
    } else if(strcmp(name, ".gnu_debuglink") == 0) {
        FURI_LOG_D(TAG, "Found .gnu_debuglink section");
        if(elf_load_debug_link(elf, section_header)) {
            return SectionTypeDebugLink;
        } else {
            return SectionTypeERROR;
        }
    }

    return SectionTypeUnused;
}

static bool elf_load_section_data(ELFFile* elf, ELFSection* section) {
    Elf32_Shdr section_header;
    if(section->sec_idx == 0) {
        FURI_LOG_D(TAG, "Section is not present");
        return true;
    }

    if(!elf_read_section_header(elf, section->sec_idx, &section_header)) {
        return false;
    }

    if(section_header.sh_size == 0) {
        FURI_LOG_D(TAG, "No data for section");
        return true;
    }

    section->data = aligned_malloc(section_header.sh_size, section_header.sh_addralign);
    section->size = section_header.sh_size;

    if(section_header.sh_type == SHT_NOBITS) {
        /* section is empty (.bss?) */
        /* no need to memset - allocator already did that */
        return true;
    }

    if((!storage_file_seek(elf->fd, section_header.sh_offset, true)) ||
       (storage_file_read(elf->fd, section->data, section_header.sh_size) !=
        section_header.sh_size)) {
        FURI_LOG_E(TAG, "    seek/read fail");
        return false;
    }

    FURI_LOG_D(TAG, "0x%X", section->data);
    return true;
}

static bool elf_relocate_section(ELFFile* elf, ELFSection* section) {
    Elf32_Shdr section_header;
    if(section->rel_sec_idx) {
        FURI_LOG_D(TAG, "Relocating section");
        if(elf_read_section_header(elf, section->rel_sec_idx, &section_header))
            return elf_relocate(elf, &section_header, section);
        else {
            FURI_LOG_E(TAG, "Error reading section header");
            return false;
        }
    } else {
        FURI_LOG_D(TAG, "No relocation index"); /* Not an error */
    }
    return true;
}

static void elf_file_call_section_list(ELFFile* elf, const char* name, bool reverse_order) {
    ELFSection* section = elf_file_get_section(elf, name);

    if(section && section->size) {
        const uint32_t* start = section->data;
        const uint32_t* end = section->data + section->size;

        if(reverse_order) {
            while(end > start) {
                end--;
                ((void (*)(void))(*end))();
            }
        } else {
            while(start < end) {
                ((void (*)(void))(*start))();
                start++;
            }
        }
    }
}

/**************************************************************************************************/
/********************************************* Public *********************************************/
/**************************************************************************************************/

ELFFile* elf_file_alloc(Storage* storage, const ElfApiInterface* api_interface) {
    ELFFile* elf = malloc(sizeof(ELFFile));
    elf->fd = storage_file_alloc(storage);
    elf->api_interface = api_interface;
    ELFSectionDict_init(elf->sections);
    AddressCache_init(elf->trampoline_cache);
    return elf;
}

void elf_file_free(ELFFile* elf) {
    // free sections data
    {
        ELFSectionDict_it_t it;
        for(ELFSectionDict_it(it, elf->sections); !ELFSectionDict_end_p(it);
            ELFSectionDict_next(it)) {
            const ELFSectionDict_itref_t* itref = ELFSectionDict_cref(it);
            if(itref->value.data) {
                aligned_free(itref->value.data);
            }
            free((void*)itref->key);
        }

        ELFSectionDict_clear(elf->sections);
    }

    // free trampoline data
    {
        AddressCache_it_t it;
        for(AddressCache_it(it, elf->trampoline_cache); !AddressCache_end_p(it);
            AddressCache_next(it)) {
            const AddressCache_itref_t* itref = AddressCache_cref(it);
            free((void*)itref->value);
        }

        AddressCache_clear(elf->trampoline_cache);
    }

    if(elf->debug_link_info.debug_link) {
        free(elf->debug_link_info.debug_link);
    }

    storage_file_free(elf->fd);
    free(elf);
}

bool elf_file_open(ELFFile* elf, const char* path) {
    Elf32_Ehdr h;
    Elf32_Shdr sH;

    if(!storage_file_open(elf->fd, path, FSAM_READ, FSOM_OPEN_EXISTING) ||
       !storage_file_seek(elf->fd, 0, true) ||
       storage_file_read(elf->fd, &h, sizeof(h)) != sizeof(h) ||
       !storage_file_seek(elf->fd, h.e_shoff + h.e_shstrndx * sizeof(sH), true) ||
       storage_file_read(elf->fd, &sH, sizeof(Elf32_Shdr)) != sizeof(Elf32_Shdr)) {
        return false;
    }

    elf->entry = h.e_entry;
    elf->sections_count = h.e_shnum;
    elf->section_table = h.e_shoff;
    elf->section_table_strings = sH.sh_offset;
    return true;
}

bool elf_file_load_manifest(ELFFile* elf, FlipperApplicationManifest* manifest) {
    bool result = false;
    string_t name;
    string_init(name);

    FURI_LOG_D(TAG, "Looking for manifest section");
    for(size_t section_idx = 1; section_idx < elf->sections_count; section_idx++) {
        Elf32_Shdr section_header;

        string_reset(name);
        if(!elf_read_section(elf, section_idx, &section_header, name)) {
            break;
        }

        if(string_cmp(name, ".fapmeta") == 0) {
            if(elf_load_metadata(elf, &section_header, manifest)) {
                FURI_LOG_D(TAG, "Load manifest done");
                result = true;
                break;
            } else {
                break;
            }
        }
    }

    string_clear(name);
    return result;
}

bool elf_file_load_section_table(ELFFile* elf, FlipperApplicationManifest* manifest) {
    SectionType loaded_sections = SectionTypeERROR;
    string_t name;
    string_init(name);

    FURI_LOG_D(TAG, "Scan ELF indexs...");
    for(size_t section_idx = 1; section_idx < elf->sections_count; section_idx++) {
        Elf32_Shdr section_header;

        string_reset(name);
        if(!elf_read_section(elf, section_idx, &section_header, name)) {
            loaded_sections = SectionTypeERROR;
            break;
        }

        FURI_LOG_D(TAG, "Preloading data for section #%d %s", section_idx, string_get_cstr(name));
        SectionType section_type =
            elf_preload_section(elf, section_idx, &section_header, name, manifest);
        loaded_sections |= section_type;

        if(section_type == SectionTypeERROR) {
            loaded_sections = SectionTypeERROR;
            break;
        }
    }

    string_clear(name);
    FURI_LOG_D(TAG, "Load symbols done");

    return IS_FLAGS_SET(loaded_sections, SectionTypeValid);
}

ELFFileLoadStatus elf_file_load_sections(ELFFile* elf) {
    ELFFileLoadStatus status = ELFFileLoadStatusSuccess;
    ELFSectionDict_it_t it;

    AddressCache_init(elf->relocation_cache);
    size_t start = furi_get_tick();

    for(ELFSectionDict_it(it, elf->sections); !ELFSectionDict_end_p(it); ELFSectionDict_next(it)) {
        ELFSectionDict_itref_t* itref = ELFSectionDict_ref(it);
        FURI_LOG_D(TAG, "Loading section '%s'", itref->key);
        if(!elf_load_section_data(elf, &itref->value)) {
            FURI_LOG_E(TAG, "Error loading section '%s'", itref->key);
            status = ELFFileLoadStatusUnspecifiedError;
        }
    }

    if(status == ELFFileLoadStatusSuccess) {
        for(ELFSectionDict_it(it, elf->sections); !ELFSectionDict_end_p(it);
            ELFSectionDict_next(it)) {
            ELFSectionDict_itref_t* itref = ELFSectionDict_ref(it);
            FURI_LOG_D(TAG, "Relocating section '%s'", itref->key);
            if(!elf_relocate_section(elf, &itref->value)) {
                FURI_LOG_E(TAG, "Error relocating section '%s'", itref->key);
                status = ELFFileLoadStatusMissingImports;
            }
        }
    }

    /* Fixing up entry point */
    if(status == ELFFileLoadStatusSuccess) {
        ELFSection* text_section = elf_file_get_section(elf, ".text");

        if(text_section == NULL) {
            FURI_LOG_E(TAG, "No .text section found");
            status = ELFFileLoadStatusUnspecifiedError;
        } else {
            elf->entry += (uint32_t)text_section->data;
        }
    }

    FURI_LOG_D(TAG, "Relocation cache size: %u", AddressCache_size(elf->relocation_cache));
    FURI_LOG_D(TAG, "Trampoline cache size: %u", AddressCache_size(elf->trampoline_cache));
    AddressCache_clear(elf->relocation_cache);
    FURI_LOG_I(TAG, "Loaded in %ums", (size_t)(furi_get_tick() - start));

    return status;
}

void elf_file_pre_run(ELFFile* elf) {
    elf_file_call_section_list(elf, ".preinit_array", false);
    elf_file_call_section_list(elf, ".init_array", false);
}

int32_t elf_file_run(ELFFile* elf, void* args) {
    int32_t result;
    result = ((int32_t(*)(void*))elf->entry)(args);
    return result;
}

void elf_file_post_run(ELFFile* elf) {
    elf_file_call_section_list(elf, ".fini_array", true);
}

const ElfApiInterface* elf_file_get_api_interface(ELFFile* elf_file) {
    return elf_file->api_interface;
}

void elf_file_init_debug_info(ELFFile* elf, ELFDebugInfo* debug_info) {
    // set entry
    debug_info->entry = elf->entry;

    // copy debug info
    memcpy(&debug_info->debug_link_info, &elf->debug_link_info, sizeof(ELFDebugLinkInfo));

    // init mmap
    debug_info->mmap_entry_count = ELFSectionDict_size(elf->sections);
    debug_info->mmap_entries = malloc(sizeof(ELFMemoryMapEntry) * debug_info->mmap_entry_count);
    uint32_t mmap_entry_idx = 0;

    ELFSectionDict_it_t it;
    for(ELFSectionDict_it(it, elf->sections); !ELFSectionDict_end_p(it); ELFSectionDict_next(it)) {
        const ELFSectionDict_itref_t* itref = ELFSectionDict_cref(it);

        const void* data_ptr = itref->value.data;
        if(data_ptr) {
            debug_info->mmap_entries[mmap_entry_idx].address = (uint32_t)data_ptr;
            debug_info->mmap_entries[mmap_entry_idx].name = itref->key;
            mmap_entry_idx++;
        }
    }
}

void elf_file_clear_debug_info(ELFDebugInfo* debug_info) {
    // clear debug info
    memset(&debug_info->debug_link_info, 0, sizeof(ELFDebugLinkInfo));

    // clear mmap
    if(debug_info->mmap_entries) {
        free(debug_info->mmap_entries);
        debug_info->mmap_entries = NULL;
    }

    debug_info->mmap_entry_count = 0;
}
