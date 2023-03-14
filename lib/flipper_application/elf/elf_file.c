#include <elf.h>
#include "elf_file.h"
#include "elf_file_i.h"
#include "elf_api_interface.h"

#define TAG "elf"

#define ELF_NAME_BUFFER_LEN 32
#define SECTION_OFFSET(e, n) ((e)->section_table + (n) * sizeof(Elf32_Shdr))
#define IS_FLAGS_SET(v, m) (((v) & (m)) == (m))
#define RESOLVER_THREAD_YIELD_STEP 30

// #define ELF_DEBUG_LOG 1

#ifndef ELF_DEBUG_LOG
#undef FURI_LOG_D
#define FURI_LOG_D(...)
#endif

#define ELF_INVALID_ADDRESS 0xFFFFFFFF

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

static ELFSection* elf_file_get_or_put_section(ELFFile* elf, const char* name) {
    ELFSection* section_p = elf_file_get_section(elf, name);
    if(!section_p) {
        ELFSectionDict_set_at(
            elf->sections,
            strdup(name),
            (ELFSection){
                .data = NULL,
                .sec_idx = 0,
                .size = 0,
                .rel_count = 0,
                .rel_offset = 0,
            });
        section_p = elf_file_get_section(elf, name);
    }

    return section_p;
}

static bool elf_read_string_from_offset(ELFFile* elf, off_t offset, FuriString* name) {
    bool result = false;

    off_t old = storage_file_tell(elf->fd);

    do {
        if(!storage_file_seek(elf->fd, offset, true)) break;

        char buffer[ELF_NAME_BUFFER_LEN + 1];
        buffer[ELF_NAME_BUFFER_LEN] = 0;

        while(true) {
            uint16_t read = storage_file_read(elf->fd, buffer, ELF_NAME_BUFFER_LEN);
            furi_string_cat(name, buffer);
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

static bool elf_read_section_name(ELFFile* elf, off_t offset, FuriString* name) {
    return elf_read_string_from_offset(elf, elf->section_table_strings + offset, name);
}

static bool elf_read_symbol_name(ELFFile* elf, off_t offset, FuriString* name) {
    return elf_read_string_from_offset(elf, elf->symbol_table_strings + offset, name);
}

static bool elf_read_section_header(ELFFile* elf, size_t section_idx, Elf32_Shdr* section_header) {
    off_t offset = SECTION_OFFSET(elf, section_idx);
    return storage_file_seek(elf->fd, offset, true) &&
           storage_file_read(elf->fd, section_header, sizeof(Elf32_Shdr)) == sizeof(Elf32_Shdr);
}

static bool elf_read_section(
    ELFFile* elf,
    size_t section_idx,
    Elf32_Shdr* section_header,
    FuriString* name) {
    if(!elf_read_section_header(elf, section_idx, section_header)) {
        return false;
    }

    if(section_header->sh_name && !elf_read_section_name(elf, section_header->sh_name, name)) {
        return false;
    }

    return true;
}

static bool elf_read_symbol(ELFFile* elf, int n, Elf32_Sym* sym, FuriString* name) {
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
        if(elf->api_interface->resolver_callback(elf->api_interface, sName, &addr)) {
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
                "can't relocate value at %lx, %s, doing trampoline",
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
    if(type == R_ARM_THM_MOVT_ABS) {
        addr >>= 16; /* upper 16 bits */
    } else {
        addr &= 0x0000FFFF; /* lower 16 bits */
    }

    /* Re-encode */
    ((uint16_t*)relAddr)[0] = (upper_insn & 0xFBF0) | (((addr >> 11) & 1) << 10) /* i */
                              | ((addr >> 12) & 0x000F); /* imm4 */
    ((uint16_t*)relAddr)[1] = (lower_insn & 0x8F00) | (((addr >> 8) & 0x7) << 12) /* imm3 */
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
    case R_ARM_CALL:
    case R_ARM_THM_JUMP24:
        elf_relocate_jmp_call(elf, relAddr, type, symAddr);
        FURI_LOG_D(
            TAG, "  R_ARM_THM_CALL/JMP relocated is 0x%08X", (unsigned int)*((uint32_t*)relAddr));
        break;
    case R_ARM_THM_MOVW_ABS_NC:
    case R_ARM_THM_MOVT_ABS:
        elf_relocate_mov(relAddr, type, symAddr);
        FURI_LOG_D(
            TAG,
            "  R_ARM_THM_MOVW_ABS_NC/MOVT_ABS relocated is 0x%08X",
            (unsigned int)*((uint32_t*)relAddr));
        break;
    default:
        FURI_LOG_E(TAG, "  Undefined relocation %d", type);
        return false;
    }
    return true;
}

static bool elf_relocate(ELFFile* elf, ELFSection* s) {
    if(s->data) {
        Elf32_Rel rel;
        size_t relEntries = s->rel_count;
        size_t relCount;
        (void)storage_file_seek(elf->fd, s->rel_offset, true);
        FURI_LOG_D(TAG, " Offset   Info     Type             Name");

        int relocate_result = true;
        FuriString* symbol_name;
        symbol_name = furi_string_alloc();

        for(relCount = 0; relCount < relEntries; relCount++) {
            if(relCount % RESOLVER_THREAD_YIELD_STEP == 0) {
                FURI_LOG_D(TAG, "  reloc YIELD");
                furi_delay_tick(1);
            }

            if(storage_file_read(elf->fd, &rel, sizeof(Elf32_Rel)) != sizeof(Elf32_Rel)) {
                FURI_LOG_E(TAG, "  reloc read fail");
                furi_string_free(symbol_name);
                return false;
            }

            Elf32_Addr symAddr;

            int symEntry = ELF32_R_SYM(rel.r_info);
            int relType = ELF32_R_TYPE(rel.r_info);
            Elf32_Addr relAddr = ((Elf32_Addr)s->data) + rel.r_offset;

            if(!address_cache_get(elf->relocation_cache, symEntry, &symAddr)) {
                Elf32_Sym sym;
                furi_string_reset(symbol_name);
                if(!elf_read_symbol(elf, symEntry, &sym, symbol_name)) {
                    FURI_LOG_E(TAG, "  symbol read fail");
                    furi_string_free(symbol_name);
                    return false;
                }

                FURI_LOG_D(
                    TAG,
                    " %08X %08X %-16s %s",
                    (unsigned int)rel.r_offset,
                    (unsigned int)rel.r_info,
                    elf_reloc_type_to_str(relType),
                    furi_string_get_cstr(symbol_name));

                symAddr = elf_address_of(elf, &sym, furi_string_get_cstr(symbol_name));
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
                FURI_LOG_E(TAG, "  No symbol address of %s", furi_string_get_cstr(symbol_name));
                relocate_result = false;
            }
        }
        furi_string_free(symbol_name);

        return relocate_result;
    } else {
        FURI_LOG_D(TAG, "Section not loaded");
    }

    return false;
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
    SectionTypeDebugLink = 1 << 5,

    SectionTypeValid = SectionTypeSymTab | SectionTypeStrTab,
} SectionType;

static bool elf_load_debug_link(ELFFile* elf, Elf32_Shdr* section_header) {
    elf->debug_link_info.debug_link_size = section_header->sh_size;
    elf->debug_link_info.debug_link = malloc(section_header->sh_size);

    return storage_file_seek(elf->fd, section_header->sh_offset, true) &&
           storage_file_read(elf->fd, elf->debug_link_info.debug_link, section_header->sh_size) ==
               section_header->sh_size;
}

static bool str_prefix(const char* str, const char* prefix) {
    return strncmp(prefix, str, strlen(prefix)) == 0;
}

static bool elf_load_section_data(ELFFile* elf, ELFSection* section, Elf32_Shdr* section_header) {
    if(section_header->sh_size == 0) {
        FURI_LOG_D(TAG, "No data for section");
        return true;
    }

    section->data = aligned_malloc(section_header->sh_size, section_header->sh_addralign);
    section->size = section_header->sh_size;

    if(section_header->sh_type == SHT_NOBITS) {
        // BSS section, no data to load
        return true;
    }

    if((!storage_file_seek(elf->fd, section_header->sh_offset, true)) ||
       (storage_file_read(elf->fd, section->data, section_header->sh_size) !=
        section_header->sh_size)) {
        FURI_LOG_E(TAG, "    seek/read fail");
        return false;
    }

    FURI_LOG_D(TAG, "0x%p", section->data);
    return true;
}

static SectionType elf_preload_section(
    ELFFile* elf,
    size_t section_idx,
    Elf32_Shdr* section_header,
    FuriString* name_string) {
    const char* name = furi_string_get_cstr(name_string);

#ifdef ELF_DEBUG_LOG
    // log section name, type and flags
    FuriString* flags_string = furi_string_alloc();
    if(section_header->sh_flags & SHF_WRITE) furi_string_cat(flags_string, "W");
    if(section_header->sh_flags & SHF_ALLOC) furi_string_cat(flags_string, "A");
    if(section_header->sh_flags & SHF_EXECINSTR) furi_string_cat(flags_string, "X");
    if(section_header->sh_flags & SHF_MERGE) furi_string_cat(flags_string, "M");
    if(section_header->sh_flags & SHF_STRINGS) furi_string_cat(flags_string, "S");
    if(section_header->sh_flags & SHF_INFO_LINK) furi_string_cat(flags_string, "I");
    if(section_header->sh_flags & SHF_LINK_ORDER) furi_string_cat(flags_string, "L");
    if(section_header->sh_flags & SHF_OS_NONCONFORMING) furi_string_cat(flags_string, "O");
    if(section_header->sh_flags & SHF_GROUP) furi_string_cat(flags_string, "G");
    if(section_header->sh_flags & SHF_TLS) furi_string_cat(flags_string, "T");
    if(section_header->sh_flags & SHF_COMPRESSED) furi_string_cat(flags_string, "T");
    if(section_header->sh_flags & SHF_MASKOS) furi_string_cat(flags_string, "o");
    if(section_header->sh_flags & SHF_MASKPROC) furi_string_cat(flags_string, "p");
    if(section_header->sh_flags & SHF_ORDERED) furi_string_cat(flags_string, "R");
    if(section_header->sh_flags & SHF_EXCLUDE) furi_string_cat(flags_string, "E");

    FURI_LOG_I(
        TAG,
        "Section %s: type: %ld, flags: %s",
        name,
        section_header->sh_type,
        furi_string_get_cstr(flags_string));
    furi_string_free(flags_string);
#endif

    // ignore .ARM and .rel.ARM sections
    // TODO: how to do it not by name?
    // .ARM: type 0x70000001, flags SHF_ALLOC | SHF_LINK_ORDER
    // .rel.ARM: type 0x9, flags SHT_REL
    if(str_prefix(name, ".ARM.") || str_prefix(name, ".rel.ARM.")) {
        FURI_LOG_D(TAG, "Ignoring ARM section");
        return SectionTypeUnused;
    }

    // Load allocable section
    if(section_header->sh_flags & SHF_ALLOC) {
        ELFSection* section_p = elf_file_get_or_put_section(elf, name);
        section_p->sec_idx = section_idx;

        if(section_header->sh_type == SHT_PREINIT_ARRAY) {
            furi_assert(elf->preinit_array == NULL);
            elf->preinit_array = section_p;
        } else if(section_header->sh_type == SHT_INIT_ARRAY) {
            furi_assert(elf->init_array == NULL);
            elf->init_array = section_p;
        } else if(section_header->sh_type == SHT_FINI_ARRAY) {
            furi_assert(elf->fini_array == NULL);
            elf->fini_array = section_p;
        }

        if(!elf_load_section_data(elf, section_p, section_header)) {
            FURI_LOG_E(TAG, "Error loading section '%s'", name);
            return SectionTypeERROR;
        } else {
            return SectionTypeData;
        }
    }

    // Load link info section
    if(section_header->sh_flags & SHF_INFO_LINK) {
        name = name + strlen(".rel");
        ELFSection* section_p = elf_file_get_or_put_section(elf, name);
        section_p->rel_count = section_header->sh_size / sizeof(Elf32_Rel);
        section_p->rel_offset = section_header->sh_offset;
        return SectionTypeRelData;
    }

    // Load symbol table
    if(strcmp(name, ".symtab") == 0) {
        FURI_LOG_D(TAG, "Found .symtab section");
        elf->symbol_table = section_header->sh_offset;
        elf->symbol_count = section_header->sh_size / sizeof(Elf32_Sym);
        return SectionTypeSymTab;
    }

    // Load string table
    if(strcmp(name, ".strtab") == 0) {
        FURI_LOG_D(TAG, "Found .strtab section");
        elf->symbol_table_strings = section_header->sh_offset;
        return SectionTypeStrTab;
    }

    // Load debug link section
    if(strcmp(name, ".gnu_debuglink") == 0) {
        FURI_LOG_D(TAG, "Found .gnu_debuglink section");
        if(elf_load_debug_link(elf, section_header)) {
            return SectionTypeDebugLink;
        } else {
            return SectionTypeERROR;
        }
    }

    return SectionTypeUnused;
}

static bool elf_relocate_section(ELFFile* elf, ELFSection* section) {
    if(section->rel_count) {
        FURI_LOG_D(TAG, "Relocating section");
        return elf_relocate(elf, section);
    } else {
        FURI_LOG_D(TAG, "No relocation index"); /* Not an error */
    }
    return true;
}

static void elf_file_call_section_list(ELFSection* section, bool reverse_order) {
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
    elf->init_array_called = false;
    return elf;
}

void elf_file_free(ELFFile* elf) {
    // furi_check(!elf->init_array_called);
    if(elf->init_array_called) {
        FURI_LOG_W(TAG, "Init array was called, but fini array wasn't");
        elf_file_call_section_list(elf->fini_array, true);
    }

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

bool elf_file_load_section_table(ELFFile* elf) {
    SectionType loaded_sections = SectionTypeERROR;
    FuriString* name = furi_string_alloc();

    FURI_LOG_D(TAG, "Scan ELF indexs...");
    // TODO: why we start from 1?
    for(size_t section_idx = 1; section_idx < elf->sections_count; section_idx++) {
        Elf32_Shdr section_header;

        furi_string_reset(name);
        if(!elf_read_section(elf, section_idx, &section_header, name)) {
            loaded_sections = SectionTypeERROR;
            break;
        }

        FURI_LOG_D(
            TAG, "Preloading data for section #%d %s", section_idx, furi_string_get_cstr(name));
        SectionType section_type = elf_preload_section(elf, section_idx, &section_header, name);
        loaded_sections |= section_type;

        if(section_type == SectionTypeERROR) {
            loaded_sections = SectionTypeERROR;
            break;
        }
    }

    furi_string_free(name);

    return IS_FLAGS_SET(loaded_sections, SectionTypeValid);
}

ElfProcessSectionResult elf_process_section(
    ELFFile* elf,
    const char* name,
    ElfProcessSection* process_section,
    void* context) {
    ElfProcessSectionResult result = ElfProcessSectionResultNotFound;
    FuriString* section_name = furi_string_alloc();
    Elf32_Shdr section_header;

    // find section
    // TODO: why we start from 1?
    for(size_t section_idx = 1; section_idx < elf->sections_count; section_idx++) {
        furi_string_reset(section_name);
        if(!elf_read_section(elf, section_idx, &section_header, section_name)) {
            break;
        }

        if(furi_string_cmp(section_name, name) == 0) {
            result = ElfProcessSectionResultCannotProcess;
            break;
        }
    }

    if(result != ElfProcessSectionResultNotFound) { //-V547
        if(process_section(elf->fd, section_header.sh_offset, section_header.sh_size, context)) {
            result = ElfProcessSectionResultSuccess;
        } else {
            result = ElfProcessSectionResultCannotProcess;
        }
    }

    furi_string_free(section_name);

    return result;
}

ELFFileLoadStatus elf_file_load_sections(ELFFile* elf) {
    ELFFileLoadStatus status = ELFFileLoadStatusSuccess;
    ELFSectionDict_it_t it;

    AddressCache_init(elf->relocation_cache);

    for(ELFSectionDict_it(it, elf->sections); !ELFSectionDict_end_p(it); ELFSectionDict_next(it)) {
        ELFSectionDict_itref_t* itref = ELFSectionDict_ref(it);
        FURI_LOG_D(TAG, "Relocating section '%s'", itref->key);
        if(!elf_relocate_section(elf, &itref->value)) {
            FURI_LOG_E(TAG, "Error relocating section '%s'", itref->key);
            status = ELFFileLoadStatusMissingImports;
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

    {
        size_t total_size = 0;
        for(ELFSectionDict_it(it, elf->sections); !ELFSectionDict_end_p(it);
            ELFSectionDict_next(it)) {
            ELFSectionDict_itref_t* itref = ELFSectionDict_ref(it);
            total_size += itref->value.size;
        }
        FURI_LOG_I(TAG, "Total size of loaded sections: %u", total_size); //-V576
    }

    return status;
}

void elf_file_call_init(ELFFile* elf) {
    furi_check(!elf->init_array_called);
    elf_file_call_section_list(elf->preinit_array, false);
    elf_file_call_section_list(elf->init_array, false);
    elf->init_array_called = true;
}

bool elf_file_is_init_complete(ELFFile* elf) {
    return elf->init_array_called;
}

void* elf_file_get_entry_point(ELFFile* elf) {
    furi_check(elf->init_array_called);
    return (void*)elf->entry;
}

void elf_file_call_fini(ELFFile* elf) {
    furi_check(elf->init_array_called);
    elf_file_call_section_list(elf->fini_array, true);
    elf->init_array_called = false;
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
