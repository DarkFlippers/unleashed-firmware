#pragma once

#include <lib/subghz/protocols/base.h>

#include <lib/subghz/blocks/const.h>
#include <lib/subghz/blocks/decoder.h>
#include <lib/subghz/blocks/encoder.h>
#include "ws_generic.h"
#include <lib/subghz/blocks/math.h>

#define WS_PROTOCOL_INFACTORY_NAME "inFactory-TH"

typedef struct WSProtocolDecoderInfactory WSProtocolDecoderInfactory;
typedef struct WSProtocolEncoderInfactory WSProtocolEncoderInfactory;

extern const SubGhzProtocolDecoder ws_protocol_infactory_decoder;
extern const SubGhzProtocolEncoder ws_protocol_infactory_encoder;
extern const SubGhzProtocol ws_protocol_infactory;

/**
 * Allocate WSProtocolDecoderInfactory.
 * @param environment Pointer to a SubGhzEnvironment instance
 * @return WSProtocolDecoderInfactory* pointer to a WSProtocolDecoderInfactory instance
 */
void* ws_protocol_decoder_infactory_alloc(SubGhzEnvironment* environment);

/**
 * Free WSProtocolDecoderInfactory.
 * @param context Pointer to a WSProtocolDecoderInfactory instance
 */
void ws_protocol_decoder_infactory_free(void* context);

/**
 * Reset decoder WSProtocolDecoderInfactory.
 * @param context Pointer to a WSProtocolDecoderInfactory instance
 */
void ws_protocol_decoder_infactory_reset(void* context);

/**
 * Parse a raw sequence of levels and durations received from the air.
 * @param context Pointer to a WSProtocolDecoderInfactory instance
 * @param level Signal level true-high false-low
 * @param duration Duration of this level in, us
 */
void ws_protocol_decoder_infactory_feed(void* context, bool level, uint32_t duration);

/**
 * Getting the hash sum of the last randomly received parcel.
 * @param context Pointer to a WSProtocolDecoderInfactory instance
 * @return hash Hash sum
 */
uint8_t ws_protocol_decoder_infactory_get_hash_data(void* context);

/**
 * Serialize data WSProtocolDecoderInfactory.
 * @param context Pointer to a WSProtocolDecoderInfactory instance
 * @param flipper_format Pointer to a FlipperFormat instance
 * @param preset The modulation on which the signal was received, SubGhzRadioPreset
 * @return status
 */
SubGhzProtocolStatus ws_protocol_decoder_infactory_serialize(
    void* context,
    FlipperFormat* flipper_format,
    SubGhzRadioPreset* preset);

/**
 * Deserialize data WSProtocolDecoderInfactory.
 * @param context Pointer to a WSProtocolDecoderInfactory instance
 * @param flipper_format Pointer to a FlipperFormat instance
 * @return status
 */
SubGhzProtocolStatus
    ws_protocol_decoder_infactory_deserialize(void* context, FlipperFormat* flipper_format);

/**
 * Getting a textual representation of the received data.
 * @param context Pointer to a WSProtocolDecoderInfactory instance
 * @param output Resulting text
 */
void ws_protocol_decoder_infactory_get_string(void* context, FuriString* output);
