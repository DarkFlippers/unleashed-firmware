#include "parity.h"

uint32_t __paritysi2(uint32_t a) {
    uint32_t x = (uint32_t)a;
    x ^= x >> 16;
    x ^= x >> 8;
    x ^= x >> 4;
    return (0x6996 >> (x & 0xF)) & 1;
}

static const uint8_t g_odd_byte_parity[256] = {
    1, 0, 0, 1, 0, 1, 1, 0, 0, 1, 1, 0, 1, 0, 0, 1, 0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0,
    1, 1, 0, 0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0, 1, 0, 0, 1, 0, 1, 1, 0, 0, 1,
    1, 0, 1, 0, 0, 1, 0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0, 1, 0, 0, 1, 0, 1, 1,
    0, 0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0, 0, 1, 1, 0, 1, 0, 0, 1, 0, 1, 1, 0,
    1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0, 0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0, 1,
    0, 0, 1, 0, 1, 1, 0, 0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0, 0, 1, 1, 0, 1, 0,
    0, 1, 0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0, 1, 0, 0, 1, 0, 1, 1, 0, 0, 1, 1,
    0, 1, 0, 0, 1, 0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0, 0, 1, 1, 0, 1, 0, 0, 1,
    1, 0, 0, 1, 0, 1, 1, 0, 1, 0, 0, 1, 0, 1, 1, 0, 0, 1, 1, 0, 1, 0, 0, 1};

#define ODD_PARITY8(x) \
    { g_odd_byte_parity[x] }
#define EVEN_PARITY8(x) \
    { !g_odd_byte_parity[x] }

uint8_t oddparity8(const uint8_t x) {
    return g_odd_byte_parity[x];
}

uint8_t evenparity8(const uint8_t x) {
    return !g_odd_byte_parity[x];
}

uint8_t evenparity16(uint16_t x) {
#if !defined __GNUC__
    x ^= x >> 8;
    return EVEN_PARITY8(x);
#else
    return (__builtin_parity(x) & 0xFF);
#endif
}

uint8_t oddparity16(uint16_t x) {
#if !defined __GNUC__
    x ^= x >> 8;
    return ODD_PARITY8(x);
#else
    return !__builtin_parity(x);
#endif
}

uint8_t evenparity32(uint32_t x) {
#if !defined __GNUC__
    x ^= x >> 16;
    x ^= x >> 8;
    return EVEN_PARITY8(x);
#else
    return (__builtin_parity(x) & 0xFF);
#endif
}

uint8_t oddparity32(uint32_t x) {
#if !defined __GNUC__
    x ^= x >> 16;
    x ^= x >> 8;
    return ODD_PARITY8(x);
#else
    return !__builtin_parity(x);
#endif
}