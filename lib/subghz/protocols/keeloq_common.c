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

/** Faac SLH (Spa) Learning
 * @param seed - seed number (32bit)
 * @param key - mfkey (64bit)
 * @return man_learning for this seed number (64bit)
 */

inline uint64_t
    subghz_protocol_keeloq_common_faac_learning(const uint32_t seed, const uint64_t key) {
    uint16_t hs = seed >> 16;
    const uint16_t ending = 0x544D;
    uint32_t lsb = (uint32_t)hs << 16 | ending;
    uint64_t man = (uint64_t)subghz_protocol_keeloq_common_encrypt(seed, key) << 32 |
                   subghz_protocol_keeloq_common_encrypt(lsb, key);
    return man;
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

// Key utils

/** KeeLoq decryption whose *outer* loop length is dictated by the caller.
 * The cipher round itself still stops after the usual 528 rounds, so an outer_limit
 * above that simply spins. Used by the AERF-style learnings.
 */
static inline uint32_t subghz_protocol_keeloq_common_manufacturer_nl_extend(
    uint32_t x,
    uint32_t k_lo,
    uint32_t k_hi,
    uint32_t outer_limit) {
    const uint64_t key = ((uint64_t)k_hi << 32) | k_lo;

    for(uint32_t r = 0; r != outer_limit; r++) {
        if(r < 0x210u) {
            x = (x << 1) ^ bit(x, 31) ^ bit(x, 15) ^ (uint32_t)bit(key, (15u - r) & 63) ^
                bit(KEELOQ_NLF, g5(x, 0, 8, 19, 25, 30));
        }
    }
    return x;
}

/** Encrypt counterpart of subghz_protocol_keeloq_common_manufacturer_nl_extend().
 * Same structure: the KeeLoq round only runs for the first 528 iterations, but the
 * outer loop length is dictated by the caller.
 */
static inline uint32_t subghz_protocol_keeloq_common_manufacturer_nl_extend_encrypt(
    uint32_t x,
    uint32_t k_lo,
    uint32_t k_hi,
    uint32_t outer_limit) {
    const uint64_t key = ((uint64_t)k_hi << 32) | k_lo;

    for(uint32_t r = 0; r != outer_limit; r++) {
        if(r < 0x210u) {
            x = (x >> 1) ^ ((bit(x, 0) ^ bit(x, 16) ^ (uint32_t)bit(key, r & 63) ^
                             bit(KEELOQ_NLF, g5(x, 1, 9, 20, 26, 31)))
                            << 31);
        }
    }
    return x;
}

static inline uint32_t subghz_protocol_keeloq_common_word_rotate16(uint32_t v) {
    return (v >> 16) | (v << 16);
}

static inline uint64_t subghz_protocol_keeloq_common_bytes_to_key(const uint8_t b[8]) {
    uint64_t v = 0;
    for(uint8_t i = 8; i > 0; i--) {
        v = (v << 8) | b[i - 1];
    }
    return v;
}

inline uint32_t subghz_protocol_keeloq_common_decrypt_derived(
    uint32_t hop_encrypted,
    uint64_t derived_manufacturing_key,
    uint32_t outer_limit) {
    return subghz_protocol_keeloq_common_manufacturer_nl_extend(
        hop_encrypted,
        (uint32_t)derived_manufacturing_key,
        (uint32_t)(derived_manufacturing_key >> 32u),
        outer_limit);
}

inline uint32_t subghz_protocol_keeloq_common_encrypt_derived(
    uint32_t data,
    uint64_t derived_manufacturing_key,
    uint32_t outer_limit) {
    return subghz_protocol_keeloq_common_manufacturer_nl_extend_encrypt(
        data,
        (uint32_t)derived_manufacturing_key,
        (uint32_t)(derived_manufacturing_key >> 32u),
        outer_limit);
}

inline uint32_t subghz_protocol_keeloq_common_encrypt_rounds(
    const uint32_t data,
    const uint64_t key,
    uint32_t rounds) {
    uint32_t x = data, r;
    for(r = 0; r < rounds; r++)
        x = (x >> 1) ^ ((bit(x, 0) ^ bit(x, 16) ^ (uint32_t)bit(key, r & 63) ^
                         bit(KEELOQ_NLF, g5(x, 1, 9, 20, 26, 31)))
                        << 31);
    return x;
}

// Protocol (Manufacturer) specific learning
// TODO: Better documentation for these functions

inline uint64_t subghz_protocol_keeloq_common_learning_aerf(uint32_t data, const uint64_t key) {
    uint32_t k_lo = (uint32_t)key;
    uint32_t k_hi = (uint32_t)(key >> 32);
    uint32_t d = data & 0x0FFFFFFFu;
    uint32_t k1 = subghz_protocol_keeloq_common_manufacturer_nl_extend(
        d | 0x20000000u, k_lo, k_hi, KEELOQ_NL_EXTEND_LIMIT_AERF_DEC);
    uint32_t k2 = subghz_protocol_keeloq_common_manufacturer_nl_extend(
        d | 0x60000000u, k_lo, k_hi, KEELOQ_NL_EXTEND_LIMIT_AERF_DEC);
    /* Note: unlike normal_learning() the halves are NOT swapped here - the 0x2-prefixed
     * result is the high word. */
    return ((uint64_t)k1 << 32) | k2;
}

inline uint64_t
    subghz_protocol_keeloq_common_learning_erreka(uint32_t data, uint32_t mix, const uint64_t key) {
    uint32_t d = data & 0x0FFFFFFFu;
    uint32_t k1 = subghz_protocol_keeloq_common_decrypt(d | 0x20000000u, key);
    uint32_t r4 = mix >> 4;
    uint32_t r1 = (mix << 4) & 0xF000F000u;
    r4 = (r4 & 0x0F000F00u) | r1;
    uint32_t r5 = mix & 0x00FF00FFu;
    uint32_t x = r4 | r5;
    x |= 0x60000000u;
    uint32_t k2 = subghz_protocol_keeloq_common_decrypt(x, key);
    return ((uint64_t)k2 << 32) | k1;
}

inline uint64_t subghz_protocol_keeloq_common_learning_pujol(uint32_t data, const uint64_t key) {
    uint32_t d = data & 0x0FFFFFFFu;
    uint32_t w1 = subghz_protocol_keeloq_common_decrypt(d | 0x20000000u, key);
    uint32_t w2 = subghz_protocol_keeloq_common_decrypt(d | 0x60000000u, key);
    uint32_t k1 = subghz_protocol_keeloq_common_word_rotate16(w1);
    uint32_t k2 = subghz_protocol_keeloq_common_word_rotate16(w2);
    return ((uint64_t)k2 << 32) | k1;
}

/** JCM GEN2 Learning
 * Rebuilds an intermediate key out of the manufacture key and the fix part, runs one
 * KeeLoq decryption with it, then shuffles the result into the final manufacture key.
 * @param data - btn + serial number (32bit)
 * @param btn - button code (8bit)
 * @param key - manufacture (64bit)
 * @return manufacture for this serial number (64bit)
 */
inline uint64_t subghz_protocol_keeloq_common_learning_jcm_gen2(
    uint32_t data,
    uint8_t btn,
    const uint64_t key) {
    uint8_t k[8];
    for(uint8_t i = 0; i < 8; i++) {
        k[i] = (uint8_t)(key >> (8u * i));
    }

    const uint8_t b0 = (uint8_t)data;
    const uint8_t b1 = (uint8_t)(data >> 8);
    const uint32_t hi = data >> 16;

    k[0] ^= b0;
    k[6] ^= (uint8_t)(((data << 4) & 0xFF0u) | (b0 >> 4));
    const uint8_t mix = (uint8_t)((uint8_t)((btn << 4) | (btn >> 4)) + k[2]);
    k[2] = mix;
    k[5] = (uint8_t)(k[5] - 1u - btn);
    k[4] ^= b1;

    const uint8_t k7 = k[7];
    const uint8_t k4 = k[4];

    const uint32_t seed = (uint8_t)(btn ^ (uint8_t)(((hi << 4) & 0xFF0u) | ((hi >> 4) & 0x0Fu))) |
                          ((uint32_t)(uint8_t)~b0 << 8) |
                          ((uint32_t)(uint8_t)(b1 ^ ((data >> 24) & 0x0Fu)) << 16) |
                          ((uint32_t)(uint8_t)(((data >> 4) & 0xFF0u) | (b1 >> 4)) << 24);

    const uint32_t d =
        subghz_protocol_keeloq_common_decrypt(seed, subghz_protocol_keeloq_common_bytes_to_key(k));

    uint8_t o[8];
    o[0] = (uint8_t)(d >> 24);
    o[1] = (uint8_t)(((d & 0x0Fu) << 4) | ((d >> 4) & 0x0Fu));
    o[2] = (uint8_t)(mix + (uint8_t)(((d >> 4) & 0xFF0u) | ((d >> 12) & 0x0Fu)));
    o[3] = (uint8_t)(d >> 8);
    o[4] = (uint8_t)((uint8_t)(d >> 24) + k4 + 1u);
    o[5] = (uint8_t)(d >> 16);
    o[6] = (uint8_t)~d;
    o[7] = (uint8_t)((uint8_t)(d >> 16) + k7);

    return subghz_protocol_keeloq_common_bytes_to_key(o);
}

/** Stagnoli Learning
 * magic_xor_type1 with the top byte of the fix part forced to 0xA0 | nibble.
 * @param data - serial number (28bit)
 * @param key - magic xor (64bit)
 * @return manufacture for this serial number (64bit)
 */
inline uint64_t
    subghz_protocol_keeloq_common_learning_stagnoli(uint32_t data, const uint64_t key) {
    const uint32_t fix = (data & 0x0FFFFFFFu) | 0xA0000000u;
    return (((uint64_t)(fix ^ (uint32_t)(key >> 32u))) << 32) | (fix ^ (uint32_t)key);
}

inline uint64_t
    subghz_protocol_keeloq_common_learning_telcoma_table(uint32_t data, const uint32_t table[4]) {
    uint32_t x = data & 0x0FFFFFFFu;

    for(uint8_t i = 0; i < 32; i++) {
        x ^= table[x & 3u];
        x = (x << 1) | (x >> 31);
    }
    const uint32_t k1 = x;

    for(uint8_t i = 0; i < 32; i++) {
        x ^= table[x & 3u];
        x = (x << 1) | (x >> 31);
    }

    return ((uint64_t)k1 << 32) | x;
}

// Keystore lookups for the table driven learnings

/**
 * Pull the 4 words used by the table driven Telcoma learning out of the keystore.
 * The halves are identified by their own learning types, not by their names: a
 * KEELOQ_LEARNING_TELCOMA_TABLE_HI entry fills table[0..1] and a
 * KEELOQ_LEARNING_TELCOMA_TABLE_LO entry fills table[2..3].
 * @return true if both halves were found
 */
bool subghz_protocol_keeloq_common_get_telcoma_table(SubGhzKeystore* keystore, uint32_t table[4]) {
    bool got_hi = false;
    bool got_lo = false;
    for
        M_EACH(entry, *subghz_keystore_get_data(keystore), SubGhzKeyArray_t) {
            if(entry->type == KEELOQ_LEARNING_TELCOMA_TABLE_HI) {
                table[0] = (uint32_t)(entry->key >> 32);
                table[1] = (uint32_t)entry->key;
                got_hi = true;
            } else if(entry->type == KEELOQ_LEARNING_TELCOMA_TABLE_LO) {
                table[2] = (uint32_t)(entry->key >> 32);
                table[3] = (uint32_t)entry->key;
                got_lo = true;
            }
        }
    return got_hi && got_lo;
}
