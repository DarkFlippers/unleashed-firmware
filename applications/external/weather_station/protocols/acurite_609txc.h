#pragma once

#include <lib/subghz/protocols/base.h>

#include <lib/subghz/blocks/const.h>
#include <lib/subghz/blocks/decoder.h>
#include <lib/subghz/blocks/encoder.h>
#include "ws_generic.h"
#include <lib/subghz/blocks/math.h>

#define WS_PROTOCOL_ACURITE_609TXC_NAME "Acurite-609TXC"

typedef struct WSProtocolDecoderAcurite_609TXC WSProtocolDecoderAcurite_609TXC;
typedef struct WSProtocolEncoderAcurite_609TXC WSProtocolEncoderAcurite_609TXC;

extern const SubGhzProtocolDecoder ws_protocol_acurite_609txc_decoder;
extern const SubGhzProtocolEncoder ws_protocol_acurite_609txc_encoder;
extern const SubGhzProtocol ws_protocol_acurite_609txc;

/**
 * Allocate WSProtocolDecoderAcurite_609TXC.
 * @param environment Pointer to a SubGhzEnvironment instance
 * @return WSProtocolDecoderAcurite_609TXC* pointer to a WSProtocolDecoderAcurite_609TXC instance
 */
void* ws_protocol_decoder_acurite_609txc_alloc(SubGhzEnvironment* environment);

/**
 * Free WSProtocolDecoderAcurite_609TXC.
 * @param context Pointer to a WSProtocolDecoderAcurite_609TXC instance
 */
void ws_protocol_decoder_acurite_609txc_free(void* context);

/**
 * Reset decoder WSProtocolDecoderAcurite_609TXC.
 * @param context Pointer to a WSProtocolDecoderAcurite_609TXC instance
 */
void ws_protocol_decoder_acurite_609txc_reset(void* context);

/**
 * Parse a raw sequence of levels and durations received from the air.
 * @param context Pointer to a WSProtocolDecoderAcurite_609TXC instance
 * @param level Signal level true-high false-low
 * @param duration Duration of this level in, us
 */
void ws_protocol_decoder_acurite_609txc_feed(void* context, bool level, uint32_t duration);

/**
 * Getting the hash sum of the last randomly received parcel.
 * @param context Pointer to a WSProtocolDecoderAcurite_609TXC instance
 * @return hash Hash sum
 */
uint8_t ws_protocol_decoder_acurite_609txc_get_hash_data(void* context);

/**
 * Serialize data WSProtocolDecoderAcurite_609TXC.
 * @param context Pointer to a WSProtocolDecoderAcurite_609TXC instance
 * @param flipper_format Pointer to a FlipperFormat instance
 * @param preset The modulation on which the signal was received, SubGhzRadioPreset
 * @return status
 */
SubGhzProtocolStatus ws_protocol_decoder_acurite_609txc_serialize(
    void* context,
    FlipperFormat* flipper_format,
    SubGhzRadioPreset* preset);

/**
 * Deserialize data WSProtocolDecoderAcurite_609TXC.
 * @param context Pointer to a WSProtocolDecoderAcurite_609TXC instance
 * @param flipper_format Pointer to a FlipperFormat instance
 * @return status
 */
SubGhzProtocolStatus
    ws_protocol_decoder_acurite_609txc_deserialize(void* context, FlipperFormat* flipper_format);

/**
 * Getting a textual representation of the received data.
 * @param context Pointer to a WSProtocolDecoderAcurite_609TXC instance
 * @param output Resulting text
 */
void ws_protocol_decoder_acurite_609txc_get_string(void* context, FuriString* output);
