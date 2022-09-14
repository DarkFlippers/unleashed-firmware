//-----------------------------------------------------------------------------
// Borrowed initially from https://github.com/holiman/loclass
// Copyright (C) 2014 Martin Holst Swende
// Copyright (C) Proxmark3 contributors. See AUTHORS.md for details.
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// See LICENSE.txt for the text of the license.
//-----------------------------------------------------------------------------
// WARNING
//
// THIS CODE IS CREATED FOR EXPERIMENTATION AND EDUCATIONAL USE ONLY.
//
// USAGE OF THIS CODE IN OTHER WAYS MAY INFRINGE UPON THE INTELLECTUAL
// PROPERTY OF OTHER PARTIES, SUCH AS INSIDE SECURE AND HID GLOBAL,
// AND MAY EXPOSE YOU TO AN INFRINGEMENT ACTION FROM THOSE PARTIES.
//
// THIS CODE SHOULD NEVER BE USED TO INFRINGE PATENTS OR INTELLECTUAL PROPERTY RIGHTS.
//-----------------------------------------------------------------------------
// It is a reconstruction of the cipher engine used in iClass, and RFID techology.
//
// The implementation is based on the work performed by
// Flavio D. Garcia, Gerhard de Koning Gans, Roel Verdult and
// Milosch Meriac in the paper "Dismantling IClass".
//-----------------------------------------------------------------------------
#include "optimized_cipherutils.h"
#include <stdint.h>

/**
 *
 * @brief Return and remove the first bit (x0) in the stream : <x0 x1 x2 x3 ... xn >
 * @param stream
 * @return
 */
bool loclass_headBit(LoclassBitstreamIn_t* stream) {
    int bytepos = stream->position >> 3; // divide by 8
    int bitpos = (stream->position++) & 7; // mask out 00000111
    return (*(stream->buffer + bytepos) >> (7 - bitpos)) & 1;
}
/**
 * @brief Return and remove the last bit (xn) in the stream: <x0 x1 x2 ... xn>
 * @param stream
 * @return
 */
bool loclass_tailBit(LoclassBitstreamIn_t* stream) {
    int bitpos = stream->numbits - 1 - (stream->position++);

    int bytepos = bitpos >> 3;
    bitpos &= 7;
    return (*(stream->buffer + bytepos) >> (7 - bitpos)) & 1;
}
/**
 * @brief Pushes bit onto the stream
 * @param stream
 * @param bit
 */
void loclass_pushBit(LoclassBitstreamOut_t* stream, bool bit) {
    int bytepos = stream->position >> 3; // divide by 8
    int bitpos = stream->position & 7;
    *(stream->buffer + bytepos) |= (bit) << (7 - bitpos);
    stream->position++;
    stream->numbits++;
}

/**
 * @brief Pushes the lower six bits onto the stream
 * as b0 b1 b2 b3 b4 b5 b6
 * @param stream
 * @param bits
 */
void loclass_push6bits(LoclassBitstreamOut_t* stream, uint8_t bits) {
    loclass_pushBit(stream, bits & 0x20);
    loclass_pushBit(stream, bits & 0x10);
    loclass_pushBit(stream, bits & 0x08);
    loclass_pushBit(stream, bits & 0x04);
    loclass_pushBit(stream, bits & 0x02);
    loclass_pushBit(stream, bits & 0x01);
}

/**
 * @brief loclass_bitsLeft
 * @param stream
 * @return number of bits left in stream
 */
int loclass_bitsLeft(LoclassBitstreamIn_t* stream) {
    return stream->numbits - stream->position;
}
/**
 * @brief numBits
 * @param stream
 * @return Number of bits stored in stream
 */
void loclass_x_num_to_bytes(uint64_t n, size_t len, uint8_t* dest) {
    while(len--) {
        dest[len] = (uint8_t)n;
        n >>= 8;
    }
}

uint64_t loclass_x_bytes_to_num(uint8_t* src, size_t len) {
    uint64_t num = 0;
    while(len--) {
        num = (num << 8) | (*src);
        src++;
    }
    return num;
}

uint8_t loclass_reversebytes(uint8_t b) {
    b = (b & 0xF0) >> 4 | (b & 0x0F) << 4;
    b = (b & 0xCC) >> 2 | (b & 0x33) << 2;
    b = (b & 0xAA) >> 1 | (b & 0x55) << 1;
    return b;
}

void loclass_reverse_arraybytes(uint8_t* arr, size_t len) {
    uint8_t i;
    for(i = 0; i < len; i++) {
        arr[i] = loclass_reversebytes(arr[i]);
    }
}

void loclass_reverse_arraycopy(uint8_t* arr, uint8_t* dest, size_t len) {
    uint8_t i;
    for(i = 0; i < len; i++) {
        dest[i] = loclass_reversebytes(arr[i]);
    }
}
