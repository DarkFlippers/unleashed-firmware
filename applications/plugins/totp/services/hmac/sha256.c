/* sha256.c - Functions to compute SHA256 message digest of files or
   memory blocks according to the NIST specification FIPS-180-2.

   Copyright (C) 2005-2006, 2008-2022 Free Software Foundation, Inc.

   This file is free software: you can redistribute it and/or modify
   it under the terms of the GNU Lesser General Public License as
   published by the Free Software Foundation; either version 2.1 of the
   License, or (at your option) any later version.

   This file is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public License
   along with this program.  If not, see <https://www.gnu.org/licenses/>.  */

/* Written by David Madore, considerably copypasting from
   Scott G. Miller's sha1.c
*/

/* Specification.  */
#include "sha256.h"

#include <stdint.h>
#include <string.h>

#ifdef WORDS_BIGENDIAN
#define SWAP(n) (n)
#else
#include "byteswap.h"
#define SWAP(n) swap_uint32(n)
#endif

/* This array contains the bytes used to pad the buffer to the next
   64-byte boundary.  */
static const unsigned char fillbuf[64] = {0x80, 0 /* , 0, 0, ...  */};

/*
  Takes a pointer to a 256 bit block of data (eight 32 bit ints) and
  initializes it to the start constants of the SHA256 algorithm.  This
  must be called before using hash in the call to sha256_hash
*/
void sha256_init_ctx(struct sha256_ctx* ctx) {
    ctx->state[0] = 0x6a09e667UL;
    ctx->state[1] = 0xbb67ae85UL;
    ctx->state[2] = 0x3c6ef372UL;
    ctx->state[3] = 0xa54ff53aUL;
    ctx->state[4] = 0x510e527fUL;
    ctx->state[5] = 0x9b05688cUL;
    ctx->state[6] = 0x1f83d9abUL;
    ctx->state[7] = 0x5be0cd19UL;

    ctx->total[0] = ctx->total[1] = 0;
    ctx->buflen = 0;
}

/* Copy the value from v into the memory location pointed to by *CP,
   If your architecture allows unaligned access, this is equivalent to
   * (__typeof__ (v) *) cp = v  */
static void set_uint32(char* cp, uint32_t v) {
    memcpy(cp, &v, sizeof v);
}

/* Put result from CTX in first 32 bytes following RESBUF.
   The result must be in little endian byte order.  */
void* sha256_read_ctx(const struct sha256_ctx* ctx, void* resbuf) {
    int i;
    char* r = resbuf;

    for(i = 0; i < 8; i++) set_uint32(r + i * sizeof ctx->state[0], SWAP(ctx->state[i]));

    return resbuf;
}

/* Process the remaining bytes in the internal buffer and the usual
   prolog according to the standard and write the result to RESBUF.  */
static void sha256_conclude_ctx(struct sha256_ctx* ctx) {
    /* Take yet unprocessed bytes into account.  */
    size_t bytes = ctx->buflen;
    size_t size = (bytes < 56) ? 64 / 4 : 64 * 2 / 4;

    /* Now count remaining bytes.  */
    ctx->total[0] += bytes;
    if(ctx->total[0] < bytes) ++ctx->total[1];

    /* Put the 64-bit file length in *bits* at the end of the buffer.
     Use set_uint32 rather than a simple assignment, to avoid risk of
     unaligned access.  */
    set_uint32((char*)&ctx->buffer[size - 2], SWAP((ctx->total[1] << 3) | (ctx->total[0] >> 29)));
    set_uint32((char*)&ctx->buffer[size - 1], SWAP(ctx->total[0] << 3));

    memcpy(&((char*)ctx->buffer)[bytes], fillbuf, (size - 2) * 4 - bytes);

    /* Process last bytes.  */
    sha256_process_block(ctx->buffer, size * 4, ctx);
}

void* sha256_finish_ctx(struct sha256_ctx* ctx, void* resbuf) {
    sha256_conclude_ctx(ctx);
    return sha256_read_ctx(ctx, resbuf);
}

/* Compute SHA256 message digest for LEN bytes beginning at BUFFER.  The
   result is always in little endian byte order, so that a byte-wise
   output yields to the wanted ASCII representation of the message
   digest.  */
void* sha256_buffer(const char* buffer, size_t len, void* resblock) {
    struct sha256_ctx ctx;

    /* Initialize the computation context.  */
    sha256_init_ctx(&ctx);

    /* Process whole buffer but last len % 64 bytes.  */
    sha256_process_bytes(buffer, len, &ctx);

    /* Put result in desired memory area.  */
    return sha256_finish_ctx(&ctx, resblock);
}

void sha256_process_bytes(const void* buffer, size_t len, struct sha256_ctx* ctx) {
    /* When we already have some bits in our internal buffer concatenate
     both inputs first.  */
    if(ctx->buflen != 0) {
        size_t left_over = ctx->buflen;
        size_t add = 128 - left_over > len ? len : 128 - left_over;

        memcpy(&((char*)ctx->buffer)[left_over], buffer, add);
        ctx->buflen += add;

        if(ctx->buflen > 64) {
            sha256_process_block(ctx->buffer, ctx->buflen & ~63, ctx);

            ctx->buflen &= 63;
            /* The regions in the following copy operation cannot overlap,
             because ctx->buflen < 64 ≤ (left_over + add) & ~63.  */
            memcpy(ctx->buffer, &((char*)ctx->buffer)[(left_over + add) & ~63], ctx->buflen);
        }

        buffer = (const char*)buffer + add;
        len -= add;
    }

    /* Process available complete blocks.  */
    if(len >= 64) {
#if !(_STRING_ARCH_unaligned || _STRING_INLINE_unaligned)
#define UNALIGNED_P(p) ((uintptr_t)(p) % sizeof(uint32_t) != 0)
        if(UNALIGNED_P(buffer))
            while(len > 64) {
                sha256_process_block(memcpy(ctx->buffer, buffer, 64), 64, ctx); //-V1086
                buffer = (const char*)buffer + 64;
                len -= 64;
            }
        else
#endif
        {
            sha256_process_block(buffer, len & ~63, ctx);
            buffer = (const char*)buffer + (len & ~63);
            len &= 63;
        }
    }

    /* Move remaining bytes in internal buffer.  */
    if(len > 0) {
        size_t left_over = ctx->buflen;

        memcpy(&((char*)ctx->buffer)[left_over], buffer, len);
        left_over += len;
        if(left_over >= 64) {
            sha256_process_block(ctx->buffer, 64, ctx);
            left_over -= 64;
            /* The regions in the following copy operation cannot overlap,
             because left_over ≤ 64.  */
            memcpy(ctx->buffer, &ctx->buffer[16], left_over);
        }
        ctx->buflen = left_over;
    }
}

/* --- Code below is the primary difference between sha1.c and sha256.c --- */

/* SHA256 round constants */
#define K(I) sha256_round_constants[I]
static const uint32_t sha256_round_constants[64] = {
    0x428a2f98UL, 0x71374491UL, 0xb5c0fbcfUL, 0xe9b5dba5UL, 0x3956c25bUL, 0x59f111f1UL,
    0x923f82a4UL, 0xab1c5ed5UL, 0xd807aa98UL, 0x12835b01UL, 0x243185beUL, 0x550c7dc3UL,
    0x72be5d74UL, 0x80deb1feUL, 0x9bdc06a7UL, 0xc19bf174UL, 0xe49b69c1UL, 0xefbe4786UL,
    0x0fc19dc6UL, 0x240ca1ccUL, 0x2de92c6fUL, 0x4a7484aaUL, 0x5cb0a9dcUL, 0x76f988daUL,
    0x983e5152UL, 0xa831c66dUL, 0xb00327c8UL, 0xbf597fc7UL, 0xc6e00bf3UL, 0xd5a79147UL,
    0x06ca6351UL, 0x14292967UL, 0x27b70a85UL, 0x2e1b2138UL, 0x4d2c6dfcUL, 0x53380d13UL,
    0x650a7354UL, 0x766a0abbUL, 0x81c2c92eUL, 0x92722c85UL, 0xa2bfe8a1UL, 0xa81a664bUL,
    0xc24b8b70UL, 0xc76c51a3UL, 0xd192e819UL, 0xd6990624UL, 0xf40e3585UL, 0x106aa070UL,
    0x19a4c116UL, 0x1e376c08UL, 0x2748774cUL, 0x34b0bcb5UL, 0x391c0cb3UL, 0x4ed8aa4aUL,
    0x5b9cca4fUL, 0x682e6ff3UL, 0x748f82eeUL, 0x78a5636fUL, 0x84c87814UL, 0x8cc70208UL,
    0x90befffaUL, 0xa4506cebUL, 0xbef9a3f7UL, 0xc67178f2UL,
};

/* Round functions.  */
#define F2(A, B, C) ((A & B) | (C & (A | B)))
#define F1(E, F, G) (G ^ (E & (F ^ G)))

/* Process LEN bytes of BUFFER, accumulating context into CTX.
   It is assumed that LEN % 64 == 0.
   Most of this code comes from GnuPG's cipher/sha1.c.  */

void sha256_process_block(const void* buffer, size_t len, struct sha256_ctx* ctx) {
    const uint32_t* words = buffer;
    size_t nwords = len / sizeof(uint32_t);
    const uint32_t* endp = words + nwords;
    uint32_t x[16];
    uint32_t a = ctx->state[0];
    uint32_t b = ctx->state[1];
    uint32_t c = ctx->state[2];
    uint32_t d = ctx->state[3];
    uint32_t e = ctx->state[4];
    uint32_t f = ctx->state[5];
    uint32_t g = ctx->state[6];
    uint32_t h = ctx->state[7];
    uint32_t lolen = len;

    /* First increment the byte count.  FIPS PUB 180-2 specifies the possible
     length of the file up to 2^64 bits.  Here we only compute the
     number of bytes.  Do a double word increment.  */
    ctx->total[0] += lolen;
    ctx->total[1] += (len >> 31 >> 1) + (ctx->total[0] < lolen);

#define rol(x, n) (((x) << (n)) | ((x) >> (32 - (n))))
#define S0(x) (rol(x, 25) ^ rol(x, 14) ^ (x >> 3))
#define S1(x) (rol(x, 15) ^ rol(x, 13) ^ (x >> 10))
#define SS0(x) (rol(x, 30) ^ rol(x, 19) ^ rol(x, 10))
#define SS1(x) (rol(x, 26) ^ rol(x, 21) ^ rol(x, 7))

#define M(I)                                                                                \
    (tm = S1(x[(I - 2) & 0x0f]) + x[(I - 7) & 0x0f] + S0(x[(I - 15) & 0x0f]) + x[I & 0x0f], \
     x[I & 0x0f] = tm)

#define R(A, B, C, D, E, F, G, H, K, M)        \
    do {                                       \
        t0 = SS0(A) + F2(A, B, C);             \
        t1 = H + SS1(E) + F1(E, F, G) + K + M; \
        D += t1;                               \
        H = t0 + t1;                           \
    } while(0)

    while(words < endp) {
        uint32_t tm;
        uint32_t t0, t1;
        int t;
        /* FIXME: see sha1.c for a better implementation.  */
        for(t = 0; t < 16; t++) {
            x[t] = SWAP(*words);
            words++;
        }

        for(int i = 0; i < 64; i++) {
            uint32_t xx = i < 16 ? x[i] : M(i);
            R(a, b, c, d, e, f, g, h, K(i), xx);
            uint32_t tt = a;
            a = h;
            h = g;
            g = f;
            f = e;
            e = d;
            d = c;
            c = b;
            b = tt;
        }

        a = ctx->state[0] += a;
        b = ctx->state[1] += b;
        c = ctx->state[2] += c;
        d = ctx->state[3] += d;
        e = ctx->state[4] += e;
        f = ctx->state[5] += f;
        g = ctx->state[6] += g;
        h = ctx->state[7] += h;
    }
}

/*
 * Hey Emacs!
 * Local Variables:
 * coding: utf-8
 * End:
 */
