/* Declarations of functions and data types used for SHA256 sum
   library functions.
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

#pragma once

#include <stdio.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

enum { SHA256_DIGEST_SIZE = 256 / 8 };

/* Structure to save state of computation between the single steps.  */
struct sha256_ctx {
    uint32_t state[8];

    uint32_t total[2];
    size_t buflen; /* ≥ 0, ≤ 128 */
    uint32_t buffer[32]; /* 128 bytes; the first buflen bytes are in use */
};

/* Initialize structure containing state of computation. */
extern void sha256_init_ctx(struct sha256_ctx* ctx);

/* Starting with the result of former calls of this function (or the
   initialization function update the context for the next LEN bytes
   starting at BUFFER.
   It is necessary that LEN is a multiple of 64!!! */
extern void sha256_process_block(const void* buffer, size_t len, struct sha256_ctx* ctx);

/* Starting with the result of former calls of this function (or the
   initialization function update the context for the next LEN bytes
   starting at BUFFER.
   It is NOT required that LEN is a multiple of 64.  */
extern void sha256_process_bytes(const void* buffer, size_t len, struct sha256_ctx* ctx);

/* Process the remaining bytes in the buffer and put result from CTX
   in first 32 (28) bytes following RESBUF.  The result is always in little
   endian byte order, so that a byte-wise output yields to the wanted
   ASCII representation of the message digest.  */
extern void* sha256_finish_ctx(struct sha256_ctx* ctx, void* restrict resbuf);

/* Put result from CTX in first 32 (28) bytes following RESBUF.  The result is
   always in little endian byte order, so that a byte-wise output yields
   to the wanted ASCII representation of the message digest.  */
extern void* sha256_read_ctx(const struct sha256_ctx* ctx, void* restrict resbuf);

/* Compute SHA256 message digest for LEN bytes beginning at BUFFER.
   The result is always in little endian byte order, so that a byte-wise
   output yields to the wanted ASCII representation of the message
   digest.  */
extern void* sha256_buffer(const char* buffer, size_t len, void* restrict resblock);

#ifdef __cplusplus
}
#endif

/*
 * Hey Emacs!
 * Local Variables:
 * coding: utf-8
 * End:
 */
