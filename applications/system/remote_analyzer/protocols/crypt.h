/**
 * @file crypt.h
 * @brief Crypt and Crypt Lite encrypted rolling-code remotes, decoder only.
 *
 * 433.92MHz OOK, Te 205us. A frame is about 15 cycles of 5*Te, a 15ms low, then one sync
 * cell at 5*Te resolution, then PWM data at 1*Te and 3*Te: long high with short low is 1,
 * the reverse is 0. The data ends where the next preamble starts, so no gap is timed out.
 *
 * The sync cell's direction is the only thing separating the two, and it also fixes the
 * length, so each decoder takes just its own and a frame can never be reported as the other:
 *
 *   Crypt      sync high 5*Te, low 10*Te  -> 56 data bits
 *   Crypt Lite sync high 10*Te, low 5*Te  -> 64 data bits
 *
 * Derived from two captures, 28 and 37 presses, ~180 and ~265 identical frames. Not one bit
 * of either frame is fixed across presses, so there is no serial, counter or button to read
 * and nothing to validate the payload against - the preamble, the sync cell and the exact
 * bit count are the whole check. The frame is reported so a remote using it is seen at all;
 * its code cannot be advanced or replayed usefully.
 *
 * The names are taken from the capture filenames, as the brand is unknown.
 */
#pragma once

#include <lib/subghz/protocols/base.h>

#define SUBGHZ_PROTOCOL_CRYPT_NAME      "Crypt"
#define SUBGHZ_PROTOCOL_CRYPT_LITE_NAME "Crypt Lite"

extern const SubGhzProtocol ra_protocol_crypt;
extern const SubGhzProtocol ra_protocol_crypt_lite;
