#include "keeloq_common.h"

#include <furi.h>

#include <m-array.h>

#define bit(x, n) (((x) >> (n)) & 1)
#define g5(x, a, b, c, d, e) \
    (bit(x, a) + bit(x, b) * 2 + bit(x, c) * 4 + bit(x, d) * 8 + bit(x, e) * 16)

/** Simple Learning Encrypt
 * @param data - 0xBSSSCCCC, B(4bit) key, S(10bit) serial&0x3FF, C(16bit) counter
 * @param key - manufacture (64bit)
 * @return keeloq encrypt data
 */
inline uint32_t subghz_protocol_keeloq_common_encrypt(const uint32_t data, const uint64_t key) {
    uint32_t x = data, r;
    for(r = 0; r < 528; r++)
        x = (x >> 1) ^ ((bit(x, 0) ^ bit(x, 16) ^ (uint32_t)bit(key, r & 63) ^
                         bit(KEELOQ_NLF, g5(x, 1, 9, 20, 26, 31)))
                        << 31);
    return x;
}

/** Simple Learning Decrypt
 * @param data - keeloq encrypt data
 * @param key - manufacture (64bit)
 * @return 0xBSSSCCCC, B(4bit) key, S(10bit) serial&0x3FF, C(16bit) counter
 */
inline uint32_t subghz_protocol_keeloq_common_decrypt(const uint32_t data, const uint64_t key) {
    uint32_t x = data, r;
    for(r = 0; r < 528; r++)
        x = (x << 1) ^ bit(x, 31) ^ bit(x, 15) ^ (uint32_t)bit(key, (15 - r) & 63) ^
            bit(KEELOQ_NLF, g5(x, 0, 8, 19, 25, 30));
    return x;
}

/** Normal Learning
 * @param data - serial number (28bit)
 * @param key - manufacture (64bit)
 * @return manufacture for this serial number (64bit)
 */
inline uint64_t subghz_protocol_keeloq_common_normal_learning(uint32_t data, const uint64_t key) {
    uint32_t k1, k2;

    data &= 0x0FFFFFFF;
    data |= 0x20000000;
    k1 = subghz_protocol_keeloq_common_decrypt(data, key);

    data &= 0x0FFFFFFF;
    data |= 0x60000000;
    k2 = subghz_protocol_keeloq_common_decrypt(data, key);

    return ((uint64_t)k2 << 32) | k1; // key - shifrovanoya
}

/** Secure Learning
 * @param data - serial number (28bit)
 * @param seed - seed number (32bit)
 * @param key - manufacture (64bit)
 * @return manufacture for this serial number (64bit)
 */

inline uint64_t subghz_protocol_keeloq_common_secure_learning(
    uint32_t data,
    uint32_t seed,
    const uint64_t key) {
    uint32_t k1, k2;

    data &= 0x0FFFFFFF;
    k1 = subghz_protocol_keeloq_common_decrypt(data, key);
    k2 = subghz_protocol_keeloq_common_decrypt(seed, key);

    return ((uint64_t)k1 << 32) | k2;
}

/** Magic_xor_type1 Learning
 * @param data - serial number (28bit)
 * @param xor - magic xor (64bit)
 * @return manufacture for this serial number (64bit)
 */

inline uint64_t
    subghz_protocol_keeloq_common_magic_xor_type1_learning(uint32_t data, uint64_t xor) {
    data &= 0x0FFFFFFF;
    return (((uint64_t)data << 32) | data) ^ xor;
}

/** Magic_serial_type1 Learning
 * @param data - serial number (28bit)
 * @param man - magic man (64bit)
 * @return manufacture for this serial number (64bit)
 */

inline uint64_t
    subghz_protocol_keeloq_common_magic_serial_type1_learning(uint32_t data, uint64_t man) {
    return (man & 0xFFFFFFFF) | ((uint64_t)data << 40) |
           ((uint64_t)(((data & 0xff) + ((data >> 8) & 0xFF)) & 0xFF) << 32);
}

/** Magic_serial_type2 Learning
 * @param data - btn+serial number (32bit)
 * @param man - magic man (64bit)
 * @return manufacture for this serial number (64bit)
 */

inline uint64_t
    subghz_protocol_keeloq_common_magic_serial_type2_learning(uint32_t data, uint64_t man) {
    uint8_t* p = (uint8_t*)&data;
    uint8_t* m = (uint8_t*)&man;
    m[7] = p[0];
    m[6] = p[1];
    m[5] = p[2];
    m[4] = p[3];
    return man;
}

/** Magic_serial_type3 Learning
 * @param data - serial number (24bit)
 * @param man - magic man (64bit)
 * @return manufacture for this serial number (64bit)
 */

inline uint64_t
    subghz_protocol_keeloq_common_magic_serial_type3_learning(uint32_t data, uint64_t man) {
    return (man & 0xFFFFFFFFFF000000) | (data & 0xFFFFFF);
}
