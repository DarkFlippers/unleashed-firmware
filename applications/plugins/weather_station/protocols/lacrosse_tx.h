#pragma once

#include <lib/subghz/protocols/base.h>

#include <lib/subghz/blocks/const.h>
#include <lib/subghz/blocks/decoder.h>
#include <lib/subghz/blocks/encoder.h>
#include "ws_generic.h"
#include <lib/subghz/blocks/math.h>

#define WS_PROTOCOL_LACROSSE_TX_NAME "LaCrosse_TX"

typedef struct WSProtocolDecoderLaCrosse_TX WSProtocolDecoderLaCrosse_TX;
typedef struct WSProtocolEncoderLaCrosse_TX WSProtocolEncoderLaCrosse_TX;

extern const SubGhzProtocolDecoder ws_protocol_lacrosse_tx_decoder;
extern const SubGhzProtocolEncoder ws_protocol_lacrosse_tx_encoder;
extern const SubGhzProtocol ws_protocol_lacrosse_tx;

/**
 * Allocate WSProtocolDecoderLaCrosse_TX.
 * @param environment Pointer to a SubGhzEnvironment instance
 * @return WSProtocolDecoderLaCrosse_TX* pointer to a WSProtocolDecoderLaCrosse_TX instance
 */
void* ws_protocol_decoder_lacrosse_tx_alloc(SubGhzEnvironment* environment);

/**
 * Free WSProtocolDecoderLaCrosse_TX.
 * @param context Pointer to a WSProtocolDecoderLaCrosse_TX instance
 */
void ws_protocol_decoder_lacrosse_tx_free(void* context);

/**
 * Reset decoder WSProtocolDecoderLaCrosse_TX.
 * @param context Pointer to a WSProtocolDecoderLaCrosse_TX instance
 */
void ws_protocol_decoder_lacrosse_tx_reset(void* context);

/**
 * Parse a raw sequence of levels and durations received from the air.
 * @param context Pointer to a WSProtocolDecoderLaCrosse_TX instance
 * @param level Signal level true-high false-low
 * @param duration Duration of this level in, us
 */
void ws_protocol_decoder_lacrosse_tx_feed(void* context, bool level, uint32_t duration);

/**
 * Getting the hash sum of the last randomly received parcel.
 * @param context Pointer to a WSProtocolDecoderLaCrosse_TX instance
 * @return hash Hash sum
 */
uint8_t ws_protocol_decoder_lacrosse_tx_get_hash_data(void* context);

/**
 * Serialize data WSProtocolDecoderLaCrosse_TX.
 * @param context Pointer to a WSProtocolDecoderLaCrosse_TX instance
 * @param flipper_format Pointer to a FlipperFormat instance
 * @param preset The modulation on which the signal was received, SubGhzRadioPreset
 * @return true On success
 */
bool ws_protocol_decoder_lacrosse_tx_serialize(
    void* context,
    FlipperFormat* flipper_format,
    SubGhzRadioPreset* preset);

/**
 * Deserialize data WSProtocolDecoderLaCrosse_TX.
 * @param context Pointer to a WSProtocolDecoderLaCrosse_TX instance
 * @param flipper_format Pointer to a FlipperFormat instance
 * @return true On success
 */
bool ws_protocol_decoder_lacrosse_tx_deserialize(void* context, FlipperFormat* flipper_format);

/**
 * Getting a textual representation of the received data.
 * @param context Pointer to a WSProtocolDecoderLaCrosse_TX instance
 * @param output Resulting text
 */
void ws_protocol_decoder_lacrosse_tx_get_string(void* context, FuriString* output);
