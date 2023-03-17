#pragma once

#include <stddef.h>

#define HMAC_SHA512_RESULT_SIZE 64

/* Compute Hashed Message Authentication Code with SHA-512, over BUFFER
   data of BUFLEN bytes using the KEY of KEYLEN bytes, writing the
   output to pre-allocated 64 byte minimum RESBUF buffer.  Return 0 on
   success.  */
int hmac_sha512(const void* key, size_t keylen, const void* in, size_t inlen, void* restrict resbuf);
