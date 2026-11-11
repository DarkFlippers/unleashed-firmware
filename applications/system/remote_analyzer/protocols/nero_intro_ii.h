/**
 * @file nero_intro_ii.h
 * @brief NERO Intro II remote, decoder only.
 *
 * Line code and detection work by Alexey Ashurok (aotd1), from the experimental protocol in
 * DarkFlippers/unleashed-firmware PR #1190. This is a decoder-only wrapper around it.
 *
 * 434.42MHz OOK, a different line format from Nero Radio. ~125us chips: a synchronisation
 * section followed by 15 symbols, each a 63-chip codeword plus a nominal `01` separator.
 * Codewords are cyclic phases of a degree-6 maximal-length sequence or its complement,
 * recurrence b[n] = b[n-5] XOR b[n-6].
 *
 * Each symbol is labelled with its six-bit initial state plus an inversion bit, giving 105
 * canonical bits - the first 64 shown as Key and the remaining 41 as Data. These are lossless
 * line-code labels, not an identified serial, button, counter or key, so nothing is split out
 * as a named field.
 *
 * Decoder and line-code core ported from the firmware's experimental protocol
 * (DarkFlippers/unleashed-firmware PR #1190), encoder and save/load left out.
 */
#pragma once

#include <lib/subghz/protocols/base.h>

#define SUBGHZ_PROTOCOL_NERO_INTRO_II_NAME "Nero Intro II"

extern const SubGhzProtocol ra_protocol_nero_intro_ii;
