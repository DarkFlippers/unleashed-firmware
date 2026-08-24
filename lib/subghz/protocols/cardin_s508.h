#pragma once

#include "base.h"
#include "public_api.h"

/*
 * Cardin "S500-series" 868 MHz rolling-code remote (FCC ID TXQ508), as used on
 * Cardin gate/garage receivers. The 2-channel S504 uses the same framing.
 *
 * Protocol and waveform provenance:
 * - original decoder and real RAW capture: upstream PR #1004:
 *   https://github.com/DarkFlippers/unleashed-firmware/pull/1004
 * - Cardin official software download (CARDINTX_SW):
 *   https://www.cardin.it/en/assistenza/software-download
 * - CARDINTX_SW V1.43 archive used to inspect DecCardin.dll exports:
 *   https://www.cardin.it/admin/software/download/2/CARDINTX_SW-V143
 * - Cardin S504/S508 manual:
 *   https://www.cardin.it/Attachment/zvl544_03.pdf
 *
 * 868.35 MHz, 2-FSK, read with the FM12K preset
 * FuriHalSubGhzPreset2FSKDev12KAsync. Manchester / biphase line code,
 * half-bit Te ~= 100 us, bit cell ~= 200 us; the captured signal has
 * asymmetric HIGH/LOW pulse widths, so the decoder uses per-level timing
 * bands. A frame contains a 12-bit fixed sync (110011110100) followed by a
 * 128-bit opaque rolling payload.
 *
 * The decoder stores the payload without trying to extract a serial, button or
 * counter. The encoder supports both replaying a saved payload and generating
 * a payload from the 128-bit transmitter key plus the 32-bit rolling counter.
 * The Counter stored in a rolling file is the current/last value: normal
 * transmissions advance it using the global Sub-GHz counter step (normally
 * +1) before generating the next payload. An explicit counter override is
 * sent unchanged and becomes the new saved value.
 * A key cannot be recovered from an ordinary RF capture, so the generator
 * still requires the key from an authorized transmitter/receiver installation.
 * Real receiver validation is required because the on-air frame does not expose
 * the receiver's acceptance window.
 *
 * The generator is reconstructed from the exported CalcKeyS500 and
 * CalcKeyS500_VB6 routines in the official CARDINTX_SW V1.43 DecCardin.dll.
 * The binary is not redistributed with the firmware; this source contains the
 * compatible arithmetic and the byte ordering used by the VB6 adapter.
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
 * @return SubGhzProtocolDecoderCardinS508* pointer to a decoder instance
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
 * @param duration Duration of this level in us
 */
void subghz_protocol_decoder_cardin_s508_feed(void* context, bool level, uint32_t duration);

/**
 * Get the hash sum of the last received parcel.
 * @param context Pointer to a SubGhzProtocolDecoderCardinS508 instance
 * @return hash Hash sum
 */
uint8_t subghz_protocol_decoder_cardin_s508_get_hash_data(void* context);

/**
 * Serialize data from SubGhzProtocolDecoderCardinS508.
 * @param context Pointer to a SubGhzProtocolDecoderCardinS508 instance
 * @param flipper_format Pointer to a FlipperFormat instance
 * @param preset The modulation on which the signal was received
 * @return status
 */
SubGhzProtocolStatus subghz_protocol_decoder_cardin_s508_serialize(
    void* context,
    FlipperFormat* flipper_format,
    SubGhzRadioPreset* preset);

/**
 * Deserialize data into SubGhzProtocolDecoderCardinS508.
 * @param context Pointer to a SubGhzProtocolDecoderCardinS508 instance
 * @param flipper_format Pointer to a FlipperFormat instance
 * @return status
 */
SubGhzProtocolStatus
    subghz_protocol_decoder_cardin_s508_deserialize(void* context, FlipperFormat* flipper_format);

/**
 * Get a textual representation of the received data.
 * @param context Pointer to a SubGhzProtocolDecoderCardinS508 instance
 * @param output Resulting text
 */
void subghz_protocol_decoder_cardin_s508_get_string(void* context, FuriString* output);

/**
 * Allocate the Cardin S508 encoder (saved-payload replay and rolling-code generation).
 * @param environment Pointer to a SubGhzEnvironment instance
 * @return SubGhzProtocolEncoderCardinS508* pointer to an encoder instance
 */
void* subghz_protocol_encoder_cardin_s508_alloc(SubGhzEnvironment* environment);

/**
 * Free the Cardin S508 encoder.
 * @param context Pointer to a SubGhzProtocolEncoderCardinS508 instance
 */
void subghz_protocol_encoder_cardin_s508_free(void* context);

/**
 * Load a Cardin payload or rolling-code input and construct the measured RF upload.
 * @param context Pointer to a SubGhzProtocolEncoderCardinS508 instance
 * @param flipper_format Pointer to a FlipperFormat instance
 * @return status
 */
SubGhzProtocolStatus
    subghz_protocol_encoder_cardin_s508_deserialize(void* context, FlipperFormat* flipper_format);

/**
 * Stop the encoder.
 * @param context Pointer to a SubGhzProtocolEncoderCardinS508 instance
 */
void subghz_protocol_encoder_cardin_s508_stop(void* context);

/**
 * Yield the next measured RF pulse.
 * @param context Pointer to a SubGhzProtocolEncoderCardinS508 instance
 * @return next level/duration pair, or reset when transmission is complete
 */
LevelDuration subghz_protocol_encoder_cardin_s508_yield(void* context);
