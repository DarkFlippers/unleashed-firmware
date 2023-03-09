/* sha1.c - Functions to compute SHA1 message digest of files or
   memory blocks according to the NIST specification FIPS-180-1.

   Copyright (C) 2000-2001, 2003-2006, 2008-2022 Free Software Foundation, Inc.

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

/* Written by Scott G. Miller
   Credits:
      Robert Klep <robert@ilse.nl>  -- Expansion function fix
*/

/* Specification.  */
#include "sha1.h"

#include <stdint.h>
#include <string.h>

#ifdef WORDS_BIGENDIAN
#define SWAP(n) (n)
#else
#include "byteswap.h"
#define SWAP(n) swap_uint32(n)
#endif

/* This array contains the bytes used to pad the buffer to the next
   64-byte boundary.  (RFC 1321, 3.1: Step 1)  */
static const unsigned char fillbuf[64] = {0x80, 0 /* , 0, 0, ...  */};

/* Take a pointer to a 160 bit block of data (five 32 bit ints) and
   initialize it to the start constants of the SHA1 algorithm.  This
   must be called before using hash in the call to sha1_hash.  */
void sha1_init_ctx(struct sha1_ctx* ctx) {
    ctx->A = 0x67452301;
    ctx->B = 0xefcdab89;
    ctx->C = 0x98badcfe;
    ctx->D = 0x10325476;
    ctx->E = 0xc3d2e1f0;

    ctx->total[0] = ctx->total[1] = 0;
    ctx->buflen = 0;
}

/* Copy the 4 byte value from v into the memory location pointed to by *cp,
   If your architecture allows unaligned access this is equivalent to
   * (uint32_t *) cp = v  */
static void set_uint32(char* cp, uint32_t v) {
    memcpy(cp, &v, sizeof v);
}

/* Put result from CTX in first 20 bytes following RESBUF.  The result
   must be in little endian byte order.  */
void* sha1_read_ctx(const struct sha1_ctx* ctx, void* resbuf) {
    char* r = resbuf;
    set_uint32(r + 0 * sizeof ctx->A, SWAP(ctx->A));
    set_uint32(r + 1 * sizeof ctx->B, SWAP(ctx->B));
    set_uint32(r + 2 * sizeof ctx->C, SWAP(ctx->C));
    set_uint32(r + 3 * sizeof ctx->D, SWAP(ctx->D));
    set_uint32(r + 4 * sizeof ctx->E, SWAP(ctx->E));

    return resbuf;
}

/* Process the remaining bytes in the internal buffer and the usual
   prolog according to the standard and write the result to RESBUF.  */
void* sha1_finish_ctx(struct sha1_ctx* ctx, void* resbuf) {
    /* Take yet unprocessed bytes into account.  */
    uint32_t bytes = ctx->buflen;
    size_t size = (bytes < 56) ? 64 / 4 : 64 * 2 / 4;

    /* Now count remaining bytes.  */
    ctx->total[0] += bytes;
    if(ctx->total[0] < bytes) ++ctx->total[1];

    /* Put the 64-bit file length in *bits* at the end of the buffer.  */
    ctx->buffer[size - 2] = SWAP((ctx->total[1] << 3) | (ctx->total[0] >> 29));
    ctx->buffer[size - 1] = SWAP(ctx->total[0] << 3);

    memcpy(&((char*)ctx->buffer)[bytes], fillbuf, (size - 2) * 4 - bytes);

    /* Process last bytes.  */
    sha1_process_block(ctx->buffer, size * 4, ctx);

    return sha1_read_ctx(ctx, resbuf);
}

/* Compute SHA1 message digest for LEN bytes beginning at BUFFER.  The
   result is always in little endian byte order, so that a byte-wise
   output yields to the wanted ASCII representation of the message
   digest.  */
void* sha1_buffer(const char* buffer, size_t len, void* resblock) {
    struct sha1_ctx ctx;

    /* Initialize the computation context.  */
    sha1_init_ctx(&ctx);

    /* Process whole buffer but last len % 64 bytes.  */
    sha1_process_bytes(buffer, len, &ctx);

    /* Put result in desired memory area.  */
    return sha1_finish_ctx(&ctx, resblock);
}

void sha1_process_bytes(const void* buffer, size_t len, struct sha1_ctx* ctx) {
    /* When we already have some bits in our internal buffer concatenate
     both inputs first.  */
    if(ctx->buflen != 0) {
        size_t left_over = ctx->buflen;
        size_t add = 128 - left_over > len ? len : 128 - left_over;

        memcpy(&((char*)ctx->buffer)[left_over], buffer, add);
        ctx->buflen += add;

        if(ctx->buflen > 64) {
            sha1_process_block(ctx->buffer, ctx->buflen & ~63, ctx);

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
                sha1_process_block(memcpy(ctx->buffer, buffer, 64), 64, ctx); //-V1086
                buffer = (const char*)buffer + 64;
                len -= 64;
            }
        else
#endif
        {
            sha1_process_block(buffer, len & ~63, ctx);
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
            sha1_process_block(ctx->buffer, 64, ctx);
            left_over -= 64;
            /* The regions in the following copy operation cannot overlap,
             because left_over ≤ 64.  */
            memcpy(ctx->buffer, &ctx->buffer[16], left_over);
        }
        ctx->buflen = left_over;
    }
}

/* --- Code below is the primary difference between md5.c and sha1.c --- */

/* SHA1 round constants */
static const int sha1_round_constants[4] = {0x5a827999, 0x6ed9eba1, 0x8f1bbcdc, 0xca62c1d6};

/* Round functions.  Note that F2 is the same as F4.  */
#define F1(B, C, D) (D ^ (B & (C ^ D)))
#define F2_4(B, C, D) (B ^ C ^ D)
#define F3(B, C, D) ((B & C) | (D & (B | C)))
#define FN(I, B, C, D) (I == 0 ? F1(B, C, D) : (I == 2 ? F3(B, C, D) : F2_4(B, C, D)))

/* Process LEN bytes of BUFFER, accumulating context into CTX.
   It is assumed that LEN % 64 == 0.
   Most of this code comes from GnuPG's cipher/sha1.c.  */

void sha1_process_block(const void* buffer, size_t len, struct sha1_ctx* ctx) {
    const uint32_t* words = buffer;
    size_t nwords = len / sizeof(uint32_t);
    const uint32_t* endp = words + nwords;
    uint32_t x[16];
    uint32_t a = ctx->A;
    uint32_t b = ctx->B;
    uint32_t c = ctx->C;
    uint32_t d = ctx->D;
    uint32_t e = ctx->E;
    uint32_t lolen = len;

    /* First increment the byte count.  RFC 1321 specifies the possible
     length of the file up to 2^64 bits.  Here we only compute the
     number of bytes.  Do a double word increment.  */
    ctx->total[0] += lolen;
    ctx->total[1] += (len >> 31 >> 1) + (ctx->total[0] < lolen);

#define rol(x, n) (((x) << (n)) | ((uint32_t)(x) >> (32 - (n))))

#define M(I)                                                                        \
    (tm = x[I & 0x0f] ^ x[(I - 14) & 0x0f] ^ x[(I - 8) & 0x0f] ^ x[(I - 3) & 0x0f], \
     (x[I & 0x0f] = rol(tm, 1)))

#define R(A, B, C, D, E, F, K, M, KI)            \
    do {                                         \
        E += rol(A, 5) + F(KI, B, C, D) + K + M; \
        B = rol(B, 30);                          \
    } while(0)

    while(words < endp) {
        uint32_t tm;
        int t;
        for(t = 0; t < 16; t++) {
            x[t] = SWAP(*words);
            words++;
        }

        for(uint8_t i = 0; i < 80; i++) {
            uint32_t m = i < 16 ? x[i] : M(i);
            uint8_t ki = i / 20;
            int k_const = sha1_round_constants[ki];
            R(a, b, c, d, e, FN, k_const, m, ki);
            uint32_t tt = a;
            a = e;
            e = d;
            d = c;
            c = b;
            b = tt;
        }

        a = ctx->A += a;
        b = ctx->B += b;
        c = ctx->C += c;
        d = ctx->D += d;
        e = ctx->E += e;
    }
}

/*
 * Hey Emacs!
 * Local Variables:
 * coding: utf-8
 * End:
 */
