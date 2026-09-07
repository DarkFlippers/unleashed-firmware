#include <flipper_application/api_hashtable/api_hashtable.h>
#include <flipper_application/api_hashtable/compilesort.hpp>

/*
 * This file contains an implementation of a symbol table
 * with private app's symbols. It is used by composite API resolver
 * to load plugins that use internal application's APIs.
 */
#include "subghz_app_api_table_i.h"

static_assert(!has_hash_collisions(subghz_app_api_table), "Detected API method hash collision!");

constexpr HashtableApiInterface subghz_application_hashtable_api_interface{
    {
        .api_version_major = 0,
        .api_version_minor = 0,
        /* generic resolver using pre-sorted array */
        .resolver_callback = &elf_resolve_from_hashtable,
    },
    /* pointers to application's API table boundaries */
    subghz_app_api_table.cbegin(),
    subghz_app_api_table.cend(),
};

/* Casting to generic resolver to use in Composite API resolver */
extern "C" const ElfApiInterface* const subghz_application_api_interface =
    &subghz_application_hashtable_api_interface;
