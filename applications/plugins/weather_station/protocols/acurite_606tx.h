#pragma once

#include <lib/subghz/protocols/base.h>

#include <lib/subghz/blocks/const.h>
#include <lib/subghz/blocks/decoder.h>
#include <lib/subghz/blocks/encoder.h>
#include "ws_generic.h"
#include <lib/subghz/blocks/math.h>

#define WS_PROTOCOL_ACURITE_606TX_NAME "Acurite-606TX"

typedef struct WSProtocolDecoderAcurite_606TX WSProtocolDecoderAcurite_606TX;
typedef struct WSProtocolEncoderAcurite_606TX WSProtocolEncoderAcurite_606TX;

extern const SubGhzProtocolDecoder ws_protocol_acurite_606tx_decoder;
extern const SubGhzProtocolEncoder ws_protocol_acurite_606tx_encoder;
extern const SubGhzProtocol ws_protocol_acurite_606tx;

/**
 * Allocate WSProtocolDecoderAcurite_606TX.
 * @param environment Pointer to a SubGhzEnvironment instance
 * @return WSProtocolDecoderAcurite_606TX* pointer to a WSProtocolDecoderAcurite_606TX instance
 */
void* ws_protocol_decoder_acurite_606tx_alloc(SubGhzEnvironment* environment);

/**
 * Free WSProtocolDecoderAcurite_606TX.
 * @param context Pointer to a WSProtocolDecoderAcurite_606TX instance
 */
void ws_protocol_decoder_acurite_606tx_free(void* context);

/**
 * Reset decoder WSProtocolDecoderAcurite_606TX.
 * @param context Pointer to a WSProtocolDecoderAcurite_606TX instance
 */
void ws_protocol_decoder_acurite_606tx_reset(void* context);

/**
 * Parse a raw sequence of levels and durations received from the air.
 * @param context Pointer to a WSProtocolDecoderAcurite_606TX instance
 * @param level Signal level true-high false-low
 * @param duration Duration of this level in, us
 */
void ws_protocol_decoder_acurite_606tx_feed(void* context, bool level, uint32_t duration);

/**
 * Getting the hash sum of the last randomly received parcel.
 * @param context Pointer to a WSProtocolDecoderAcurite_606TX instance
 * @return hash Hash sum
 */
uint8_t ws_protocol_decoder_acurite_606tx_get_hash_data(void* context);

/**
 * Serialize data WSProtocolDecoderAcurite_606TX.
 * @param context Pointer to a WSProtocolDecoderAcurite_606TX instance
 * @param flipper_format Pointer to a FlipperFormat instance
 * @param preset The modulation on which the signal was received, SubGhzRadioPreset
 * @return status
 */
SubGhzProtocolStatus ws_protocol_decoder_acurite_606tx_serialize(
    void* context,
    FlipperFormat* flipper_format,
    SubGhzRadioPreset* preset);

/**
 * Deserialize data WSProtocolDecoderAcurite_606TX.
 * @param context Pointer to a WSProtocolDecoderAcurite_606TX instance
 * @param flipper_format Pointer to a FlipperFormat instance
 * @return status
 */
SubGhzProtocolStatus
    ws_protocol_decoder_acurite_606tx_deserialize(void* context, FlipperFormat* flipper_format);

/**
 * Getting a textual representation of the received data.
 * @param context Pointer to a WSProtocolDecoderAcurite_606TX instance
 * @param output Resulting text
 */
void ws_protocol_decoder_acurite_606tx_get_string(void* context, FuriString* output);
