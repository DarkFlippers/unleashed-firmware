#pragma once

#include <lib/subghz/protocols/base.h>

#include <lib/subghz/blocks/const.h>
#include <lib/subghz/blocks/decoder.h>
#include <lib/subghz/blocks/encoder.h>
#include "ws_generic.h"
#include <lib/subghz/blocks/math.h>

#define WS_PROTOCOL_LACROSSE_TX141THBV2_NAME "TX141THBv2"

typedef struct WSProtocolDecoderLaCrosse_TX141THBv2 WSProtocolDecoderLaCrosse_TX141THBv2;
typedef struct WSProtocolEncoderLaCrosse_TX141THBv2 WSProtocolEncoderLaCrosse_TX141THBv2;

extern const SubGhzProtocolDecoder ws_protocol_lacrosse_tx141thbv2_decoder;
extern const SubGhzProtocolEncoder ws_protocol_lacrosse_tx141thbv2_encoder;
extern const SubGhzProtocol ws_protocol_lacrosse_tx141thbv2;

/**
 * Allocate WSProtocolDecoderLaCrosse_TX141THBv2.
 * @param environment Pointer to a SubGhzEnvironment instance
 * @return WSProtocolDecoderLaCrosse_TX141THBv2* pointer to a WSProtocolDecoderLaCrosse_TX141THBv2 instance
 */
void* ws_protocol_decoder_lacrosse_tx141thbv2_alloc(SubGhzEnvironment* environment);

/**
 * Free WSProtocolDecoderLaCrosse_TX141THBv2.
 * @param context Pointer to a WSProtocolDecoderLaCrosse_TX141THBv2 instance
 */
void ws_protocol_decoder_lacrosse_tx141thbv2_free(void* context);

/**
 * Reset decoder WSProtocolDecoderLaCrosse_TX141THBv2.
 * @param context Pointer to a WSProtocolDecoderLaCrosse_TX141THBv2 instance
 */
void ws_protocol_decoder_lacrosse_tx141thbv2_reset(void* context);

/**
 * Parse a raw sequence of levels and durations received from the air.
 * @param context Pointer to a WSProtocolDecoderLaCrosse_TX141THBv2 instance
 * @param level Signal level true-high false-low
 * @param duration Duration of this level in, us
 */
void ws_protocol_decoder_lacrosse_tx141thbv2_feed(void* context, bool level, uint32_t duration);

/**
 * Getting the hash sum of the last randomly received parcel.
 * @param context Pointer to a WSProtocolDecoderLaCrosse_TX141THBv2 instance
 * @return hash Hash sum
 */
uint8_t ws_protocol_decoder_lacrosse_tx141thbv2_get_hash_data(void* context);

/**
 * Serialize data WSProtocolDecoderLaCrosse_TX141THBv2.
 * @param context Pointer to a WSProtocolDecoderLaCrosse_TX141THBv2 instance
 * @param flipper_format Pointer to a FlipperFormat instance
 * @param preset The modulation on which the signal was received, SubGhzRadioPreset
 * @return status
 */
SubGhzProtocolStatus ws_protocol_decoder_lacrosse_tx141thbv2_serialize(
    void* context,
    FlipperFormat* flipper_format,
    SubGhzRadioPreset* preset);

/**
 * Deserialize data WSProtocolDecoderLaCrosse_TX141THBv2.
 * @param context Pointer to a WSProtocolDecoderLaCrosse_TX141THBv2 instance
 * @param flipper_format Pointer to a FlipperFormat instance
 * @return status
 */
SubGhzProtocolStatus ws_protocol_decoder_lacrosse_tx141thbv2_deserialize(
    void* context,
    FlipperFormat* flipper_format);

/**
 * Getting a textual representation of the received data.
 * @param context Pointer to a WSProtocolDecoderLaCrosse_TX141THBv2 instance
 * @param output Resulting text
 */
void ws_protocol_decoder_lacrosse_tx141thbv2_get_string(void* context, FuriString* output);
