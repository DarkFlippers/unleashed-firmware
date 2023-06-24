//
// Created by Avilov Vasily on 10.06.2023.
//

#ifndef FLIPPERZERO_FIRMWARE_ENDIANNESS_H
#define FLIPPERZERO_FIRMWARE_ENDIANNESS_H

inline static void store16(uint8_t* b, uint16_t i) {
    memcpy(b, &i, 2);
}

inline static void store32(uint8_t* b, uint32_t i) {
    memcpy(b, &i, 4);
}

inline static uint16_t load16(uint8_t* b) {
    uint16_t x;
    memcpy(&x, b, 2);
    return x;
}

inline static uint32_t load32(uint8_t* b) {
    uint32_t x;
    memcpy(&x, b, 4);
    return x;
}

#if BYTE_ORDER == BIG_ENDIAN
#define htobe16(x) (x)
#define htobe32(x) (x)
#define htole16(x) __builtin_bswap16(x)
#define htole32(x) __builtin_bswap32(x)
#define be16toh(x) (x)
#define be32toh(x) (x)
#define le16toh(x) __builtin_bswap16(x)
#define le32toh(x) __builtin_bswap32(x)
#elif BYTE_ORDER == LITTLE_ENDIAN
#define htobe16(x) __builtin_bswap16(x)
#define htobe32(x) __builtin_bswap32(x)
#define htole16(x) (x)
#define htole32(x) (x)
#define be16toh(x) __builtin_bswap16(x)
#define be32toh(x) __builtin_bswap32(x)
#define le16toh(x) (x)
#define le32toh(x) (x)
#else
#error "What kind of system is this?"
#endif

#define load16_le(b) (le16toh(load16(b)))
#define load32_le(b) (le32toh(load32(b)))
#define store16_le(b, i) (store16(b, htole16(i)))
#define store32_le(b, i) (store32(b, htole32(i)))

#define load16_be(b) (be16toh(load16(b)))
#define load32_be(b) (be32toh(load32(b)))
#define store16_be(b, i) (store16(b, htobe16(i)))
#define store32_be(b, i) (store32(b, htobe32(i)))

#endif //FLIPPERZERO_FIRMWARE_ENDIANNESS_H
