#pragma once

#include <lib/subghz/protocols/base.h>

#include <lib/subghz/blocks/const.h>
#include <lib/subghz/blocks/decoder.h>
#include <lib/subghz/blocks/encoder.h>
#include "ws_generic.h"
#include <lib/subghz/blocks/math.h>

#define WS_PROTOCOL_THERMOPRO_TX4_NAME "ThermoPRO-TX4"

typedef struct WSProtocolDecoderThermoPRO_TX4 WSProtocolDecoderThermoPRO_TX4;
typedef struct WSProtocolEncoderThermoPRO_TX4 WSProtocolEncoderThermoPRO_TX4;

extern const SubGhzProtocolDecoder ws_protocol_thermopro_tx4_decoder;
extern const SubGhzProtocolEncoder ws_protocol_thermopro_tx4_encoder;
extern const SubGhzProtocol ws_protocol_thermopro_tx4;

/**
 * Allocate WSProtocolDecoderThermoPRO_TX4.
 * @param environment Pointer to a SubGhzEnvironment instance
 * @return WSProtocolDecoderThermoPRO_TX4* pointer to a WSProtocolDecoderThermoPRO_TX4 instance
 */
void* ws_protocol_decoder_thermopro_tx4_alloc(SubGhzEnvironment* environment);

/**
 * Free WSProtocolDecoderThermoPRO_TX4.
 * @param context Pointer to a WSProtocolDecoderThermoPRO_TX4 instance
 */
void ws_protocol_decoder_thermopro_tx4_free(void* context);

/**
 * Reset decoder WSProtocolDecoderThermoPRO_TX4.
 * @param context Pointer to a WSProtocolDecoderThermoPRO_TX4 instance
 */
void ws_protocol_decoder_thermopro_tx4_reset(void* context);

/**
 * Parse a raw sequence of levels and durations received from the air.
 * @param context Pointer to a WSProtocolDecoderThermoPRO_TX4 instance
 * @param level Signal level true-high false-low
 * @param duration Duration of this level in, us
 */
void ws_protocol_decoder_thermopro_tx4_feed(void* context, bool level, uint32_t duration);

/**
 * Getting the hash sum of the last randomly received parcel.
 * @param context Pointer to a WSProtocolDecoderThermoPRO_TX4 instance
 * @return hash Hash sum
 */
uint8_t ws_protocol_decoder_thermopro_tx4_get_hash_data(void* context);

/**
 * Serialize data WSProtocolDecoderThermoPRO_TX4.
 * @param context Pointer to a WSProtocolDecoderThermoPRO_TX4 instance
 * @param flipper_format Pointer to a FlipperFormat instance
 * @param preset The modulation on which the signal was received, SubGhzRadioPreset
 * @return status
 */
SubGhzProtocolStatus ws_protocol_decoder_thermopro_tx4_serialize(
    void* context,
    FlipperFormat* flipper_format,
    SubGhzRadioPreset* preset);

/**
 * Deserialize data WSProtocolDecoderThermoPRO_TX4.
 * @param context Pointer to a WSProtocolDecoderThermoPRO_TX4 instance
 * @param flipper_format Pointer to a FlipperFormat instance
 * @return status
 */
SubGhzProtocolStatus
    ws_protocol_decoder_thermopro_tx4_deserialize(void* context, FlipperFormat* flipper_format);

/**
 * Getting a textual representation of the received data.
 * @param context Pointer to a WSProtocolDecoderThermoPRO_TX4 instance
 * @param output Resulting text
 */
void ws_protocol_decoder_thermopro_tx4_get_string(void* context, FuriString* output);
