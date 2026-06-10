#pragma once

#include "base.h"

/*
 * Cardin "S500-series" 868 MHz rolling-code remote (FCC ID TXQ508), as used on
 * Cardin gate/garage receivers. DECODE + DISPLAY ONLY — there is intentionally
 * no encoder, "Add Manually", or TX path (see below).
 *
 * 868.35 MHz, 2-FSK (~12 kHz deviation, read with the FM12K preset
 * FuriHalSubGhzPreset2FSKDev12KAsync). Manchester / biphase line code,
 * half-bit element Te ~= 100us, bit cell ~= 200us; on the air durations are
 * cleanly ~Te or ~2*Te.
 *
 * Frame per button press:
 *   - long radio-quiet idle (~50ms),
 *   - a wakeup burst (~25ms of long, unequal ~1100-1700us pulses) then a run of
 *     constant Manchester bits (a preamble) — neither is decoded; they just
 *     establish bit timing/phase,
 *   - the codeword, repeated while the button is held.
 * The decoder gates on the 12-bit sync, not on the wakeup burst: on a real
 * FM12K capture the burst is too jittery to gate on reliably, while the sync
 * plus the exact 140-bit length is a strong, low-false-positive frame marker.
 * Codeword: 140 bits = 12-bit fixed sync (110011110100) + 128-bit payload.
 *   The payload is a fully rolling 128-bit AES-128 ciphertext: every one of the
 *   128 bits changes between presses; there is NO cleartext serial, button or
 *   counter (verified across many presses). Button and serial live INSIDE the
 *   encrypted block. We therefore store the 128-bit payload opaquely and never
 *   attempt to parse fields out of it.
 *
 * WHY DECODE-ONLY: the rolling payload is AES-128 encrypted and the key lives
 * in the receiver, so valid future codes cannot be generated. Emulation is
 * impossible and is deliberately not implemented (the encoder is a NULL stub).
 *
 * NAMING: the remote is a Cardin S508 (S500-series, FCC ID TXQ508; the
 * 2-channel S504 uses the same protocol). It is the 868 MHz rolling-code
 * sibling of the 433.92 MHz Cardin S449 (FSK KeeLoq, decoded by the keeloq
 * protocol); see issue #908. It is NOT the older fixed-code Cardin S435/S437
 * family, the 27 MHz S466, nor Telcoma/Cardin EDGE. No public bit-level spec
 * exists; the model was reverse-engineered from clean captures and cross-checked
 * two independent ways.
 */
#define SUBGHZ_PROTOCOL_CARDIN_S508_NAME "Cardin S508"

typedef struct SubGhzProtocolDecoderCardinS508 SubGhzProtocolDecoderCardinS508;
typedef struct SubGhzProtocolEncoderCardinS508 SubGhzProtocolEncoderCardinS508;

extern const SubGhzProtocolDecoder subghz_protocol_cardin_s508_decoder;
extern const SubGhzProtocolEncoder subghz_protocol_cardin_s508_encoder;
extern const SubGhzProtocol subghz_protocol_cardin_s508;

/**
 * Allocate SubGhzProtocolDecoderCardinS508.
 * @param environment Pointer to a SubGhzEnvironment instance
 * @return SubGhzProtocolDecoderCardinS508* pointer to a SubGhzProtocolDecoderCardinS508 instance
 */
void* subghz_protocol_decoder_cardin_s508_alloc(SubGhzEnvironment* environment);

/**
 * Free SubGhzProtocolDecoderCardinS508.
 * @param context Pointer to a SubGhzProtocolDecoderCardinS508 instance
 */
void subghz_protocol_decoder_cardin_s508_free(void* context);

/**
 * Reset decoder SubGhzProtocolDecoderCardinS508.
 * @param context Pointer to a SubGhzProtocolDecoderCardinS508 instance
 */
void subghz_protocol_decoder_cardin_s508_reset(void* context);

/**
 * Parse a raw sequence of levels and durations received from the air.
 * @param context Pointer to a SubGhzProtocolDecoderCardinS508 instance
 * @param level Signal level true-high false-low
 * @param duration Duration of this level in, us
 */
void subghz_protocol_decoder_cardin_s508_feed(void* context, bool level, uint32_t duration);

/**
 * Getting the hash sum of the last randomly received parcel.
 * @param context Pointer to a SubGhzProtocolDecoderCardinS508 instance
 * @return hash Hash sum
 */
uint8_t subghz_protocol_decoder_cardin_s508_get_hash_data(void* context);

/**
 * Serialize data SubGhzProtocolDecoderCardinS508.
 * @param context Pointer to a SubGhzProtocolDecoderCardinS508 instance
 * @param flipper_format Pointer to a FlipperFormat instance
 * @param preset The modulation on which the signal was received, SubGhzRadioPreset
 * @return status
 */
SubGhzProtocolStatus subghz_protocol_decoder_cardin_s508_serialize(
    void* context,
    FlipperFormat* flipper_format,
    SubGhzRadioPreset* preset);

/**
 * Deserialize data SubGhzProtocolDecoderCardinS508.
 * @param context Pointer to a SubGhzProtocolDecoderCardinS508 instance
 * @param flipper_format Pointer to a FlipperFormat instance
 * @return status
 */
SubGhzProtocolStatus
    subghz_protocol_decoder_cardin_s508_deserialize(void* context, FlipperFormat* flipper_format);

/**
 * Getting a textual representation of the received data.
 * @param context Pointer to a SubGhzProtocolDecoderCardinS508 instance
 * @param output Resulting text
 */
void subghz_protocol_decoder_cardin_s508_get_string(void* context, FuriString* output);
