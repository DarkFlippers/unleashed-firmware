#pragma once

#include "elf.h"
#include "flipper_application.h"
#include <m-dict.h>

#ifdef __cplusplus
extern "C" {
#endif

DICT_DEF2(RelocationAddressCache, int, M_DEFAULT_OPLIST, Elf32_Addr, M_DEFAULT_OPLIST)

/**
 * Callable elf entry type
 */
typedef int32_t(entry_t)(void*);

typedef struct {
    void* data;
    uint16_t sec_idx;
    uint16_t rel_sec_idx;
} ELFSection_t;

struct FlipperApplication {
    const ElfApiInterface* api_interface;
    File* fd;
    FlipperApplicationState state;
    FlipperApplicationManifest manifest;

    size_t sections;
    off_t section_table;
    off_t section_table_strings;

    size_t symbol_count;
    off_t symbol_table;
    off_t symbol_table_strings;
    off_t entry;

    ELFSection_t text;
    ELFSection_t rodata;
    ELFSection_t data;
    ELFSection_t bss;

    FuriThread* thread;
    RelocationAddressCache_t relocation_cache;
};

typedef enum {
    FoundERROR = 0,
    FoundSymTab = (1 << 0),
    FoundStrTab = (1 << 2),
    FoundText = (1 << 3),
    FoundRodata = (1 << 4),
    FoundData = (1 << 5),
    FoundBss = (1 << 6),
    FoundRelText = (1 << 7),
    FoundRelRodata = (1 << 8),
    FoundRelData = (1 << 9),
    FoundRelBss = (1 << 10),
    FoundFappManifest = (1 << 11),
    FoundDebugLink = (1 << 12),
    FoundValid = FoundSymTab | FoundStrTab | FoundFappManifest,
    FoundExec = FoundValid | FoundText,
    FoundGdbSection = FoundText | FoundRodata | FoundData | FoundBss,
    FoundAll = FoundSymTab | FoundStrTab | FoundText | FoundRodata | FoundData | FoundBss |
               FoundRelText | FoundRelRodata | FoundRelData | FoundRelBss | FoundDebugLink,
} FindFlags_t;

/**
 * @brief Load and validate basic ELF file headers
 * @param e Application instance
 * @param path FS path to application file
 * @return true if ELF file is valid 
 */
bool flipper_application_load_elf_headers(FlipperApplication* e, const char* path);

/**
 * @brief Iterate over all sections and save related indexes
 * @param e Application instance
 * @return true if all required sections are found
 */
bool flipper_application_load_section_table(FlipperApplication* e);

/**
 * @brief Load section data to memory and process relocations
 * @param e Application instance 
 * @return Status code
 */
FlipperApplicationLoadStatus flipper_application_load_sections(FlipperApplication* e);

/**
 * @brief Release section data
 * @param s section pointer
 */
void flipper_application_free_section(ELFSection_t* s);

#ifdef __cplusplus
}
#endif