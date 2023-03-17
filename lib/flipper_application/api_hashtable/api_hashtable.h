#pragma once

#include <flipper_application/elf/elf_api_interface.h>

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Symbol table entry
 */
struct sym_entry {
    uint32_t hash;
    uint32_t address;
};

/**
 * @brief Resolver for API entries using a pre-sorted table with hashes
 * @param interface pointer to HashtableApiInterface
 * @param name function name
 * @param address output for function address
 * @return true if the table contains a function
 */
bool elf_resolve_from_hashtable(
    const ElfApiInterface* interface,
    const char* name,
    Elf32_Addr* address);

#ifdef __cplusplus
}

#include <array>
#include <algorithm>

/**
 * @brief  HashtableApiInterface is an implementation of ElfApiInterface
 * that uses a hash table to resolve function addresses.
 * table_cbegin and table_cend must point to a sorted array of sym_entry
 */
struct HashtableApiInterface : public ElfApiInterface {
    const sym_entry *table_cbegin, *table_cend;
};

#define API_METHOD(x, ret_type, args_type)                                                     \
    sym_entry {                                                                                \
        .hash = elf_gnu_hash(#x), .address = (uint32_t)(static_cast<ret_type(*) args_type>(x)) \
    }

#define API_VARIABLE(x, var_type) \
    sym_entry { .hash = elf_gnu_hash(#x), .address = (uint32_t)(&(x)), }

constexpr bool operator<(const sym_entry& k1, const sym_entry& k2) {
    return k1.hash < k2.hash;
}

/**
 * @brief Calculate hash for a string using the ELF GNU hash algorithm
 * @param s string to calculate hash for
 * @return hash value
 */
constexpr uint32_t elf_gnu_hash(const char* s) {
    uint32_t h = 0x1505;
    for(unsigned char c = *s; c != '\0'; c = *++s) {
        h = (h << 5) + h + c;
    }
    return h;
}

/* Compile-time check for hash collisions in API table.
 * Usage: static_assert(!has_hash_collisions(api_methods), "Hash collision detected"); 
 */
template <std::size_t N>
constexpr bool has_hash_collisions(const std::array<sym_entry, N>& api_methods) {
    for(std::size_t i = 0; i < (N - 1); ++i) {
        if(api_methods[i].hash == api_methods[i + 1].hash) {
            return true;
        }
    }

    return false;
}

#endif