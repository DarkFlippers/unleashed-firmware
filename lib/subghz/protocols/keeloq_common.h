#pragma once

#include "base.h"

#include <furi.h>

/*
 * Keeloq
 * https://ru.wikipedia.org/wiki/KeeLoq
 * https://phreakerclub.com/forum/showthread.php?t=1094
 *
 */
#define KEELOQ_NLF 0x3A5C742E

/*
 * KeeLoq learning types
 * https://phreakerclub.com/forum/showthread.php?t=67
 */
#define KEELOQ_LEARNING_UNKNOWN 0u
#define KEELOQ_LEARNING_SIMPLE 1u
#define KEELOQ_LEARNING_NORMAL 2u
#define KEELOQ_LEARNING_SECURE 3u
#define KEELOQ_LEARNING_MAGIC_XOR_TYPE_1 4u

/**
 * Simple Learning Encrypt
 * @param data - 0xBSSSCCCC, B(4bit) key, S(10bit) serial&0x3FF, C(16bit) counter
 * @param key - manufacture (64bit)
 * @return keeloq encrypt data
 */
uint32_t subghz_protocol_keeloq_common_encrypt(const uint32_t data, const uint64_t key);

/** 
 * Simple Learning Decrypt
 * @param data - keeloq encrypt data
 * @param key - manufacture (64bit)
 * @return 0xBSSSCCCC, B(4bit) key, S(10bit) serial&0x3FF, C(16bit) counter
 */
uint32_t subghz_protocol_keeloq_common_decrypt(const uint32_t data, const uint64_t key);

/** 
 * Normal Learning
 * @param data - serial number (28bit)
 * @param key - manufacture (64bit)
 * @return manufacture for this serial number (64bit)
 */
uint64_t subghz_protocol_keeloq_common_normal_learning(uint32_t data, const uint64_t key);

/** 
 * Secure Learning
 * @param data - serial number (28bit)
 * @param seed - seed number (32bit)
 * @param key - manufacture (64bit)
 * @return manufacture for this serial number (64bit)
 */
uint64_t
    subghz_protocol_keeloq_common_secure_learning(uint32_t data, uint32_t seed, const uint64_t key);

/** 
 * Magic_xor_type1 Learning
 * @param data - serial number (28bit)
 * @param xor - magic xor (64bit)
 * @return manufacture for this serial number (64bit)
 */
uint64_t subghz_protocol_keeloq_common_magic_xor_type1_learning(uint32_t data, uint64_t xor);
