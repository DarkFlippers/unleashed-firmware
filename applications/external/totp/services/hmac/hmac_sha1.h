#pragma once

#include <stddef.h>

#define HMAC_SHA1_RESULT_SIZE 20

/* Compute Hashed Message Authentication Code with SHA-1, over BUFFER
   data of BUFLEN bytes using the KEY of KEYLEN bytes, writing the
   output to pre-allocated 20 byte minimum RESBUF buffer.  Return 0 on
   success.  */
int hmac_sha1(const void* key, size_t keylen, const void* in, size_t inlen, void* restrict resbuf);
