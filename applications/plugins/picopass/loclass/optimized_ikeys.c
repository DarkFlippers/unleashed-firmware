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

/**
From "Dismantling iclass":
    This section describes in detail the built-in key diversification algorithm of iClass.
    Besides the obvious purpose of deriving a card key from a master key, this
    algorithm intends to circumvent weaknesses in the cipher by preventing the
    usage of certain ‘weak’ keys. In order to compute a diversified key, the iClass
    reader first encrypts the card identity id with the master key K, using single
    DES. The resulting ciphertext is then input to a function called loclass_hash0 which
    outputs the diversified key k.

    k = loclass_hash0(DES enc (id, K))

    Here the DES encryption of id with master key K outputs a cryptogram c
    of 64 bits. These 64 bits are divided as c = x, y, z [0] , . . . , z [7] ∈ F 82 × F 82 × (F 62 ) 8
    which is used as input to the loclass_hash0 function. This function introduces some
    obfuscation by performing a number of permutations, complement and modulo
    operations, see Figure 2.5. Besides that, it checks for and removes patterns like
    similar key bytes, which could produce a strong bias in the cipher. Finally, the
    output of loclass_hash0 is the diversified card key k = k [0] , . . . , k [7] ∈ (F 82 ) 8 .

**/
#include "optimized_ikeys.h"

#include <stdint.h>
#include <stdbool.h>
#include <inttypes.h>
#include <mbedtls/des.h>
#include "optimized_cipherutils.h"

static const uint8_t loclass_pi[35] = {0x0F, 0x17, 0x1B, 0x1D, 0x1E, 0x27, 0x2B, 0x2D, 0x2E,
                                       0x33, 0x35, 0x39, 0x36, 0x3A, 0x3C, 0x47, 0x4B, 0x4D,
                                       0x4E, 0x53, 0x55, 0x56, 0x59, 0x5A, 0x5C, 0x63, 0x65,
                                       0x66, 0x69, 0x6A, 0x6C, 0x71, 0x72, 0x74, 0x78};

/**
 * @brief The key diversification algorithm uses 6-bit bytes.
 * This implementation uses 64 bit uint to pack seven of them into one
 * variable. When they are there, they are placed as follows:
 * XXXX XXXX N0 .... N7, occupying the last 48 bits.
 *
 * This function picks out one from such a collection
 * @param all
 * @param n bitnumber
 * @return
 */
static uint8_t loclass_getSixBitByte(uint64_t c, int n) {
    return (c >> (42 - 6 * n)) & 0x3F;
}

/**
 * @brief Puts back a six-bit 'byte' into a uint64_t.
 * @param c buffer
 * @param z the value to place there
 * @param n bitnumber.
 */
static void loclass_pushbackSixBitByte(uint64_t* c, uint8_t z, int n) {
    //0x XXXX YYYY ZZZZ ZZZZ ZZZZ
    //             ^z0         ^z7
    //z0:  1111 1100 0000 0000

    uint64_t masked = z & 0x3F;
    uint64_t eraser = 0x3F;
    masked <<= 42 - 6 * n;
    eraser <<= 42 - 6 * n;

    //masked <<= 6*n;
    //eraser <<= 6*n;

    eraser = ~eraser;
    (*c) &= eraser;
    (*c) |= masked;
}
/**
 * @brief Swaps the z-values.
 * If the input value has format XYZ0Z1...Z7, the output will have the format
 * XYZ7Z6...Z0 instead
 * @param c
 * @return
 */
static uint64_t loclass_swapZvalues(uint64_t c) {
    uint64_t newz = 0;
    loclass_pushbackSixBitByte(&newz, loclass_getSixBitByte(c, 0), 7);
    loclass_pushbackSixBitByte(&newz, loclass_getSixBitByte(c, 1), 6);
    loclass_pushbackSixBitByte(&newz, loclass_getSixBitByte(c, 2), 5);
    loclass_pushbackSixBitByte(&newz, loclass_getSixBitByte(c, 3), 4);
    loclass_pushbackSixBitByte(&newz, loclass_getSixBitByte(c, 4), 3);
    loclass_pushbackSixBitByte(&newz, loclass_getSixBitByte(c, 5), 2);
    loclass_pushbackSixBitByte(&newz, loclass_getSixBitByte(c, 6), 1);
    loclass_pushbackSixBitByte(&newz, loclass_getSixBitByte(c, 7), 0);
    newz |= (c & 0xFFFF000000000000);
    return newz;
}

/**
* @return 4 six-bit bytes chunked into a uint64_t,as 00..00a0a1a2a3
*/
static uint64_t loclass_ck(int i, int j, uint64_t z) {
    if(i == 1 && j == -1) {
        // loclass_ck(1, −1, z [0] . . . z [3] ) = z [0] . . . z [3]
        return z;
    } else if(j == -1) {
        // loclass_ck(i, −1, z [0] . . . z [3] ) = loclass_ck(i − 1, i − 2, z [0] . . . z [3] )
        return loclass_ck(i - 1, i - 2, z);
    }

    if(loclass_getSixBitByte(z, i) == loclass_getSixBitByte(z, j)) {
        //loclass_ck(i, j − 1, z [0] . . . z [i] ← j . . . z [3] )
        uint64_t newz = 0;
        int c;
        for(c = 0; c < 4; c++) {
            uint8_t val = loclass_getSixBitByte(z, c);
            if(c == i)
                loclass_pushbackSixBitByte(&newz, j, c);
            else
                loclass_pushbackSixBitByte(&newz, val, c);
        }
        return loclass_ck(i, j - 1, newz);
    } else {
        return loclass_ck(i, j - 1, z);
    }
}
/**

    Definition 8.
    Let the function check : (F 62 ) 8 → (F 62 ) 8 be defined as
    check(z [0] . . . z [7] ) = loclass_ck(3, 2, z [0] . . . z [3] ) · loclass_ck(3, 2, z [4] . . . z [7] )

    where loclass_ck : N × N × (F 62 ) 4 → (F 62 ) 4 is defined as

        loclass_ck(1, −1, z [0] . . . z [3] ) = z [0] . . . z [3]
        loclass_ck(i, −1, z [0] . . . z [3] ) = loclass_ck(i − 1, i − 2, z [0] . . . z [3] )
        loclass_ck(i, j, z [0] . . . z [3] ) =
        loclass_ck(i, j − 1, z [0] . . . z [i] ← j . . . z [3] ),  if z [i] = z [j] ;
        loclass_ck(i, j − 1, z [0] . . . z [3] ), otherwise

    otherwise.
**/

static uint64_t loclass_check(uint64_t z) {
    //These 64 bits are divided as c = x, y, z [0] , . . . , z [7]

    // loclass_ck(3, 2, z [0] . . . z [3] )
    uint64_t ck1 = loclass_ck(3, 2, z);

    // loclass_ck(3, 2, z [4] . . . z [7] )
    uint64_t ck2 = loclass_ck(3, 2, z << 24);

    //The loclass_ck function will place the values
    // in the middle of z.
    ck1 &= 0x00000000FFFFFF000000;
    ck2 &= 0x00000000FFFFFF000000;

    return ck1 | ck2 >> 24;
}

static void loclass_permute(
    LoclassBitstreamIn_t* p_in,
    uint64_t z,
    int l,
    int r,
    LoclassBitstreamOut_t* out) {
    if(loclass_bitsLeft(p_in) == 0) return;

    bool pn = loclass_tailBit(p_in);
    if(pn) { // pn = 1
        uint8_t zl = loclass_getSixBitByte(z, l);

        loclass_push6bits(out, zl + 1);
        loclass_permute(p_in, z, l + 1, r, out);
    } else { // otherwise
        uint8_t zr = loclass_getSixBitByte(z, r);

        loclass_push6bits(out, zr);
        loclass_permute(p_in, z, l, r + 1, out);
    }
}

/**
 * @brief
 *Definition 11. Let the function loclass_hash0 : F 82 × F 82 × (F 62 ) 8 → (F 82 ) 8 be defined as
 *  loclass_hash0(x, y, z [0] . . . z [7] ) = k [0] . . . k [7] where
 * z'[i] = (z[i] mod (63-i)) + i      i =  0...3
 * z'[i+4] = (z[i+4] mod (64-i)) + i  i =  0...3
 * ẑ = check(z');
 * @param c
 * @param k this is where the diversified key is put (should be 8 bytes)
 * @return
 */
void loclass_hash0(uint64_t c, uint8_t k[8]) {
    c = loclass_swapZvalues(c);

    //These 64 bits are divided as c = x, y, z [0] , . . . , z [7]
    // x = 8 bits
    // y = 8 bits
    // z0-z7 6 bits each : 48 bits
    uint8_t x = (c & 0xFF00000000000000) >> 56;
    uint8_t y = (c & 0x00FF000000000000) >> 48;
    uint64_t zP = 0;

    for(int n = 0; n < 4; n++) {
        uint8_t zn = loclass_getSixBitByte(c, n);
        uint8_t zn4 = loclass_getSixBitByte(c, n + 4);
        uint8_t _zn = (zn % (63 - n)) + n;
        uint8_t _zn4 = (zn4 % (64 - n)) + n;
        loclass_pushbackSixBitByte(&zP, _zn, n);
        loclass_pushbackSixBitByte(&zP, _zn4, n + 4);
    }

    uint64_t zCaret = loclass_check(zP);
    uint8_t p = loclass_pi[x % 35];

    if(x & 1) //Check if x7 is 1
        p = ~p;

    LoclassBitstreamIn_t p_in = {&p, 8, 0};
    uint8_t outbuffer[] = {0, 0, 0, 0, 0, 0, 0, 0};
    LoclassBitstreamOut_t out = {outbuffer, 0, 0};
    loclass_permute(&p_in, zCaret, 0, 4, &out); //returns 48 bits? or 6 8-bytes

    //Out is now a buffer containing six-bit bytes, should be 48 bits
    // if all went well
    //Shift z-values down onto the lower segment

    uint64_t zTilde = loclass_x_bytes_to_num(outbuffer, sizeof(outbuffer));

    zTilde >>= 16;

    for(int i = 0; i < 8; i++) {
        // the key on index i is first a bit from y
        // then six bits from z,
        // then a bit from p

        // Init with zeroes
        k[i] = 0;
        // First, place yi leftmost in k
        //k[i] |= (y  << i) & 0x80 ;

        // First, place y(7-i) leftmost in k
        k[i] |= (y << (7 - i)) & 0x80;

        uint8_t zTilde_i = loclass_getSixBitByte(zTilde, i);
        // zTildeI is now on the form 00XXXXXX
        // with one leftshift, it'll be
        // 0XXXXXX0
        // So after leftshift, we can OR it into k
        // However, when doing complement, we need to
        // again MASK 0XXXXXX0 (0x7E)
        zTilde_i <<= 1;

        //Finally, add bit from p or p-mod
        //Shift bit i into rightmost location (mask only after complement)
        uint8_t p_i = p >> i & 0x1;

        if(k[i]) { // yi = 1
            k[i] |= ~zTilde_i & 0x7E;
            k[i] |= p_i & 1;
            k[i] += 1;

        } else { // otherwise
            k[i] |= zTilde_i & 0x7E;
            k[i] |= (~p_i) & 1;
        }
    }
}
/**
 * @brief Performs Elite-class key diversification
 * @param csn
 * @param key
 * @param div_key
 */
void loclass_diversifyKey(uint8_t* csn, const uint8_t* key, uint8_t* div_key) {
    mbedtls_des_context loclass_ctx_enc;

    // Prepare the DES key
    mbedtls_des_setkey_enc(&loclass_ctx_enc, key);

    uint8_t crypted_csn[8] = {0};

    // Calculate DES(CSN, KEY)
    mbedtls_des_crypt_ecb(&loclass_ctx_enc, csn, crypted_csn);

    //Calculate HASH0(DES))
    uint64_t c_csn = loclass_x_bytes_to_num(crypted_csn, sizeof(crypted_csn));

    loclass_hash0(c_csn, div_key);
}
