/**
 * @file belgate.h
 * @brief BelGate remote, decoder only.
 *
 * 433.92MHz OOK, 119 pulses per frame between 7.7ms carrier-off headers. Not Manchester: the
 * frame duration varies while the pulse count does not, so each pulse carries one bit by its
 * own width - ~480us is 0, ~960us is 1 - on both levels alike. A frame starts and ends on a
 * high, so 60 highs and 59 lows.
 *
 * Bit layout, from the first pulse:
 *   0..15    preamble, always 0x8017
 *   16..47   serial, fixed per remote
 *   48..111  rolling payload
 *   112..117 button, one bit of six set
 *   118      trailing high, clipped about half the time, so dropped
 *
 * Derived from five captures, 2359 frames. The preamble and the one-hot button are 21 fixed
 * bits together, which is what keeps this from answering to anything but a real BelGate frame.
 */
#pragma once

#include <lib/subghz/protocols/base.h>

#define SUBGHZ_PROTOCOL_BELGATE_NAME "BelGate"

extern const SubGhzProtocol ra_protocol_belgate;
