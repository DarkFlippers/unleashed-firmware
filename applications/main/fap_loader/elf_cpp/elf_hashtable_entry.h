#pragma once
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

struct sym_entry {
    uint32_t hash;
    uint32_t address;
};

#ifdef __cplusplus
}

#include <array>
#include <algorithm>

#define API_METHOD(x, ret_type, args_type)                                                     \
    sym_entry {                                                                                \
        .hash = elf_gnu_hash(#x), .address = (uint32_t)(static_cast<ret_type(*) args_type>(x)) \
    }

#define API_VARIABLE(x, var_type)                              \
    sym_entry {                                                \
        .hash = elf_gnu_hash(#x), .address = (uint32_t)(&(x)), \
    }

constexpr bool operator<(const sym_entry& k1, const sym_entry& k2) {
    return k1.hash < k2.hash;
}

constexpr uint32_t elf_gnu_hash(const char* s) {
    uint32_t h = 0x1505;
    for(unsigned char c = *s; c != '\0'; c = *++s) {
        h = (h << 5) + h + c;
    }
    return h;
}

#endif
