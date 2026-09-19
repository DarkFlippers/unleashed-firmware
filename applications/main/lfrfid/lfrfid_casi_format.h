/** @file lfrfid_casi_format.h
 *
 * Casi-Rusco 40-bit (C10106) badges. They transmit a plain EM4100 frame at RF/32, so the
 * firmware reads them as EM4100; this reads the badge id out of those 40 bits and packs one
 * back in.
 * The id is printed as twelve digits: a credential that begins with 15, then a card number.
 */

#pragma once

#include <furi.h>

#ifdef __cplusplus
extern "C" {
#endif

/** Bits in the credential and in the card field of the frame */
#define LFRFID_CASI_FIELD_SIZE     (19)
/** Smallest credential, the first six digits of the badge id begin with 15 */
#define LFRFID_CASI_CREDENTIAL_MIN (150000)
/** Largest credential */
#define LFRFID_CASI_CREDENTIAL_MAX (159999)
/** What the access system subtracts from a card field with its top bit set */
#define LFRFID_CASI_CARD_OFFSET    (66606)
/** Largest card number, the last six digits of the badge id */
#define LFRFID_CASI_CARD_MAX       ((1UL << LFRFID_CASI_FIELD_SIZE) - 1 - LFRFID_CASI_CARD_OFFSET)

/** Pack a badge id into the 5 bytes of EM4100 data. The credential must lie within
 * LFRFID_CASI_CREDENTIAL_MIN..LFRFID_CASI_CREDENTIAL_MAX and the card number must not
 * exceed LFRFID_CASI_CARD_MAX. */
void lfrfid_casi_format_encode(uint32_t credential, uint32_t card, uint8_t* data);

/** Append an "or C10106: credential card" line when the 5 bytes of EM4100 data read as a badge.
 *
 * A badge is an ordinary EM4100 frame, so this is one possible reading of the bits, shown
 * alongside the EM4100 one.
 */
void lfrfid_casi_format_render(const uint8_t* data, FuriString* result);

#ifdef __cplusplus
}
#endif
