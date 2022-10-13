#pragma once

#include <stddef.h>

#define HMAC_SHA256_RESULT_SIZE 32

/* Compute Hashed Message Authentication Code with SHA-256, over BUFFER
   data of BUFLEN bytes using the KEY of KEYLEN bytes, writing the
   output to pre-allocated 32 byte minimum RESBUF buffer.  Return 0 on
   success.  */
int hmac_sha256(const void* key, size_t keylen, const void* in, size_t inlen, void* restrict resbuf);
