//-----------------------------------------------------------------------------
// Borrowed initially from https://github.com/holiman/loclass
// More recently from https://github.com/RfidResearchGroup/proxmark3
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
#ifndef CIPHERUTILS_H
#define CIPHERUTILS_H
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>

typedef struct {
    uint8_t *buffer;
    uint8_t numbits;
    uint8_t position;
} BitstreamIn_t;

typedef struct {
    uint8_t *buffer;
    uint8_t numbits;
    uint8_t position;
} BitstreamOut_t;

bool headBit(BitstreamIn_t *stream);
bool tailBit(BitstreamIn_t *stream);
void pushBit(BitstreamOut_t *stream, bool bit);
int bitsLeft(BitstreamIn_t *stream);

void push6bits(BitstreamOut_t *stream, uint8_t bits);
void x_num_to_bytes(uint64_t n, size_t len, uint8_t *dest);
uint64_t x_bytes_to_num(uint8_t *src, size_t len);
uint8_t reversebytes(uint8_t b);
void reverse_arraybytes(uint8_t *arr, size_t len);
void reverse_arraycopy(uint8_t *arr, uint8_t *dest, size_t len);
#endif // CIPHERUTILS_H
