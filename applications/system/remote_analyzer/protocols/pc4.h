/**
 * @file pc4.h
 * @brief PC4 remote / DRX-2M receiver, decoder only.
 *
 * Original decoder by @xMasterX (MMX), 2025.04.
 *
 * 868.35MHz OOK. Two or three (2000, 2000)us header cells, then 56 (high, low) bit cells at
 * 400/800us - short high with long low is 1, long high with short low is 0 - then one short
 * trailing high and a 16.2ms carrier-off gap.
 *
 * Derived from three captures, 694 clean frames: every bit rolls except bits 16..19, which hold
 * still inside one capture and differ between remotes, and bits 28..31, which are nearly still.
 * 71, 40 and 57 distinct values in the three files, so there is no plaintext serial or counter
 * to split out and the whole value is shown as one key.
 */
#pragma once

#include <lib/subghz/protocols/base.h>

#define SUBGHZ_PROTOCOL_PC4_NAME "PC4"

extern const SubGhzProtocol ra_protocol_pc4;
