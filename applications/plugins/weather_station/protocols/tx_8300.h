#pragma once

#include <lib/subghz/protocols/base.h>

#include <lib/subghz/blocks/const.h>
#include <lib/subghz/blocks/decoder.h>
#include <lib/subghz/blocks/encoder.h>
#include "ws_generic.h"
#include <lib/subghz/blocks/math.h>

#define WS_PROTOCOL_TX_8300_NAME "TX8300"

typedef struct WSProtocolDecoderTX_8300 WSProtocolDecoderTX_8300;
typedef struct WSProtocolEncoderTX_8300 WSProtocolEncoderTX_8300;

extern const SubGhzProtocolDecoder ws_protocol_tx_8300_decoder;
extern const SubGhzProtocolEncoder ws_protocol_tx_8300_encoder;
extern const SubGhzProtocol ws_protocol_tx_8300;

/**
 * Allocate WSProtocolDecoderTX_8300.
 * @param environment Pointer to a SubGhzEnvironment instance
 * @return WSProtocolDecoderTX_8300* pointer to a WSProtocolDecoderTX_8300 instance
 */
void* ws_protocol_decoder_tx_8300_alloc(SubGhzEnvironment* environment);

/**
 * Free WSProtocolDecoderTX_8300.
 * @param context Pointer to a WSProtocolDecoderTX_8300 instance
 */
void ws_protocol_decoder_tx_8300_free(void* context);

/**
 * Reset decoder WSProtocolDecoderTX_8300.
 * @param context Pointer to a WSProtocolDecoderTX_8300 instance
 */
void ws_protocol_decoder_tx_8300_reset(void* context);

/**
 * Parse a raw sequence of levels and durations received from the air.
 * @param context Pointer to a WSProtocolDecoderTX_8300 instance
 * @param level Signal level true-high false-low
 * @param duration Duration of this level in, us
 */
void ws_protocol_decoder_tx_8300_feed(void* context, bool level, uint32_t duration);

/**
 * Getting the hash sum of the last randomly received parcel.
 * @param context Pointer to a WSProtocolDecoderTX_8300 instance
 * @return hash Hash sum
 */
uint8_t ws_protocol_decoder_tx_8300_get_hash_data(void* context);

/**
 * Serialize data WSProtocolDecoderTX_8300.
 * @param context Pointer to a WSProtocolDecoderTX_8300 instance
 * @param flipper_format Pointer to a FlipperFormat instance
 * @param preset The modulation on which the signal was received, SubGhzRadioPreset
 * @return status
 */
SubGhzProtocolStatus ws_protocol_decoder_tx_8300_serialize(
    void* context,
    FlipperFormat* flipper_format,
    SubGhzRadioPreset* preset);

/**
 * Deserialize data WSProtocolDecoderTX_8300.
 * @param context Pointer to a WSProtocolDecoderTX_8300 instance
 * @param flipper_format Pointer to a FlipperFormat instance
 * @return status
 */
SubGhzProtocolStatus
    ws_protocol_decoder_tx_8300_deserialize(void* context, FlipperFormat* flipper_format);

/**
 * Getting a textual representation of the received data.
 * @param context Pointer to a WSProtocolDecoderTX_8300 instance
 * @param output Resulting text
 */
void ws_protocol_decoder_tx_8300_get_string(void* context, FuriString* output);
