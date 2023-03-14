#include "api_hashtable.h"

#include <furi.h>
#include <algorithm>

#define TAG "hashtable_api"

bool elf_resolve_from_hashtable(
    const ElfApiInterface* interface,
    const char* name,
    Elf32_Addr* address) {
    const HashtableApiInterface* hashtable_interface =
        static_cast<const HashtableApiInterface*>(interface);
    bool result = false;
    uint32_t gnu_sym_hash = elf_gnu_hash(name);

    sym_entry key = {
        .hash = gnu_sym_hash,
        .address = 0,
    };

    auto find_res =
        std::lower_bound(hashtable_interface->table_cbegin, hashtable_interface->table_cend, key);
    if((find_res == hashtable_interface->table_cend || (find_res->hash != gnu_sym_hash))) {
        FURI_LOG_W(
            TAG,
            "Can't find symbol '%s' (hash %lx) @ %p!",
            name,
            gnu_sym_hash,
            hashtable_interface->table_cbegin);
        result = false;
    } else {
        result = true;
        *address = find_res->address;
    }

    return result;
}
