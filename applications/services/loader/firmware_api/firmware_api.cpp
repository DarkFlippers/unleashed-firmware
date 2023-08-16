#include "firmware_api.h"

#include <flipper_application/api_hashtable/api_hashtable.h>
#include <flipper_application/api_hashtable/compilesort.hpp>

/* Generated table */
#include <firmware_api_table.h>

#include <furi_hal_info.h>

static_assert(!has_hash_collisions(elf_api_table), "Detected API method hash collision!");

#ifdef APP_UNIT_TESTS
constexpr HashtableApiInterface mock_elf_api_interface{
    {
        .api_version_major = 0,
        .api_version_minor = 0,
        .resolver_callback = &elf_resolve_from_hashtable,
    },
    .table_cbegin = nullptr,
    .table_cend = nullptr,
};

const ElfApiInterface* const firmware_api_interface = &mock_elf_api_interface;
#else
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
#endif

extern "C" void furi_hal_info_get_api_version(uint16_t* major, uint16_t* minor) {
    *major = firmware_api_interface->api_version_major;
    *minor = firmware_api_interface->api_version_minor;
}