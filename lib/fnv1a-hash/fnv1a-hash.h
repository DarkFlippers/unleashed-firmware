#pragma once
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define FNV_1A_INIT 2166136261UL

// FNV-1a hash, 32-bit
uint32_t fnv1a_buffer_hash(const uint8_t* buffer, uint32_t length, uint32_t hash);

#ifdef __cplusplus
}
#endif

#ifdef __cplusplus
// constexpr FNV-1a hash for strings, 32-bit
inline constexpr uint32_t fnv1a_string_hash(const char* str) {
    uint32_t hash = FNV_1A_INIT;

    while(*str) {
        hash = (hash ^ *str) * 16777619ULL;
        str += 1;
    }
    return hash;
}
#else
// FNV-1a hash for strings, 32-bit
inline uint32_t fnv1a_string_hash(const char* str) {
    uint32_t hash = FNV_1A_INIT;

    while(*str) {
        hash = (hash ^ *str) * 16777619ULL;
        str += 1;
    }
    return hash;
}
#endif
