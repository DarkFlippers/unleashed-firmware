/** @file lfrfid_casi_format.h
 *
 * Casi-Rusco 40-bit (C10106) badges. They transmit a plain EM4100 frame, so the firmware
 * reads them as EM4100; this reads the badge id out of those 40 bits and packs one back in.
 */

#pragma once

#include <furi.h>

#ifdef __cplusplus
extern "C" {
#endif

/** Largest value of the two 19-bit fields */
#define LFRFID_CASI_FIELD_MAX      (0x7FFFF)
/** Largest credential, the first six digits of the badge id */
#define LFRFID_CASI_CREDENTIAL_MAX LFRFID_CASI_FIELD_MAX
/** What the access system subtracts from a card field with its top bit set */
#define LFRFID_CASI_CARD_OFFSET    (66606)
/** Largest card number, the last six digits of the badge id */
#define LFRFID_CASI_CARD_MAX       (LFRFID_CASI_FIELD_MAX - LFRFID_CASI_CARD_OFFSET)

/** Pack a badge id, credential up to LFRFID_CASI_CREDENTIAL_MAX and card number up to
 * LFRFID_CASI_CARD_MAX, into the 5 bytes of EM4100 data. */
void lfrfid_casi_format_encode(uint32_t credential, uint32_t card, uint8_t* data);

/** Append a "C10106: credential card" line when the 5 bytes of EM4100 data read as a badge.
 *
 * A badge is an ordinary EM4100 frame, so this is one possible reading of the bits, shown
 * alongside the EM4100 one.
 */
void lfrfid_casi_format_render(const uint8_t* data, FuriString* result);

#ifdef __cplusplus
}
#endif
