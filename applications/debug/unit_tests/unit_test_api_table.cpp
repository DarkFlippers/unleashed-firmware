#include <flipper_application/api_hashtable/api_hashtable.h>
#include <flipper_application/api_hashtable/compilesort.hpp>

#include "unit_test_api_table_i.h"

static_assert(!has_hash_collisions(unit_tests_api_table), "Detected API method hash collision!");

constexpr HashtableApiInterface unit_tests_hashtable_api_interface{
    {
        .api_version_major = 0,
        .api_version_minor = 0,
        .resolver_callback = &elf_resolve_from_hashtable,
    },
    unit_tests_api_table.cbegin(),
    unit_tests_api_table.cend(),
};

extern "C" const ElfApiInterface* const unit_tests_api_interface =
    &unit_tests_hashtable_api_interface;
