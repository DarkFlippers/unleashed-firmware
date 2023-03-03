#pragma once

#include <lib/subghz/protocols/base.h>

#include <lib/subghz/blocks/const.h>
#include <lib/subghz/blocks/decoder.h>
#include <lib/subghz/blocks/encoder.h>
#include "ws_generic.h"
#include <lib/subghz/blocks/math.h>

#define WS_PROTOCOL_OREGON_V1_NAME "Oregon-v1"

typedef struct WSProtocolDecoderOregon_V1 WSProtocolDecoderOregon_V1;
typedef struct WSProtocolEncoderOregon_V1 WSProtocolEncoderOregon_V1;

extern const SubGhzProtocolDecoder ws_protocol_oregon_v1_decoder;
extern const SubGhzProtocolEncoder ws_protocol_oregon_v1_encoder;
extern const SubGhzProtocol ws_protocol_oregon_v1;

/**
 * Allocate WSProtocolDecoderOregon_V1.
 * @param environment Pointer to a SubGhzEnvironment instance
 * @return WSProtocolDecoderOregon_V1* pointer to a WSProtocolDecoderOregon_V1 instance
 */
void* ws_protocol_decoder_oregon_v1_alloc(SubGhzEnvironment* environment);

/**
 * Free WSProtocolDecoderOregon_V1.
 * @param context Pointer to a WSProtocolDecoderOregon_V1 instance
 */
void ws_protocol_decoder_oregon_v1_free(void* context);

/**
 * Reset decoder WSProtocolDecoderOregon_V1.
 * @param context Pointer to a WSProtocolDecoderOregon_V1 instance
 */
void ws_protocol_decoder_oregon_v1_reset(void* context);

/**
 * Parse a raw sequence of levels and durations received from the air.
 * @param context Pointer to a WSProtocolDecoderOregon_V1 instance
 * @param level Signal level true-high false-low
 * @param duration Duration of this level in, us
 */
void ws_protocol_decoder_oregon_v1_feed(void* context, bool level, uint32_t duration);

/**
 * Getting the hash sum of the last randomly received parcel.
 * @param context Pointer to a WSProtocolDecoderOregon_V1 instance
 * @return hash Hash sum
 */
uint8_t ws_protocol_decoder_oregon_v1_get_hash_data(void* context);

/**
 * Serialize data WSProtocolDecoderOregon_V1.
 * @param context Pointer to a WSProtocolDecoderOregon_V1 instance
 * @param flipper_format Pointer to a FlipperFormat instance
 * @param preset The modulation on which the signal was received, SubGhzRadioPreset
 * @return status
 */
SubGhzProtocolStatus ws_protocol_decoder_oregon_v1_serialize(
    void* context,
    FlipperFormat* flipper_format,
    SubGhzRadioPreset* preset);

/**
 * Deserialize data WSProtocolDecoderOregon_V1.
 * @param context Pointer to a WSProtocolDecoderOregon_V1 instance
 * @param flipper_format Pointer to a FlipperFormat instance
 * @return status
 */
SubGhzProtocolStatus
    ws_protocol_decoder_oregon_v1_deserialize(void* context, FlipperFormat* flipper_format);

/**
 * Getting a textual representation of the received data.
 * @param context Pointer to a WSProtocolDecoderOregon_V1 instance
 * @param output Resulting text
 */
void ws_protocol_decoder_oregon_v1_get_string(void* context, FuriString* output);
