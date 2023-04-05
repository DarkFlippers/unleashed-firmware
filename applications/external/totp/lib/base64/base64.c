/*
 * Base64 encoding/decoding (RFC1341)
 * Copyright (c) 2005, Jouni Malinen <j@w1.fi>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * Alternatively, this software may be distributed under the terms of BSD
 * license.
 *
 */

#include "base64.h"
#include <string.h>

static const uint8_t dtable[] = {0x3e, 0x80, 0x80, 0x80, 0x3f, 0x34, 0x35, 0x36, 0x37, 0x38,
                                 0x39, 0x3a, 0x3b, 0x3c, 0x3d, 0x80, 0x80, 0x80, 0x0,  0x80,
                                 0x80, 0x80, 0x0,  0x1,  0x2,  0x3,  0x4,  0x5,  0x6,  0x7,
                                 0x8,  0x9,  0xa,  0xb,  0xc,  0xd,  0xe,  0xf,  0x10, 0x11,
                                 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x80, 0x80,
                                 0x80, 0x80, 0x80, 0x80, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f,
                                 0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28, 0x29,
                                 0x2a, 0x2b, 0x2c, 0x2d, 0x2e, 0x2f, 0x30, 0x31, 0x32, 0x33};
// "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
static uint8_t get_dtable_value(uint8_t index) {
    return (index < 43 || index > 122) ? 0x80 : dtable[index - 43];
}

uint8_t* base64_decode(const uint8_t* src, size_t len, size_t* out_len, size_t* out_size) {
    uint8_t *out;
    uint8_t *pos;
    uint8_t in[4];
    uint8_t block[4];
    uint8_t tmp;
    size_t i;
    size_t count;
    size_t olen;

    count = 0;
    for(i = 0; i < len; i++) {
        if(get_dtable_value(src[i]) != 0x80) count++;
    }

    if(count == 0 || count % 4) return NULL;
    olen = count / 4 * 3;
    pos = out = malloc(olen);
    *out_size = olen;
    if(out == NULL) return NULL;
    count = 0;
    for(i = 0; i < len; i++) {
        tmp = get_dtable_value(src[i]);
        if(tmp == 0x80) continue;
        in[count] = src[i];
        block[count] = tmp;
        count++;
        if(count == 4) {
            *pos++ = (block[0] << 2) | (block[1] >> 4);
            *pos++ = (block[1] << 4) | (block[2] >> 2);
            *pos++ = (block[2] << 6) | block[3];
            count = 0;
        }
    }
    if(pos > out) {
        if(in[2] == '=')
            pos -= 2;
        else if(in[3] == '=')
            pos--;
    }
    *out_len = pos - out;
    return out;
}