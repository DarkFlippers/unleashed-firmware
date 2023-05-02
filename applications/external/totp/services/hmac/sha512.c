/* sha512.c - Functions to compute SHA512 message digest of files or
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
#include "sha512.h"

#include <stdint.h>
#include <string.h>

#include "byteswap.h"
#include "sha_pad_buffer.h"

#define SWAP(n) swap_uint64(n)

/*
  Takes a pointer to a 512 bit block of data (eight 64 bit ints) and
  initializes it to the start constants of the SHA512 algorithm.  This
  must be called before using hash in the call to sha512_hash
*/
void sha512_init_ctx(struct sha512_ctx* ctx) {
    ctx->state[0] = u64hilo(0x6a09e667, 0xf3bcc908);
    ctx->state[1] = u64hilo(0xbb67ae85, 0x84caa73b);
    ctx->state[2] = u64hilo(0x3c6ef372, 0xfe94f82b);
    ctx->state[3] = u64hilo(0xa54ff53a, 0x5f1d36f1);
    ctx->state[4] = u64hilo(0x510e527f, 0xade682d1);
    ctx->state[5] = u64hilo(0x9b05688c, 0x2b3e6c1f);
    ctx->state[6] = u64hilo(0x1f83d9ab, 0xfb41bd6b);
    ctx->state[7] = u64hilo(0x5be0cd19, 0x137e2179);

    ctx->total[0] = ctx->total[1] = u64lo(0);
    ctx->buflen = 0;
}

/* Copy the value from V into the memory location pointed to by *CP,
   If your architecture allows unaligned access, this is equivalent to
   * (__typeof__ (v) *) cp = v  */
static void set_uint64(char* cp, u64 v) {
    memcpy(cp, &v, sizeof v);
}

/* Put result from CTX in first 64 bytes following RESBUF.
   The result must be in little endian byte order.  */
void* sha512_read_ctx(const struct sha512_ctx* ctx, void* resbuf) {
    int i;
    char* r = resbuf;

    for(i = 0; i < 8; i++) set_uint64(r + i * sizeof ctx->state[0], SWAP(ctx->state[i]));

    return resbuf;
}

/* Process the remaining bytes in the internal buffer and the usual
   prolog according to the standard and write the result to RESBUF.  */
static void sha512_conclude_ctx(struct sha512_ctx* ctx) {
    /* Take yet unprocessed bytes into account.  */
    size_t bytes = ctx->buflen;
    size_t size = (bytes < 112) ? 128 / 8 : 128 * 2 / 8;

    /* Now count remaining bytes.  */
    ctx->total[0] = u64plus(ctx->total[0], u64lo(bytes));
    if(u64lt(ctx->total[0], u64lo(bytes))) ctx->total[1] = u64plus(ctx->total[1], u64lo(1));

    /* Put the 128-bit file length in *bits* at the end of the buffer.
     Use set_uint64 rather than a simple assignment, to avoid risk of
     unaligned access.  */
    set_uint64(
        (char*)&ctx->buffer[size - 2],
        SWAP(u64or(u64shl(ctx->total[1], 3), u64shr(ctx->total[0], 61))));
    set_uint64((char*)&ctx->buffer[size - 1], SWAP(u64shl(ctx->total[0], 3)));

    sha_pad_buffer(&((uint8_t*)ctx->buffer)[bytes], (size - 2) * 8 - bytes);

    /* Process last bytes.  */
    sha512_process_block(ctx->buffer, size * 8, ctx);
}

void* sha512_finish_ctx(struct sha512_ctx* ctx, void* resbuf) {
    sha512_conclude_ctx(ctx);
    return sha512_read_ctx(ctx, resbuf);
}

/* Compute SHA512 message digest for LEN bytes beginning at BUFFER.  The
   result is always in little endian byte order, so that a byte-wise
   output yields to the wanted ASCII representation of the message
   digest.  */
void* sha512_buffer(const char* buffer, size_t len, void* resblock) {
    struct sha512_ctx ctx;

    /* Initialize the computation context.  */
    sha512_init_ctx(&ctx);

    /* Process whole buffer but last len % 128 bytes.  */
    sha512_process_bytes(buffer, len, &ctx);

    /* Put result in desired memory area.  */
    return sha512_finish_ctx(&ctx, resblock);
}

void sha512_process_bytes(const void* buffer, size_t len, struct sha512_ctx* ctx) {
    /* When we already have some bits in our internal buffer concatenate
     both inputs first.  */
    if(ctx->buflen != 0) {
        size_t left_over = ctx->buflen;
        size_t add = 256 - left_over > len ? len : 256 - left_over;

        memcpy(&((char*)ctx->buffer)[left_over], buffer, add);
        ctx->buflen += add;

        if(ctx->buflen > 128) {
            sha512_process_block(ctx->buffer, ctx->buflen & ~127, ctx);

            ctx->buflen &= 127;
            /* The regions in the following copy operation cannot overlap,
             because ctx->buflen < 128 ≤ (left_over + add) & ~127.  */
            memcpy(ctx->buffer, &((char*)ctx->buffer)[(left_over + add) & ~127], ctx->buflen);
        }

        buffer = (const char*)buffer + add;
        len -= add;
    }

    /* Process available complete blocks.  */
    if(len >= 128) {
#if !(_STRING_ARCH_unaligned || _STRING_INLINE_unaligned)
#define UNALIGNED_P(p) ((uintptr_t)(p) % sizeof(u64) != 0)
        if(UNALIGNED_P(buffer))
            while(len > 128) {
                sha512_process_block(memcpy(ctx->buffer, buffer, 128), 128, ctx); //-V1086
                buffer = (const char*)buffer + 128;
                len -= 128;
            }
        else
#endif
        {
            sha512_process_block(buffer, len & ~127, ctx);
            buffer = (const char*)buffer + (len & ~127);
            len &= 127;
        }
    }

    /* Move remaining bytes in internal buffer.  */
    if(len > 0) {
        size_t left_over = ctx->buflen;

        memcpy(&((char*)ctx->buffer)[left_over], buffer, len);
        left_over += len;
        if(left_over >= 128) {
            sha512_process_block(ctx->buffer, 128, ctx);
            left_over -= 128;
            /* The regions in the following copy operation cannot overlap,
             because left_over ≤ 128.  */
            memcpy(ctx->buffer, &ctx->buffer[16], left_over);
        }
        ctx->buflen = left_over;
    }
}

/* --- Code below is the primary difference between sha1.c and sha512.c --- */

/* SHA512 round constants */
#define K(I) sha512_round_constants[I]
static u64 const sha512_round_constants[80] = {
    u64init(0x428a2f98, 0xd728ae22), u64init(0x71374491, 0x23ef65cd),
    u64init(0xb5c0fbcf, 0xec4d3b2f), u64init(0xe9b5dba5, 0x8189dbbc),
    u64init(0x3956c25b, 0xf348b538), u64init(0x59f111f1, 0xb605d019),
    u64init(0x923f82a4, 0xaf194f9b), u64init(0xab1c5ed5, 0xda6d8118),
    u64init(0xd807aa98, 0xa3030242), u64init(0x12835b01, 0x45706fbe),
    u64init(0x243185be, 0x4ee4b28c), u64init(0x550c7dc3, 0xd5ffb4e2),
    u64init(0x72be5d74, 0xf27b896f), u64init(0x80deb1fe, 0x3b1696b1),
    u64init(0x9bdc06a7, 0x25c71235), u64init(0xc19bf174, 0xcf692694),
    u64init(0xe49b69c1, 0x9ef14ad2), u64init(0xefbe4786, 0x384f25e3),
    u64init(0x0fc19dc6, 0x8b8cd5b5), u64init(0x240ca1cc, 0x77ac9c65),
    u64init(0x2de92c6f, 0x592b0275), u64init(0x4a7484aa, 0x6ea6e483),
    u64init(0x5cb0a9dc, 0xbd41fbd4), u64init(0x76f988da, 0x831153b5),
    u64init(0x983e5152, 0xee66dfab), u64init(0xa831c66d, 0x2db43210),
    u64init(0xb00327c8, 0x98fb213f), u64init(0xbf597fc7, 0xbeef0ee4),
    u64init(0xc6e00bf3, 0x3da88fc2), u64init(0xd5a79147, 0x930aa725),
    u64init(0x06ca6351, 0xe003826f), u64init(0x14292967, 0x0a0e6e70),
    u64init(0x27b70a85, 0x46d22ffc), u64init(0x2e1b2138, 0x5c26c926),
    u64init(0x4d2c6dfc, 0x5ac42aed), u64init(0x53380d13, 0x9d95b3df),
    u64init(0x650a7354, 0x8baf63de), u64init(0x766a0abb, 0x3c77b2a8),
    u64init(0x81c2c92e, 0x47edaee6), u64init(0x92722c85, 0x1482353b),
    u64init(0xa2bfe8a1, 0x4cf10364), u64init(0xa81a664b, 0xbc423001),
    u64init(0xc24b8b70, 0xd0f89791), u64init(0xc76c51a3, 0x0654be30),
    u64init(0xd192e819, 0xd6ef5218), u64init(0xd6990624, 0x5565a910),
    u64init(0xf40e3585, 0x5771202a), u64init(0x106aa070, 0x32bbd1b8),
    u64init(0x19a4c116, 0xb8d2d0c8), u64init(0x1e376c08, 0x5141ab53),
    u64init(0x2748774c, 0xdf8eeb99), u64init(0x34b0bcb5, 0xe19b48a8),
    u64init(0x391c0cb3, 0xc5c95a63), u64init(0x4ed8aa4a, 0xe3418acb),
    u64init(0x5b9cca4f, 0x7763e373), u64init(0x682e6ff3, 0xd6b2b8a3),
    u64init(0x748f82ee, 0x5defb2fc), u64init(0x78a5636f, 0x43172f60),
    u64init(0x84c87814, 0xa1f0ab72), u64init(0x8cc70208, 0x1a6439ec),
    u64init(0x90befffa, 0x23631e28), u64init(0xa4506ceb, 0xde82bde9),
    u64init(0xbef9a3f7, 0xb2c67915), u64init(0xc67178f2, 0xe372532b),
    u64init(0xca273ece, 0xea26619c), u64init(0xd186b8c7, 0x21c0c207),
    u64init(0xeada7dd6, 0xcde0eb1e), u64init(0xf57d4f7f, 0xee6ed178),
    u64init(0x06f067aa, 0x72176fba), u64init(0x0a637dc5, 0xa2c898a6),
    u64init(0x113f9804, 0xbef90dae), u64init(0x1b710b35, 0x131c471b),
    u64init(0x28db77f5, 0x23047d84), u64init(0x32caab7b, 0x40c72493),
    u64init(0x3c9ebe0a, 0x15c9bebc), u64init(0x431d67c4, 0x9c100d4c),
    u64init(0x4cc5d4be, 0xcb3e42b6), u64init(0x597f299c, 0xfc657e2a),
    u64init(0x5fcb6fab, 0x3ad6faec), u64init(0x6c44198c, 0x4a475817),
};

/* Round functions.  */
#define F2(A, B, C) u64or(u64and(A, B), u64and(C, u64or(A, B)))
#define F1(E, F, G) u64xor(G, u64and(E, u64xor(F, G)))

/* Process LEN bytes of BUFFER, accumulating context into CTX.
   It is assumed that LEN % 128 == 0.
   Most of this code comes from GnuPG's cipher/sha1.c.  */

void sha512_process_block(const void* buffer, size_t len, struct sha512_ctx* ctx) {
    u64 const* words = buffer;
    u64 const* endp = words + len / sizeof(u64);
    u64 x[16];
    u64 a = ctx->state[0];
    u64 b = ctx->state[1];
    u64 c = ctx->state[2];
    u64 d = ctx->state[3];
    u64 e = ctx->state[4];
    u64 f = ctx->state[5];
    u64 g = ctx->state[6];
    u64 h = ctx->state[7];
    u64 lolen = u64size(len);

    /* First increment the byte count.  FIPS PUB 180-2 specifies the possible
     length of the file up to 2^128 bits.  Here we only compute the
     number of bytes.  Do a double word increment.  */
    ctx->total[0] = u64plus(ctx->total[0], lolen);
    ctx->total[1] = u64plus(
        ctx->total[1], u64plus(u64size(len >> 31 >> 31 >> 2), u64lo(u64lt(ctx->total[0], lolen))));

#define S0(x) u64xor(u64rol(x, 63), u64xor(u64rol(x, 56), u64shr(x, 7)))
#define S1(x) u64xor(u64rol(x, 45), u64xor(u64rol(x, 3), u64shr(x, 6)))
#define SS0(x) u64xor(u64rol(x, 36), u64xor(u64rol(x, 30), u64rol(x, 25)))
#define SS1(x) u64xor(u64rol(x, 50), u64xor(u64rol(x, 46), u64rol(x, 23)))

#define M(I)              \
    (x[(I)&15] = u64plus( \
         x[(I)&15],       \
         u64plus(S1(x[((I)-2) & 15]), u64plus(x[((I)-7) & 15], S0(x[((I)-15) & 15])))))

#define R(A, B, C, D, E, F, G, H, K, M)                                            \
    do {                                                                           \
        u64 t0 = u64plus(SS0(A), F2(A, B, C));                                     \
        u64 t1 = u64plus(H, u64plus(SS1(E), u64plus(F1(E, F, G), u64plus(K, M)))); \
        D = u64plus(D, t1);                                                        \
        H = u64plus(t0, t1);                                                       \
    } while(0)

    while(words < endp) {
        int t;
        /* FIXME: see sha1.c for a better implementation.  */
        for(t = 0; t < 16; t++) {
            x[t] = SWAP(*words);
            words++;
        }

        for(int i = 0; i < 80; i++) {
            u64 xx = i < 16 ? x[i] : M(i);
            R(a, b, c, d, e, f, g, h, K(i), xx);
            u64 tt = a;
            a = h;
            h = g;
            g = f;
            f = e;
            e = d;
            d = c;
            c = b;
            b = tt;
        }

        a = ctx->state[0] = u64plus(ctx->state[0], a);
        b = ctx->state[1] = u64plus(ctx->state[1], b);
        c = ctx->state[2] = u64plus(ctx->state[2], c);
        d = ctx->state[3] = u64plus(ctx->state[3], d);
        e = ctx->state[4] = u64plus(ctx->state[4], e);
        f = ctx->state[5] = u64plus(ctx->state[5], f);
        g = ctx->state[6] = u64plus(ctx->state[6], g);
        h = ctx->state[7] = u64plus(ctx->state[7], h);
    }
}

/*
 * Hey Emacs!
 * Local Variables:
 * coding: utf-8
 * End:
 */
