/*
 * sha256.c -- Compute SHA-256 hash
 *
 * Just for little endian architecture.
 *
 * Code taken from:
 *  http://gladman.plushost.co.uk/oldsite/cryptography_technology/sha/index.php
 *
 *  File names are sha2.c, sha2.h, brg_types.h, brg_endian.h
 *  in the archive sha2-07-01-07.zip.
 *
 * Code is modified in the style of PolarSSL API.
 *
 * See original copyright notice below.
 */
/*
 ---------------------------------------------------------------------------
 Copyright (c) 2002, Dr Brian Gladman, Worcester, UK.   All rights reserved.

 LICENSE TERMS

 The free distribution and use of this software in both source and binary
 form is allowed (with or without changes) provided that:

   1. distributions of this source code include the above copyright
      notice, this list of conditions and the following disclaimer;

   2. distributions in binary form include the above copyright
      notice, this list of conditions and the following disclaimer
      in the documentation and/or other associated materials;

   3. the copyright holder's name is not used to endorse products
      built using this software without specific written permission.

 ALTERNATIVELY, provided that this notice is retained in full, this product
 may be distributed under the terms of the GNU General Public License (GPL),
 in which case the provisions of the GPL apply INSTEAD OF those given above.

 DISCLAIMER

 This software is provided 'as is' with no explicit or implied warranties
 in respect of its properties, including, but not limited to, correctness
 and/or fitness for purpose.
 ---------------------------------------------------------------------------
 Issue Date: 01/08/2005
*/

#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include "sha256.h"

#define SHA256_MASK (SHA256_BLOCK_SIZE - 1)

static void memcpy_output_bswap32(unsigned char* dst, const uint32_t* p) {
    int i;
    uint32_t q = 0;

    for(i = 0; i < 32; i++) {
        if((i & 3) == 0) q = __builtin_bswap32(p[i >> 2]); /* bswap32 is GCC extention */
        dst[i] = q >> ((i & 3) * 8);
    }
}

#define rotr32(x, n) (((x) >> n) | ((x) << (32 - (n))))

#define ch(x, y, z) ((z) ^ ((x) & ((y) ^ (z))))
#define maj(x, y, z) (((x) & (y)) | ((z) & ((x) ^ (y))))

/* round transforms for SHA256 compression functions */
#define vf(n, i) v[((n) - (i)) & 7]

#define hf(i) (p[(i)&15] += g_1(p[((i) + 14) & 15]) + p[((i) + 9) & 15] + g_0(p[((i) + 1) & 15]))

#define v_cycle0(i)                                                               \
    p[i] = __builtin_bswap32(p[i]);                                               \
    vf(7, i) += p[i] + k_0[i] + s_1(vf(4, i)) + ch(vf(4, i), vf(5, i), vf(6, i)); \
    vf(3, i) += vf(7, i);                                                         \
    vf(7, i) += s_0(vf(0, i)) + maj(vf(0, i), vf(1, i), vf(2, i))

#define v_cycle(i, j)                                                                  \
    vf(7, i) += hf(i) + k_0[i + j] + s_1(vf(4, i)) + ch(vf(4, i), vf(5, i), vf(6, i)); \
    vf(3, i) += vf(7, i);                                                              \
    vf(7, i) += s_0(vf(0, i)) + maj(vf(0, i), vf(1, i), vf(2, i))

#define s_0(x) (rotr32((x), 2) ^ rotr32((x), 13) ^ rotr32((x), 22))
#define s_1(x) (rotr32((x), 6) ^ rotr32((x), 11) ^ rotr32((x), 25))
#define g_0(x) (rotr32((x), 7) ^ rotr32((x), 18) ^ ((x) >> 3))
#define g_1(x) (rotr32((x), 17) ^ rotr32((x), 19) ^ ((x) >> 10))
#define k_0 k256

static const uint32_t k256[64] = {
    0X428A2F98, 0X71374491, 0XB5C0FBCF, 0XE9B5DBA5, 0X3956C25B, 0X59F111F1, 0X923F82A4, 0XAB1C5ED5,
    0XD807AA98, 0X12835B01, 0X243185BE, 0X550C7DC3, 0X72BE5D74, 0X80DEB1FE, 0X9BDC06A7, 0XC19BF174,
    0XE49B69C1, 0XEFBE4786, 0X0FC19DC6, 0X240CA1CC, 0X2DE92C6F, 0X4A7484AA, 0X5CB0A9DC, 0X76F988DA,
    0X983E5152, 0XA831C66D, 0XB00327C8, 0XBF597FC7, 0XC6E00BF3, 0XD5A79147, 0X06CA6351, 0X14292967,
    0X27B70A85, 0X2E1B2138, 0X4D2C6DFC, 0X53380D13, 0X650A7354, 0X766A0ABB, 0X81C2C92E, 0X92722C85,
    0XA2BFE8A1, 0XA81A664B, 0XC24B8B70, 0XC76C51A3, 0XD192E819, 0XD6990624, 0XF40E3585, 0X106AA070,
    0X19A4C116, 0X1E376C08, 0X2748774C, 0X34B0BCB5, 0X391C0CB3, 0X4ED8AA4A, 0X5B9CCA4F, 0X682E6FF3,
    0X748F82EE, 0X78A5636F, 0X84C87814, 0X8CC70208, 0X90BEFFFA, 0XA4506CEB, 0XBEF9A3F7, 0XC67178F2,
};

void sha256_process(sha256_context* ctx) {
    uint32_t i;
    uint32_t* p = ctx->wbuf;
    uint32_t v[8];

    memcpy(v, ctx->state, 8 * sizeof(uint32_t));

    v_cycle0(0);
    v_cycle0(1);
    v_cycle0(2);
    v_cycle0(3);
    v_cycle0(4);
    v_cycle0(5);
    v_cycle0(6);
    v_cycle0(7);
    v_cycle0(8);
    v_cycle0(9);
    v_cycle0(10);
    v_cycle0(11);
    v_cycle0(12);
    v_cycle0(13);
    v_cycle0(14);
    v_cycle0(15);

    for(i = 16; i < 64; i += 16) {
        v_cycle(0, i);
        v_cycle(1, i);
        v_cycle(2, i);
        v_cycle(3, i);
        v_cycle(4, i);
        v_cycle(5, i);
        v_cycle(6, i);
        v_cycle(7, i);
        v_cycle(8, i);
        v_cycle(9, i);
        v_cycle(10, i);
        v_cycle(11, i);
        v_cycle(12, i);
        v_cycle(13, i);
        v_cycle(14, i);
        v_cycle(15, i);
    }

    ctx->state[0] += v[0];
    ctx->state[1] += v[1];
    ctx->state[2] += v[2];
    ctx->state[3] += v[3];
    ctx->state[4] += v[4];
    ctx->state[5] += v[5];
    ctx->state[6] += v[6];
    ctx->state[7] += v[7];
}

void sha256_update(sha256_context* ctx, const unsigned char* input, unsigned int ilen) {
    uint32_t left = (ctx->total[0] & SHA256_MASK);
    uint32_t fill = SHA256_BLOCK_SIZE - left;

    ctx->total[0] += ilen;
    if(ctx->total[0] < ilen) ctx->total[1]++;

    while(ilen >= fill) {
        memcpy(((unsigned char*)ctx->wbuf) + left, input, fill);
        sha256_process(ctx);
        input += fill;
        ilen -= fill;
        left = 0;
        fill = SHA256_BLOCK_SIZE;
    }

    memcpy(((unsigned char*)ctx->wbuf) + left, input, ilen);
}

void sha256_finish(sha256_context* ctx, unsigned char output[32]) {
    uint32_t last = (ctx->total[0] & SHA256_MASK);

    ctx->wbuf[last >> 2] = __builtin_bswap32(ctx->wbuf[last >> 2]);
    ctx->wbuf[last >> 2] &= 0xffffff80UL << (8 * (~last & 3));
    ctx->wbuf[last >> 2] |= 0x00000080UL << (8 * (~last & 3));
    ctx->wbuf[last >> 2] = __builtin_bswap32(ctx->wbuf[last >> 2]);

    if(last > SHA256_BLOCK_SIZE - 9) {
        if(last < 60) ctx->wbuf[15] = 0;
        sha256_process(ctx);
        last = 0;
    } else
        last = (last >> 2) + 1;

    while(last < 14) ctx->wbuf[last++] = 0;

    ctx->wbuf[14] = __builtin_bswap32((ctx->total[0] >> 29) | (ctx->total[1] << 3));
    ctx->wbuf[15] = __builtin_bswap32(ctx->total[0] << 3);
    sha256_process(ctx);

    memcpy_output_bswap32(output, ctx->state);
    memset(ctx, 0, sizeof(sha256_context));
}

static const uint32_t initial_state[8] = {
    0x6a09e667,
    0xbb67ae85,
    0x3c6ef372,
    0xa54ff53a,
    0x510e527f,
    0x9b05688c,
    0x1f83d9ab,
    0x5be0cd19};

void sha256_start(sha256_context* ctx) {
    ctx->total[0] = ctx->total[1] = 0;
    memcpy(ctx->state, initial_state, 8 * sizeof(uint32_t));
}

void sha256(const unsigned char* input, unsigned int ilen, unsigned char output[32]) {
    sha256_context ctx;

    sha256_start(&ctx);
    sha256_update(&ctx, input, ilen);
    sha256_finish(&ctx, output);
}
