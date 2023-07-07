#include "api_hashtable.h"

#include <furi.h>
#include <algorithm>

#define TAG "hashtable_api"

bool elf_resolve_from_hashtable(
    const ElfApiInterface* interface,
    uint32_t hash,
    Elf32_Addr* address) {
    bool result = false;
    const HashtableApiInterface* hashtable_interface =
        static_cast<const HashtableApiInterface*>(interface);

    sym_entry key = {
        .hash = hash,
        .address = 0,
    };

    auto find_res =
        std::lower_bound(hashtable_interface->table_cbegin, hashtable_interface->table_cend, key);
    if((find_res == hashtable_interface->table_cend || (find_res->hash != hash))) {
        FURI_LOG_W(
            TAG, "Can't find symbol with hash %lx @ %p!", hash, hashtable_interface->table_cbegin);
        result = false;
    } else {
        result = true;
        *address = find_res->address;
    }

    return result;
}

uint32_t elf_symbolname_hash(const char* s) {
    return elf_gnu_hash(s);
}