//-----------------------------------------------------------------------------
// Borrowed initially from https://github.com/holiman/loclass
// Copyright (C) 2014 Martin Holst Swende
// Copyright (C) Proxmark3 contributors. See AUTHORS.md for details.
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// See LICENSE.txt for the text of the license.
//-----------------------------------------------------------------------------
// WARNING
//
// THIS CODE IS CREATED FOR EXPERIMENTATION AND EDUCATIONAL USE ONLY.
//
// USAGE OF THIS CODE IN OTHER WAYS MAY INFRINGE UPON THE INTELLECTUAL
// PROPERTY OF OTHER PARTIES, SUCH AS INSIDE SECURE AND HID GLOBAL,
// AND MAY EXPOSE YOU TO AN INFRINGEMENT ACTION FROM THOSE PARTIES.
//
// THIS CODE SHOULD NEVER BE USED TO INFRINGE PATENTS OR INTELLECTUAL PROPERTY RIGHTS.
//-----------------------------------------------------------------------------
// It is a reconstruction of the cipher engine used in iClass, and RFID techology.
//
// The implementation is based on the work performed by
// Flavio D. Garcia, Gerhard de Koning Gans, Roel Verdult and
// Milosch Meriac in the paper "Dismantling IClass".
//-----------------------------------------------------------------------------
#include "optimized_elite.h"

#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <mbedtls/des.h>
#include "optimized_ikeys.h"

/**
 * @brief Permutes a key from standard NIST format to Iclass specific format
 *  from http://www.proxmark.org/forum/viewtopic.php?pid=11220#p11220
 *
 *  If you loclass_permute [6c 8d 44 f9 2a 2d 01 bf]  you get  [8a 0d b9 88 bb a7 90 ea]  as shown below.
 *
 *  1 0 1 1 1 1 1 1  bf
 *  0 0 0 0 0 0 0 1  01
 *  0 0 1 0 1 1 0 1  2d
 *  0 0 1 0 1 0 1 0  2a
 *  1 1 1 1 1 0 0 1  f9
 *  0 1 0 0 0 1 0 0  44
 *  1 0 0 0 1 1 0 1  8d
 *  0 1 1 0 1 1 0 0  6c
 *
 *  8 0 b 8 b a 9 e
 *  a d 9 8 b 7 0 a
 *
 * @param key
 * @param dest
 */
void loclass_permutekey(const uint8_t key[8], uint8_t dest[8]) {
    int i;
    for(i = 0; i < 8; i++) {
        dest[i] = (((key[7] & (0x80 >> i)) >> (7 - i)) << 7) |
                  (((key[6] & (0x80 >> i)) >> (7 - i)) << 6) |
                  (((key[5] & (0x80 >> i)) >> (7 - i)) << 5) |
                  (((key[4] & (0x80 >> i)) >> (7 - i)) << 4) |
                  (((key[3] & (0x80 >> i)) >> (7 - i)) << 3) |
                  (((key[2] & (0x80 >> i)) >> (7 - i)) << 2) |
                  (((key[1] & (0x80 >> i)) >> (7 - i)) << 1) |
                  (((key[0] & (0x80 >> i)) >> (7 - i)) << 0);
    }
}
/**
 * Permutes  a key from iclass specific format to NIST format
 * @brief loclass_permutekey_rev
 * @param key
 * @param dest
 */
void loclass_permutekey_rev(const uint8_t key[8], uint8_t dest[8]) {
    int i;
    for(i = 0; i < 8; i++) {
        dest[7 - i] = (((key[0] & (0x80 >> i)) >> (7 - i)) << 7) |
                      (((key[1] & (0x80 >> i)) >> (7 - i)) << 6) |
                      (((key[2] & (0x80 >> i)) >> (7 - i)) << 5) |
                      (((key[3] & (0x80 >> i)) >> (7 - i)) << 4) |
                      (((key[4] & (0x80 >> i)) >> (7 - i)) << 3) |
                      (((key[5] & (0x80 >> i)) >> (7 - i)) << 2) |
                      (((key[6] & (0x80 >> i)) >> (7 - i)) << 1) |
                      (((key[7] & (0x80 >> i)) >> (7 - i)) << 0);
    }
}

/**
 * Helper function for loclass_hash1
 * @brief loclass_rr
 * @param val
 * @return
 */
static uint8_t loclass_rr(uint8_t val) {
    return val >> 1 | ((val & 1) << 7);
}

/**
 * Helper function for loclass_hash1
 * @brief rl
 * @param val
 * @return
 */
static uint8_t loclass_rl(uint8_t val) {
    return val << 1 | ((val & 0x80) >> 7);
}

/**
 * Helper function for loclass_hash1
 * @brief loclass_swap
 * @param val
 * @return
 */
static uint8_t loclass_swap(uint8_t val) {
    return ((val >> 4) & 0xFF) | ((val & 0xFF) << 4);
}

/**
 * Hash1 takes CSN as input, and determines what bytes in the keytable will be used
 * when constructing the K_sel.
 * @param csn the CSN used
 * @param k output
 */
void loclass_hash1(const uint8_t csn[], uint8_t k[]) {
    k[0] = csn[0] ^ csn[1] ^ csn[2] ^ csn[3] ^ csn[4] ^ csn[5] ^ csn[6] ^ csn[7];
    k[1] = csn[0] + csn[1] + csn[2] + csn[3] + csn[4] + csn[5] + csn[6] + csn[7];
    k[2] = loclass_rr(loclass_swap(csn[2] + k[1]));
    k[3] = loclass_rl(loclass_swap(csn[3] + k[0]));
    k[4] = ~loclass_rr(csn[4] + k[2]) + 1;
    k[5] = ~loclass_rl(csn[5] + k[3]) + 1;
    k[6] = loclass_rr(csn[6] + (k[4] ^ 0x3c));
    k[7] = loclass_rl(csn[7] + (k[5] ^ 0xc3));

    k[7] &= 0x7F;
    k[6] &= 0x7F;
    k[5] &= 0x7F;
    k[4] &= 0x7F;
    k[3] &= 0x7F;
    k[2] &= 0x7F;
    k[1] &= 0x7F;
    k[0] &= 0x7F;
}
/**
Definition 14. Define the rotate key function loclass_rk : (F 82 ) 8 × N → (F 82 ) 8 as
loclass_rk(x [0] . . . x [7] , 0) = x [0] . . . x [7]
loclass_rk(x [0] . . . x [7] , n + 1) = loclass_rk(loclass_rl(x [0] ) . . . loclass_rl(x [7] ), n)
**/
static void loclass_rk(uint8_t* key, uint8_t n, uint8_t* outp_key) {
    memcpy(outp_key, key, 8);
    uint8_t j;
    while(n-- > 0) {
        for(j = 0; j < 8; j++) outp_key[j] = loclass_rl(outp_key[j]);
    }
    return;
}

static mbedtls_des_context loclass_ctx_enc;
static mbedtls_des_context loclass_ctx_dec;

static void loclass_desdecrypt_iclass(uint8_t* iclass_key, uint8_t* input, uint8_t* output) {
    uint8_t key_std_format[8] = {0};
    loclass_permutekey_rev(iclass_key, key_std_format);
    mbedtls_des_setkey_dec(&loclass_ctx_dec, key_std_format);
    mbedtls_des_crypt_ecb(&loclass_ctx_dec, input, output);
}

static void loclass_desencrypt_iclass(uint8_t* iclass_key, uint8_t* input, uint8_t* output) {
    uint8_t key_std_format[8] = {0};
    loclass_permutekey_rev(iclass_key, key_std_format);
    mbedtls_des_setkey_enc(&loclass_ctx_enc, key_std_format);
    mbedtls_des_crypt_ecb(&loclass_ctx_enc, input, output);
}

/**
 * @brief Insert uint8_t[8] custom master key to calculate hash2 and return key_select.
 * @param key unpermuted custom key
 * @param loclass_hash1 loclass_hash1
 * @param key_sel output key_sel=h[loclass_hash1[i]]
 */
void hash2(uint8_t* key64, uint8_t* outp_keytable) {
    /**
     *Expected:
     * High Security Key Table

    00  F1 35 59 A1 0D 5A 26 7F 18 60 0B 96 8A C0 25 C1
    10  BF A1 3B B0 FF 85 28 75 F2 1F C6 8F 0E 74 8F 21
    20  14 7A 55 16 C8 A9 7D B3 13 0C 5D C9 31 8D A9 B2
    30  A3 56 83 0F 55 7E DE 45 71 21 D2 6D C1 57 1C 9C
    40  78 2F 64 51 42 7B 64 30 FA 26 51 76 D3 E0 FB B6
    50  31 9F BF 2F 7E 4F 94 B4 BD 4F 75 91 E3 1B EB 42
    60  3F 88 6F B8 6C 2C 93 0D 69 2C D5 20 3C C1 61 95
    70  43 08 A0 2F FE B3 26 D7 98 0B 34 7B 47 70 A0 AB

    **** The 64-bit HS Custom Key Value = 5B7C62C491C11B39 ******/
    uint8_t key64_negated[8] = {0};
    uint8_t z[8][8] = {{0}, {0}};
    uint8_t temp_output[8] = {0};

    //calculate complement of key
    int i;
    for(i = 0; i < 8; i++) key64_negated[i] = ~key64[i];

    // Once again, key is on iclass-format
    loclass_desencrypt_iclass(key64, key64_negated, z[0]);

    uint8_t y[8][8] = {{0}, {0}};

    // y[0]=DES_dec(z[0],~key)
    // Once again, key is on iclass-format
    loclass_desdecrypt_iclass(z[0], key64_negated, y[0]);

    for(i = 1; i < 8; i++) {
        loclass_rk(key64, i, temp_output);
        loclass_desdecrypt_iclass(temp_output, z[i - 1], z[i]);
        loclass_desencrypt_iclass(temp_output, y[i - 1], y[i]);
    }

    if(outp_keytable != NULL) {
        for(i = 0; i < 8; i++) {
            memcpy(outp_keytable + i * 16, y[i], 8);
            memcpy(outp_keytable + 8 + i * 16, z[i], 8);
        }
    }
}
