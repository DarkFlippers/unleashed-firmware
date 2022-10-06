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
#ifndef OPTIMIZED_CIPHER_H
#define OPTIMIZED_CIPHER_H
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>

/**
* Definition 1 (Cipher state). A cipher state of iClass s is an element of F 40/2
* consisting of the following four components:
*   1. the left register l = (l 0 . . . l 7 ) ∈ F 8/2 ;
*   2. the right register r = (r 0 . . . r 7 ) ∈ F 8/2 ;
*   3. the top register t = (t 0 . . . t 15 ) ∈ F 16/2 .
*   4. the bottom register b = (b 0 . . . b 7 ) ∈ F 8/2 .
**/
typedef struct {
    uint8_t l;
    uint8_t r;
    uint8_t b;
    uint16_t t;
} LoclassState_t;

/** The reader MAC is MAC(key, CC * NR )
 **/
void loclass_opt_doReaderMAC(uint8_t* cc_nr_p, uint8_t* div_key_p, uint8_t mac[4]);

void loclass_opt_doReaderMAC_2(
    LoclassState_t _init,
    uint8_t* nr,
    uint8_t mac[4],
    const uint8_t* div_key_p);

/**
 * The tag MAC is MAC(key, CC * NR * 32x0))
 */
void loclass_opt_doTagMAC(uint8_t* cc_p, const uint8_t* div_key_p, uint8_t mac[4]);

/**
 * The tag MAC can be divided (both can, but no point in dividing the reader mac) into
 * two functions, since the first 8 bytes are known, we can pre-calculate the state
 * reached after feeding CC to the cipher.
 * @param cc_p
 * @param div_key_p
 * @return the cipher state
 */
LoclassState_t loclass_opt_doTagMAC_1(uint8_t* cc_p, const uint8_t* div_key_p);
/**
 * The second part of the tag MAC calculation, since the CC is already calculated into the state,
 * this function is fed only the NR, and internally feeds the remaining 32 0-bits to generate the tag
 * MAC response.
 * @param _init - precalculated cipher state
 * @param nr - the reader challenge
 * @param mac - where to store the MAC
 * @param div_key_p - the key to use
 */
void loclass_opt_doTagMAC_2(
    LoclassState_t _init,
    uint8_t* nr,
    uint8_t mac[4],
    const uint8_t* div_key_p);

void loclass_doMAC_N(uint8_t* in_p, uint8_t in_size, uint8_t* div_key_p, uint8_t mac[4]);
void loclass_iclass_calc_div_key(uint8_t* csn, uint8_t* key, uint8_t* div_key, bool elite);
#endif // OPTIMIZED_CIPHER_H
