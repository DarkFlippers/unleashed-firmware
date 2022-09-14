#include "flipper_application_i.h"
#include <furi.h>

#define TAG "fapp-i"

#define RESOLVER_THREAD_YIELD_STEP 30

#define IS_FLAGS_SET(v, m) ((v & m) == m)
#define SECTION_OFFSET(e, n) (e->section_table + n * sizeof(Elf32_Shdr))
#define SYMBOL_OFFSET(e, n) (e->_table + n * sizeof(Elf32_Shdr))

bool flipper_application_load_elf_headers(FlipperApplication* e, const char* path) {
    Elf32_Ehdr h;
    Elf32_Shdr sH;

    if(!storage_file_open(e->fd, path, FSAM_READ, FSOM_OPEN_EXISTING) ||
       !storage_file_seek(e->fd, 0, true) ||
       storage_file_read(e->fd, &h, sizeof(h)) != sizeof(h) ||
       !storage_file_seek(e->fd, h.e_shoff + h.e_shstrndx * sizeof(sH), true) ||
       storage_file_read(e->fd, &sH, sizeof(Elf32_Shdr)) != sizeof(Elf32_Shdr)) {
        return false;
    }

    e->entry = h.e_entry;
    e->sections = h.e_shnum;
    e->section_table = h.e_shoff;
    e->section_table_strings = sH.sh_offset;
    return true;
}

static bool flipper_application_load_metadata(FlipperApplication* e, Elf32_Shdr* sh) {
    if(sh->sh_size < sizeof(e->manifest)) {
        return false;
    }

    return storage_file_seek(e->fd, sh->sh_offset, true) &&
           storage_file_read(e->fd, &e->manifest, sh->sh_size) == sh->sh_size;
}

static bool flipper_application_load_debug_link(FlipperApplication* e, Elf32_Shdr* sh) {
    e->state.debug_link_size = sh->sh_size;
    e->state.debug_link = malloc(sh->sh_size);

    return storage_file_seek(e->fd, sh->sh_offset, true) &&
           storage_file_read(e->fd, e->state.debug_link, sh->sh_size) == sh->sh_size;
}

static FindFlags_t flipper_application_preload_section(
    FlipperApplication* e,
    Elf32_Shdr* sh,
    const char* name,
    int n) {
    FURI_LOG_D(TAG, "Processing: %s", name);

    const struct {
        const char* name;
        uint16_t* ptr_section_idx;
        FindFlags_t flags;
    } lookup_sections[] = {
        {".text", &e->text.sec_idx, FoundText},
        {".rodata", &e->rodata.sec_idx, FoundRodata},
        {".data", &e->data.sec_idx, FoundData},
        {".bss", &e->bss.sec_idx, FoundBss},
        {".rel.text", &e->text.rel_sec_idx, FoundRelText},
        {".rel.rodata", &e->rodata.rel_sec_idx, FoundRelRodata},
        {".rel.data", &e->data.rel_sec_idx, FoundRelData},
    };

    for(size_t i = 0; i < COUNT_OF(lookup_sections); i++) {
        if(strcmp(name, lookup_sections[i].name) == 0) {
            *lookup_sections[i].ptr_section_idx = n;
            return lookup_sections[i].flags;
        }
    }

    if(strcmp(name, ".symtab") == 0) {
        e->symbol_table = sh->sh_offset;
        e->symbol_count = sh->sh_size / sizeof(Elf32_Sym);
        return FoundSymTab;
    } else if(strcmp(name, ".strtab") == 0) {
        e->symbol_table_strings = sh->sh_offset;
        return FoundStrTab;
    } else if(strcmp(name, ".fapmeta") == 0) {
        // Load metadata immediately
        if(flipper_application_load_metadata(e, sh)) {
            return FoundFappManifest;
        }
    } else if(strcmp(name, ".gnu_debuglink") == 0) {
        if(flipper_application_load_debug_link(e, sh)) {
            return FoundDebugLink;
        }
    }
    return FoundERROR;
}

static bool
    read_string_from_offset(FlipperApplication* e, off_t offset, char* buffer, size_t buffer_size) {
    bool success = false;

    off_t old = storage_file_tell(e->fd);
    if(storage_file_seek(e->fd, offset, true) &&
       (storage_file_read(e->fd, buffer, buffer_size) == buffer_size)) {
        success = true;
    }
    storage_file_seek(e->fd, old, true);

    return success;
}

static bool read_section_name(FlipperApplication* e, off_t off, char* buf, size_t max) {
    return read_string_from_offset(e, e->section_table_strings + off, buf, max);
}

static bool read_symbol_name(FlipperApplication* e, off_t off, char* buf, size_t max) {
    return read_string_from_offset(e, e->symbol_table_strings + off, buf, max);
}

static bool read_section_header(FlipperApplication* e, int n, Elf32_Shdr* h) {
    off_t offset = SECTION_OFFSET(e, n);
    return storage_file_seek(e->fd, offset, true) &&
           storage_file_read(e->fd, h, sizeof(Elf32_Shdr)) == sizeof(Elf32_Shdr);
}

static bool read_section(FlipperApplication* e, int n, Elf32_Shdr* h, char* name, size_t nlen) {
    if(!read_section_header(e, n, h)) {
        return false;
    }
    if(!h->sh_name) {
        return true;
    }
    return read_section_name(e, h->sh_name, name, nlen);
}

bool flipper_application_load_section_table(FlipperApplication* e) {
    furi_check(e->state.mmap_entry_count == 0);

    size_t n;
    FindFlags_t found = FoundERROR;
    FURI_LOG_D(TAG, "Scan ELF indexs...");
    for(n = 1; n < e->sections; n++) {
        Elf32_Shdr section_header;
        char name[33] = {0};
        if(!read_section_header(e, n, &section_header)) {
            return false;
        }
        if(section_header.sh_name &&
           !read_section_name(e, section_header.sh_name, name, sizeof(name))) {
            return false;
        }

        FURI_LOG_T(TAG, "Examining section %d %s", n, name);
        FindFlags_t section_flags =
            flipper_application_preload_section(e, &section_header, name, n);
        found |= section_flags;
        if((section_flags & FoundGdbSection) != 0) {
            e->state.mmap_entry_count++;
        }
        if(IS_FLAGS_SET(found, FoundAll)) {
            return true;
        }
    }

    FURI_LOG_D(TAG, "Load symbols done");
    return IS_FLAGS_SET(found, FoundValid);
}

static const char* type_to_str(int symt) {
#define STRCASE(name) \
    case name:        \
        return #name;
    switch(symt) {
        STRCASE(R_ARM_NONE)
        STRCASE(R_ARM_ABS32)
        STRCASE(R_ARM_THM_PC22)
        STRCASE(R_ARM_THM_JUMP24)
    default:
        return "R_<unknow>";
    }
#undef STRCASE
}

static void relocate_jmp_call(Elf32_Addr relAddr, int type, Elf32_Addr symAddr) {
    UNUSED(type);
    uint16_t upper_insn = ((uint16_t*)relAddr)[0];
    uint16_t lower_insn = ((uint16_t*)relAddr)[1];
    uint32_t S = (upper_insn >> 10) & 1;
    uint32_t J1 = (lower_insn >> 13) & 1;
    uint32_t J2 = (lower_insn >> 11) & 1;

    int32_t offset = (S << 24) | /* S     -> offset[24] */
                     ((~(J1 ^ S) & 1) << 23) | /* J1    -> offset[23] */
                     ((~(J2 ^ S) & 1) << 22) | /* J2    -> offset[22] */
                     ((upper_insn & 0x03ff) << 12) | /* imm10 -> offset[12:21] */
                     ((lower_insn & 0x07ff) << 1); /* imm11 -> offset[1:11] */
    if(offset & 0x01000000) offset -= 0x02000000;

    offset += symAddr - relAddr;

    S = (offset >> 24) & 1;
    J1 = S ^ (~(offset >> 23) & 1);
    J2 = S ^ (~(offset >> 22) & 1);

    upper_insn = ((upper_insn & 0xf800) | (S << 10) | ((offset >> 12) & 0x03ff));
    ((uint16_t*)relAddr)[0] = upper_insn;

    lower_insn = ((lower_insn & 0xd000) | (J1 << 13) | (J2 << 11) | ((offset >> 1) & 0x07ff));
    ((uint16_t*)relAddr)[1] = lower_insn;
}

static bool relocate_symbol(Elf32_Addr relAddr, int type, Elf32_Addr symAddr) {
    switch(type) {
    case R_ARM_ABS32:
        *((uint32_t*)relAddr) += symAddr;
        FURI_LOG_D(TAG, "  R_ARM_ABS32 relocated is 0x%08X", (unsigned int)*((uint32_t*)relAddr));
        break;
    case R_ARM_THM_PC22:
    case R_ARM_THM_JUMP24:
        relocate_jmp_call(relAddr, type, symAddr);
        FURI_LOG_D(
            TAG, "  R_ARM_THM_CALL/JMP relocated is 0x%08X", (unsigned int)*((uint32_t*)relAddr));
        break;
    default:
        FURI_LOG_D(TAG, "  Undefined relocation %d", type);
        return false;
    }
    return true;
}

static ELFSection_t* section_of(FlipperApplication* e, int index) {
    if(e->text.sec_idx == index) {
        return &e->text;
    } else if(e->data.sec_idx == index) {
        return &e->data;
    } else if(e->bss.sec_idx == index) {
        return &e->bss;
    } else if(e->rodata.sec_idx == index) {
        return &e->rodata;
    }
    return NULL;
}

static Elf32_Addr address_of(FlipperApplication* e, Elf32_Sym* sym, const char* sName) {
    if(sym->st_shndx == SHN_UNDEF) {
        Elf32_Addr addr = 0;
        if(e->api_interface->resolver_callback(sName, &addr)) {
            return addr;
        }
    } else {
        ELFSection_t* symSec = section_of(e, sym->st_shndx);
        if(symSec) {
            return ((Elf32_Addr)symSec->data) + sym->st_value;
        }
    }
    FURI_LOG_D(TAG, "  Can not find address for symbol %s", sName);
    return ELF_INVALID_ADDRESS;
}

static bool read_symbol(FlipperApplication* e, int n, Elf32_Sym* sym, char* name, size_t nlen) {
    bool success = false;
    off_t old = storage_file_tell(e->fd);
    off_t pos = e->symbol_table + n * sizeof(Elf32_Sym);
    if(storage_file_seek(e->fd, pos, true) &&
       storage_file_read(e->fd, sym, sizeof(Elf32_Sym)) == sizeof(Elf32_Sym)) {
        if(sym->st_name)
            success = read_symbol_name(e, sym->st_name, name, nlen);
        else {
            Elf32_Shdr shdr;
            success = read_section(e, sym->st_shndx, &shdr, name, nlen);
        }
    }
    storage_file_seek(e->fd, old, true);
    return success;
}

static bool
    relocation_cache_get(RelocationAddressCache_t cache, int symEntry, Elf32_Addr* symAddr) {
    Elf32_Addr* addr = RelocationAddressCache_get(cache, symEntry);
    if(addr) {
        *symAddr = *addr;
        return true;
    } else {
        return false;
    }
}

static void
    relocation_cache_put(RelocationAddressCache_t cache, int symEntry, Elf32_Addr symAddr) {
    RelocationAddressCache_set_at(cache, symEntry, symAddr);
}

#define MAX_SYMBOL_NAME_LEN 128u

static bool relocate(FlipperApplication* e, Elf32_Shdr* h, ELFSection_t* s) {
    if(s->data) {
        Elf32_Rel rel;
        size_t relEntries = h->sh_size / sizeof(rel);
        size_t relCount;
        (void)storage_file_seek(e->fd, h->sh_offset, true);
        FURI_LOG_D(TAG, " Offset   Info     Type             Name");

        int relocate_result = true;
        char symbol_name[MAX_SYMBOL_NAME_LEN + 1] = {0};

        for(relCount = 0; relCount < relEntries; relCount++) {
            if(relCount % RESOLVER_THREAD_YIELD_STEP == 0) {
                FURI_LOG_D(TAG, "  reloc YIELD");
                furi_delay_tick(1);
            }

            if(storage_file_read(e->fd, &rel, sizeof(Elf32_Rel)) != sizeof(Elf32_Rel)) {
                FURI_LOG_E(TAG, "  reloc read fail");
                return false;
            }

            Elf32_Addr symAddr;

            int symEntry = ELF32_R_SYM(rel.r_info);
            int relType = ELF32_R_TYPE(rel.r_info);
            Elf32_Addr relAddr = ((Elf32_Addr)s->data) + rel.r_offset;

            if(!relocation_cache_get(e->relocation_cache, symEntry, &symAddr)) {
                Elf32_Sym sym;
                if(!read_symbol(e, symEntry, &sym, symbol_name, MAX_SYMBOL_NAME_LEN)) {
                    FURI_LOG_E(TAG, "  symbol read fail");
                    return false;
                }

                FURI_LOG_D(
                    TAG,
                    " %08X %08X %-16s %s",
                    (unsigned int)rel.r_offset,
                    (unsigned int)rel.r_info,
                    type_to_str(relType),
                    symbol_name);

                symAddr = address_of(e, &sym, symbol_name);
                relocation_cache_put(e->relocation_cache, symEntry, symAddr);
            }

            if(symAddr != ELF_INVALID_ADDRESS) {
                FURI_LOG_D(
                    TAG,
                    "  symAddr=%08X relAddr=%08X",
                    (unsigned int)symAddr,
                    (unsigned int)relAddr);
                if(!relocate_symbol(relAddr, relType, symAddr)) {
                    relocate_result = false;
                }
            } else {
                FURI_LOG_D(TAG, "  No symbol address of %s", symbol_name);
                relocate_result = false;
            }
        }

        return relocate_result;
    } else
        FURI_LOG_I(TAG, "Section not loaded");

    return false;
}

static bool flipper_application_load_section_data(FlipperApplication* e, ELFSection_t* s) {
    Elf32_Shdr section_header;
    if(s->sec_idx == 0) {
        FURI_LOG_I(TAG, "Section is not present");
        return true;
    }

    if(!read_section_header(e, s->sec_idx, &section_header)) {
        return false;
    }

    if(section_header.sh_size == 0) {
        FURI_LOG_I(TAG, "No data for section");
        return true;
    }

    s->data = aligned_malloc(section_header.sh_size, section_header.sh_addralign);
    // e->state.mmap_entry_count++;

    if(section_header.sh_type == SHT_NOBITS) {
        /* section is empty (.bss?) */
        /* no need to memset - allocator already did that */
        /* memset(s->data, 0, h->sh_size); */
        FURI_LOG_D(TAG, "0x%X", s->data);
        return true;
    }

    if((!storage_file_seek(e->fd, section_header.sh_offset, true)) ||
       (storage_file_read(e->fd, s->data, section_header.sh_size) != section_header.sh_size)) {
        FURI_LOG_E(TAG, "    seek/read fail");
        flipper_application_free_section(s);
        return false;
    }

    FURI_LOG_D(TAG, "0x%X", s->data);
    return true;
}

static bool flipper_application_relocate_section(FlipperApplication* e, ELFSection_t* s) {
    Elf32_Shdr section_header;
    if(s->rel_sec_idx) {
        FURI_LOG_D(TAG, "Relocating section");
        if(read_section_header(e, s->rel_sec_idx, &section_header))
            return relocate(e, &section_header, s);
        else {
            FURI_LOG_E(TAG, "Error reading section header");
            return false;
        }
    } else
        FURI_LOG_D(TAG, "No relocation index"); /* Not an error */
    return true;
}

FlipperApplicationLoadStatus flipper_application_load_sections(FlipperApplication* e) {
    FlipperApplicationLoadStatus status = FlipperApplicationLoadStatusSuccess;
    RelocationAddressCache_init(e->relocation_cache);
    size_t start = furi_get_tick();

    struct {
        ELFSection_t* section;
        const char* name;
    } sections[] = {
        {&e->text, ".text"},
        {&e->rodata, ".rodata"},
        {&e->data, ".data"},
        {&e->bss, ".bss"},
    };

    for(size_t i = 0; i < COUNT_OF(sections); i++) {
        if(!flipper_application_load_section_data(e, sections[i].section)) {
            FURI_LOG_E(TAG, "Error loading section '%s'", sections[i].name);
            status = FlipperApplicationLoadStatusUnspecifiedError;
        }
    }

    if(status == FlipperApplicationLoadStatusSuccess) {
        for(size_t i = 0; i < COUNT_OF(sections); i++) {
            if(!flipper_application_relocate_section(e, sections[i].section)) {
                FURI_LOG_E(TAG, "Error relocating section '%s'", sections[i].name);
                status = FlipperApplicationLoadStatusMissingImports;
            }
        }
    }

    if(status == FlipperApplicationLoadStatusSuccess) {
        e->state.mmap_entries =
            malloc(sizeof(FlipperApplicationMemoryMapEntry) * e->state.mmap_entry_count);
        uint32_t mmap_entry_idx = 0;
        for(size_t i = 0; i < COUNT_OF(sections); i++) {
            const void* data_ptr = sections[i].section->data;
            if(data_ptr) {
                FURI_LOG_I(TAG, "0x%X %s", (uint32_t)data_ptr, sections[i].name);
                e->state.mmap_entries[mmap_entry_idx].address = (uint32_t)data_ptr;
                e->state.mmap_entries[mmap_entry_idx].name = sections[i].name;
                mmap_entry_idx++;
            }
        }
        furi_check(mmap_entry_idx == e->state.mmap_entry_count);

        /* Fixing up entry point */
        e->entry += (uint32_t)e->text.data;
    }

    FURI_LOG_D(TAG, "Relocation cache size: %u", RelocationAddressCache_size(e->relocation_cache));
    RelocationAddressCache_clear(e->relocation_cache);
    FURI_LOG_I(TAG, "Loaded in %ums", (size_t)(furi_get_tick() - start));

    return status;
}

void flipper_application_free_section(ELFSection_t* s) {
    if(s->data) {
        aligned_free(s->data);
    }
    s->data = NULL;
}
