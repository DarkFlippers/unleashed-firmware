#pragma once

#include <lib/subghz/protocols/base.h>

#include <lib/subghz/blocks/const.h>
#include <lib/subghz/blocks/decoder.h>
#include <lib/subghz/blocks/encoder.h>
#include "ws_generic.h"
#include <lib/subghz/blocks/math.h>

#define WS_PROTOCOL_WENDOX_NAME "Wendox"

typedef struct WSProtocolDecoderWendox WSProtocolDecoderWendox;
typedef struct WSProtocolEncoderWendox WSProtocolEncoderWendox;

extern const SubGhzProtocolDecoder ws_protocol_wendox_decoder;
extern const SubGhzProtocolEncoder ws_protocol_wendox_encoder;
extern const SubGhzProtocol ws_protocol_wendox;

/**
 * Allocate WSProtocolDecoderWendox.
 * @param environment Pointer to a SubGhzEnvironment instance
 * @return WSProtocolDecoderWendox* pointer to a WSProtocolDecoderWendox instance
 */
void* ws_protocol_decoder_wendox_alloc(SubGhzEnvironment* environment);

/**
 * Free WSProtocolDecoderWendox.
 * @param context Pointer to a WSProtocolDecoderWendox instance
 */
void ws_protocol_decoder_wendox_free(void* context);

/**
 * Reset decoder WSProtocolDecoderWendox.
 * @param context Pointer to a WSProtocolDecoderWendox instance
 */
void ws_protocol_decoder_wendox_reset(void* context);

/**
 * Parse a raw sequence of levels and durations received from the air.
 * @param context Pointer to a WSProtocolDecoderWendox instance
 * @param level Signal level true-high false-low
 * @param duration Duration of this level in, us
 */
void ws_protocol_decoder_wendox_feed(void* context, bool level, uint32_t duration);

/**
 * Getting the hash sum of the last randomly received parcel.
 * @param context Pointer to a WSProtocolDecoderWendox instance
 * @return hash Hash sum
 */
uint8_t ws_protocol_decoder_wendox_get_hash_data(void* context);

/**
 * Serialize data WSProtocolDecoderWendox.
 * @param context Pointer to a WSProtocolDecoderWendox instance
 * @param flipper_format Pointer to a FlipperFormat instance
 * @param preset The modulation on which the signal was received, SubGhzRadioPreset
 * @return status
 */
SubGhzProtocolStatus ws_protocol_decoder_wendox_serialize(
    void* context,
    FlipperFormat* flipper_format,
    SubGhzRadioPreset* preset);

/**
 * Deserialize data WSProtocolDecoderWendox.
 * @param context Pointer to a WSProtocolDecoderWendox instance
 * @param flipper_format Pointer to a FlipperFormat instance
 * @return status
 */
SubGhzProtocolStatus
    ws_protocol_decoder_wendox_deserialize(void* context, FlipperFormat* flipper_format);

/**
 * Getting a textual representation of the received data.
 * @param context Pointer to a WSProtocolDecoderWendox instance
 * @param output Resulting text
 */
void ws_protocol_decoder_wendox_get_string(void* context, FuriString* output);
