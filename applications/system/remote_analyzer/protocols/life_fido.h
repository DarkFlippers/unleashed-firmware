/**
 * @file life_fido.h
 * @brief Life Fido remote, decoder only.
 *
 * 433.92MHz OOK, Manchester at 404us. A frame is 11 cycles of te_short, a 2638us low, one
 * half-bit belonging to the sync rather than to bit 0, 72 bits as 144 half-bits, and one
 * trailing half-bit - so it always ends on a high and can be reported as soon as the last
 * bit lands, instead of waiting for a gap that would have swallowed it.
 *
 * Derived from two remotes, 30 presses, 259 clean frames. The code is a rolling one and its
 * encryption is not reversible from captures: 35 of the 72 bits are fixed, and the other 37
 * change in about half of their positions from one press to the next. All 21 consecutive XOR
 * differences are distinct, including within a single counter parity, so the rolling part is
 * not an LFSR, a CRC, a Gray code or a fixed mask - those all repeat one difference. Without
 * the cipher and the remote's key the next code cannot be computed, which is why this lives
 * here and not in lib/subghz: nothing it could send would open anything.
 *
 * Bits 0-11 and bits 45-54 are identical in every frame of both remotes and stand in as the
 * protocol signature; that check is what stops a Manchester decoder from reporting noise,
 * since any 1T/2T stream is valid Manchester. Bits 12-13 are not part of it: one remote
 * sends 01 there and the other 10, so they look like a remote or model id.
 *
 * Which button was pressed is not readable. Across four buttons pressed twice each, the only
 * fields that hold still within a button are the ones in bits 36-51, and those take 8 to 12
 * different values over 22 presses of the other remote - a field that changes every second
 * press, not a button code. The button is inside the encrypted part.
 */
#pragma once

#include <lib/subghz/protocols/base.h>

#define SUBGHZ_PROTOCOL_LIFE_FIDO_NAME "Life Fido"

extern const SubGhzProtocol ra_protocol_life_fido;
