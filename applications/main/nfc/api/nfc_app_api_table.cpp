#include <flipper_application/api_hashtable/api_hashtable.h>
#include <flipper_application/api_hashtable/compilesort.hpp>

/* 
 * This file contains an implementation of a symbol table 
 * with private app's symbols. It is used by composite API resolver
 * to load plugins that use internal application's APIs.
 */
#include "nfc_app_api_table_i.h"

static_assert(!has_hash_collisions(nfc_app_api_table), "Detected API method hash collision!");

constexpr HashtableApiInterface nfc_application_hashtable_api_interface{
    {
        .api_version_major = 0,
        .api_version_minor = 0,
        /* generic resolver using pre-sorted array */
        .resolver_callback = &elf_resolve_from_hashtable,
    },
    /* pointers to application's API table boundaries */
    nfc_app_api_table.cbegin(),
    nfc_app_api_table.cend(),
};

/* Casting to generic resolver to use in Composite API resolver */
extern "C" const ElfApiInterface* const nfc_application_api_interface =
    &nfc_application_hashtable_api_interface;
