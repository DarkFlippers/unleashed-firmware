#include "fnv1a-hash.h"

// FNV-1a hash, 32-bit
uint32_t fnv1a_buffer_hash(const uint8_t* buffer, uint32_t length, uint32_t hash)
{
    for (uint32_t i = 0; i < length; i++) {
        hash = (hash ^ buffer[i]) * 16777619ULL;
    }
    return hash;
}
