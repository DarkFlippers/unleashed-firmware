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
/*
  This file contains an optimized version of the MAC-calculation algorithm. Some measurements on
  a std laptop showed it runs in about 1/3 of the time:

    Std: 0.428962
    Opt: 0.151609

  Additionally, it is self-reliant, not requiring e.g. bitstreams from the cipherutils, thus can
  be easily dropped into a code base.

  The optimizations have been performed in the following steps:
  * Parameters passed by reference instead of by value.
  * Iteration instead of recursion, un-nesting recursive loops into for-loops.
  * Handling of bytes instead of individual bits, for less shuffling and masking
  * Less creation of "objects", structs, and instead reuse of alloc:ed memory
  * Inlining some functions via #define:s

  As a consequence, this implementation is less generic. Also, I haven't bothered documenting this.
  For a thorough documentation, check out the MAC-calculation within cipher.c instead.

  -- MHS 2015
**/

/**

  The runtime of opt_doTagMAC_2() with the MHS optimized version was 403 microseconds on Proxmark3.
  This was still to slow for some newer readers which didn't want to wait that long.

  Further optimizations to speedup the MAC calculations:
  * Optimized opt_Tt logic
  * Look up table for opt_select
  * Removing many unnecessary bit maskings (& 0x1)
  * updating state in place instead of alternating use of a second state structure
  * remove the necessity to reverse bits of input and output bytes

  opt_doTagMAC_2() now completes in 270 microseconds.

  -- piwi 2019
**/

/**
  add the possibility to do iCLASS on device only
  -- iceman 2020
**/

#include "optimized_cipher.h"
#include "optimized_elite.h"
#include "optimized_ikeys.h"
#include "optimized_cipherutils.h"

static const uint8_t opt_select_LUT[256] = {
    00, 03, 02, 01, 02, 03, 00, 01, 04, 07, 07, 04, 06, 07, 05, 04,
    01, 02, 03, 00, 02, 03, 00, 01, 05, 06, 06, 05, 06, 07, 05, 04,
    06, 05, 04, 07, 04, 05, 06, 07, 06, 05, 05, 06, 04, 05, 07, 06,
    07, 04, 05, 06, 04, 05, 06, 07, 07, 04, 04, 07, 04, 05, 07, 06,
    06, 05, 04, 07, 04, 05, 06, 07, 02, 01, 01, 02, 00, 01, 03, 02,
    03, 00, 01, 02, 00, 01, 02, 03, 07, 04, 04, 07, 04, 05, 07, 06,
    00, 03, 02, 01, 02, 03, 00, 01, 00, 03, 03, 00, 02, 03, 01, 00,
    05, 06, 07, 04, 06, 07, 04, 05, 05, 06, 06, 05, 06, 07, 05, 04,
    02, 01, 00, 03, 00, 01, 02, 03, 06, 05, 05, 06, 04, 05, 07, 06,
    03, 00, 01, 02, 00, 01, 02, 03, 07, 04, 04, 07, 04, 05, 07, 06,
    02, 01, 00, 03, 00, 01, 02, 03, 02, 01, 01, 02, 00, 01, 03, 02,
    03, 00, 01, 02, 00, 01, 02, 03, 03, 00, 00, 03, 00, 01, 03, 02,
    04, 07, 06, 05, 06, 07, 04, 05, 00, 03, 03, 00, 02, 03, 01, 00,
    01, 02, 03, 00, 02, 03, 00, 01, 05, 06, 06, 05, 06, 07, 05, 04,
    04, 07, 06, 05, 06, 07, 04, 05, 04, 07, 07, 04, 06, 07, 05, 04,
    01, 02, 03, 00, 02, 03, 00, 01, 01, 02, 02, 01, 02, 03, 01, 00
};

/********************** the table above has been generated with this code: ********
#include "util.h"
static void init_opt_select_LUT(void) {
    for (int r = 0; r < 256; r++) {
        uint8_t r_ls2 = r << 2;
        uint8_t r_and_ls2 = r & r_ls2;
        uint8_t r_or_ls2  = r | r_ls2;
        uint8_t z0 = (r_and_ls2 >> 5) ^ ((r & ~r_ls2) >> 4) ^ ( r_or_ls2 >> 3);
        uint8_t z1 = (r_or_ls2 >> 6) ^ ( r_or_ls2 >> 1) ^ (r >> 5) ^ r;
        uint8_t z2 = ((r & ~r_ls2) >> 4) ^ (r_and_ls2 >> 3) ^ r;
        opt_select_LUT[r] = (z0 & 4) | (z1 & 2) | (z2 & 1);
    }
    print_result("", opt_select_LUT, 256);
}
***********************************************************************************/

#define opt__select(x,y,r)  (4 & (((r & (r << 2)) >> 5) ^ ((r & ~(r << 2)) >> 4) ^ ( (r | r << 2) >> 3)))\
    |(2 & (((r | r << 2) >> 6) ^ ( (r | r << 2) >> 1) ^ (r >> 5) ^ r ^ ((x^y) << 1)))\
    |(1 & (((r & ~(r << 2)) >> 4) ^ ((r & (r << 2)) >> 3) ^ r ^ x))

/*
 * Some background on the expression above can be found here...
uint8_t xopt__select(bool x, bool y, uint8_t r)
{

    //r:      r0 r1 r2 r3 r4 r5 r6 r7
    //r_ls2:  r2 r3 r4 r5 r6 r7  0  0
    //                       z0
    //                          z1

//  uint8_t z0 = (r0 & r2) ^ (r1 & ~r3) ^ (r2 | r4); // <-- original
    uint8_t z0 = (r_and_ls2 >> 5) ^ ((r & ~r_ls2) >> 4) ^ ( r_or_ls2 >> 3);

//  uint8_t z1 = (r0 | r2) ^ ( r5 | r7) ^ r1 ^ r6 ^ x ^ y;  // <-- original
    uint8_t z1 = (r_or_ls2 >> 6) ^ ( r_or_ls2 >> 1) ^ (r >> 5) ^ r ^ ((x^y) << 1);

//  uint8_t z2 = (r3 & ~r5) ^ (r4 & r6 ) ^ r7 ^ x;  // <-- original
    uint8_t z2 = ((r & ~r_ls2) >> 4) ^ (r_and_ls2 >> 3) ^ r ^ x;

    return (z0 & 4) | (z1 & 2) | (z2 & 1);
}
*/

static void opt_successor(const uint8_t *k, State_t *s, uint8_t y) {
// #define opt_T(s) (0x1 & ((s->t >> 15) ^ (s->t >> 14) ^ (s->t >> 10) ^ (s->t >> 8) ^ (s->t >> 5) ^ (s->t >> 4)^ (s->t >> 1) ^ s->t))
    // uint8_t Tt = opt_T(s);
    uint16_t Tt = s->t & 0xc533;
    Tt = Tt ^ (Tt >> 1);
    Tt = Tt ^ (Tt >> 4);
    Tt = Tt ^ (Tt >> 10);
    Tt = Tt ^ (Tt >> 8);

    s->t = (s->t >> 1);
    s->t |= (Tt ^ (s->r >> 7) ^ (s->r >> 3)) << 15;

    uint8_t opt_B = s->b;
    opt_B ^= s->b >> 6;
    opt_B ^= s->b >> 5;
    opt_B ^= s->b >> 4;

    s->b = s->b >> 1;
    s->b |= (opt_B ^ s->r) << 7;

    uint8_t opt_select = opt_select_LUT[s->r] & 0x04;
    opt_select |= (opt_select_LUT[s->r] ^ ((Tt ^ y) << 1)) & 0x02;
    opt_select |= (opt_select_LUT[s->r] ^ Tt) & 0x01;

    uint8_t r = s->r;
    s->r = (k[opt_select] ^ s->b) + s->l ;
    s->l = s->r + r;
}

static void opt_suc(const uint8_t *k, State_t *s, const uint8_t *in, uint8_t length, bool add32Zeroes) {
    for (int i = 0; i < length; i++) {
        uint8_t head;
        head = in[i];
        opt_successor(k, s, head);

        head >>= 1;
        opt_successor(k, s, head);

        head >>= 1;
        opt_successor(k, s, head);

        head >>= 1;
        opt_successor(k, s, head);

        head >>= 1;
        opt_successor(k, s, head);

        head >>= 1;
        opt_successor(k, s, head);

        head >>= 1;
        opt_successor(k, s, head);

        head >>= 1;
        opt_successor(k, s, head);
    }
    //For tag MAC, an additional 32 zeroes
    if (add32Zeroes) {
        for (int i = 0; i < 16; i++) {
            opt_successor(k, s, 0);
            opt_successor(k, s, 0);
        }
    }
}

static void opt_output(const uint8_t *k, State_t *s,  uint8_t *buffer) {
    for (uint8_t times = 0; times < 4; times++) {
        uint8_t bout = 0;
        bout |= (s->r & 0x4) >> 2;
        opt_successor(k, s, 0);
        bout |= (s->r & 0x4) >> 1;
        opt_successor(k, s, 0);
        bout |= (s->r & 0x4);
        opt_successor(k, s, 0);
        bout |= (s->r & 0x4) << 1;
        opt_successor(k, s, 0);
        bout |= (s->r & 0x4) << 2;
        opt_successor(k, s, 0);
        bout |= (s->r & 0x4) << 3;
        opt_successor(k, s, 0);
        bout |= (s->r & 0x4) << 4;
        opt_successor(k, s, 0);
        bout |= (s->r & 0x4) << 5;
        opt_successor(k, s, 0);
        buffer[times] = bout;
    }
}

static void opt_MAC(uint8_t *k, uint8_t *input, uint8_t *out) {
    State_t _init  =  {
        ((k[0] ^ 0x4c) + 0xEC) & 0xFF,// l
        ((k[0] ^ 0x4c) + 0x21) & 0xFF,// r
        0x4c, // b
        0xE012 // t
    };

    opt_suc(k, &_init, input, 12, false);
    opt_output(k, &_init, out);
}

static void opt_MAC_N(uint8_t *k, uint8_t *input, uint8_t in_size, uint8_t *out) {
    State_t _init  =  {
        ((k[0] ^ 0x4c) + 0xEC) & 0xFF,// l
        ((k[0] ^ 0x4c) + 0x21) & 0xFF,// r
        0x4c, // b
        0xE012 // t
    };

    opt_suc(k, &_init, input, in_size, false);
    opt_output(k, &_init, out);
}

void opt_doReaderMAC(uint8_t *cc_nr_p, uint8_t *div_key_p, uint8_t mac[4]) {
    uint8_t dest [] = {0, 0, 0, 0, 0, 0, 0, 0};
    opt_MAC(div_key_p, cc_nr_p, dest);
    memcpy(mac, dest, 4);
}

void opt_doReaderMAC_2(State_t _init,  uint8_t *nr, uint8_t mac[4], const uint8_t *div_key_p) {
    opt_suc(div_key_p, &_init, nr, 4, false);
    opt_output(div_key_p, &_init, mac);
}


void doMAC_N(uint8_t *in_p, uint8_t in_size, uint8_t *div_key_p, uint8_t mac[4]) {
    uint8_t dest [] = {0, 0, 0, 0, 0, 0, 0, 0};
    opt_MAC_N(div_key_p, in_p, in_size, dest);
    memcpy(mac, dest, 4);
}

void opt_doTagMAC(uint8_t *cc_p, const uint8_t *div_key_p, uint8_t mac[4]) {
    State_t _init  =  {
        ((div_key_p[0] ^ 0x4c) + 0xEC) & 0xFF,// l
        ((div_key_p[0] ^ 0x4c) + 0x21) & 0xFF,// r
        0x4c, // b
        0xE012 // t
    };
    opt_suc(div_key_p, &_init, cc_p, 12, true);
    opt_output(div_key_p, &_init, mac);
}

/**
 * The tag MAC can be divided (both can, but no point in dividing the reader mac) into
 * two functions, since the first 8 bytes are known, we can pre-calculate the state
 * reached after feeding CC to the cipher.
 * @param cc_p
 * @param div_key_p
 * @return the cipher state
 */
State_t opt_doTagMAC_1(uint8_t *cc_p, const uint8_t *div_key_p) {
    State_t _init  =  {
        ((div_key_p[0] ^ 0x4c) + 0xEC) & 0xFF,// l
        ((div_key_p[0] ^ 0x4c) + 0x21) & 0xFF,// r
        0x4c, // b
        0xE012 // t
    };
    opt_suc(div_key_p, &_init, cc_p, 8, false);
    return _init;
}

/**
 * The second part of the tag MAC calculation, since the CC is already calculated into the state,
 * this function is fed only the NR, and internally feeds the remaining 32 0-bits to generate the tag
 * MAC response.
 * @param _init - precalculated cipher state
 * @param nr - the reader challenge
 * @param mac - where to store the MAC
 * @param div_key_p - the key to use
 */
void opt_doTagMAC_2(State_t _init,  uint8_t *nr, uint8_t mac[4], const uint8_t *div_key_p) {
    opt_suc(div_key_p, &_init, nr, 4, true);
    opt_output(div_key_p, &_init, mac);
}


void iclass_calc_div_key(uint8_t *csn, uint8_t *key, uint8_t *div_key, bool elite) {
    if (elite) {
        uint8_t keytable[128] = {0};
        uint8_t key_index[8] = {0};
        uint8_t key_sel[8] = { 0 };
        uint8_t key_sel_p[8] = { 0 };
        hash2(key, keytable);
        hash1(csn, key_index);
        for (uint8_t i = 0; i < 8 ; i++)
            key_sel[i] = keytable[key_index[i]];

        //Permute from iclass format to standard format
        permutekey_rev(key_sel, key_sel_p);
        diversifyKey(csn, key_sel_p, div_key);
    } else {
        diversifyKey(csn, key, div_key);
    }
}
