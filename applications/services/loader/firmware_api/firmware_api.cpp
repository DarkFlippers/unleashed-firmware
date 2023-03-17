#include "firmware_api.h"

#include <flipper_application/api_hashtable/api_hashtable.h>
#include <flipper_application/api_hashtable/compilesort.hpp>

/* Generated table */
#include <symbols.h>

static_assert(!has_hash_collisions(elf_api_table), "Detected API method hash collision!");

constexpr HashtableApiInterface elf_api_interface{
    {
        .api_version_major = (elf_api_version >> 16),
        .api_version_minor = (elf_api_version & 0xFFFF),
        .resolver_callback = &elf_resolve_from_hashtable,
    },
    .table_cbegin = elf_api_table.cbegin(),
    .table_cend = elf_api_table.cend(),
};

const ElfApiInterface* const firmware_api_interface = &elf_api_interface;
