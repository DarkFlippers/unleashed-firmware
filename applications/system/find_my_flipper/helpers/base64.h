/*
 * Base64 encoding/decoding (RFC1341)
 * Copyright (c) 2005, Jouni Malinen <j@w1.fi>
 *
 * This software may be distributed under the terms of the BSD license.
 * See README for more details.
 */
// https://web.mit.edu/freebsd/head/contrib/wpa/src/utils/base64.h

#ifndef BASE64_H
#define BASE64_H

#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

unsigned char* base64_encode(const unsigned char* src, size_t len, size_t* out_len);
unsigned char* base64_decode(const unsigned char* src, size_t len, size_t* out_len);

#endif /* BASE64_H */
