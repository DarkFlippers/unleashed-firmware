#pragma once

#include <lib/subghz/protocols/base.h>

#include <lib/subghz/blocks/const.h>
#include <lib/subghz/blocks/decoder.h>
#include <lib/subghz/blocks/encoder.h>
#include "ws_generic.h"
#include <lib/subghz/blocks/math.h>

#define WS_PROTOCOL_ACURITE_592TXR_NAME "Acurite 592TXR"

typedef struct WSProtocolDecoderAcurite_592TXR WSProtocolDecoderAcurite_592TXR;
typedef struct WSProtocolEncoderAcurite_592TXR WSProtocolEncoderAcurite_592TXR;

extern const SubGhzProtocolDecoder ws_protocol_acurite_592txr_decoder;
extern const SubGhzProtocolEncoder ws_protocol_acurite_592txr_encoder;
extern const SubGhzProtocol ws_protocol_acurite_592txr;

/**
 * Allocate WSProtocolDecoderAcurite_592TXR.
 * @param environment Pointer to a SubGhzEnvironment instance
 * @return WSProtocolDecoderAcurite_592TXR* pointer to a WSProtocolDecoderAcurite_592TXR instance
 */
void* ws_protocol_decoder_acurite_592txr_alloc(SubGhzEnvironment* environment);

/**
 * Free WSProtocolDecoderAcurite_592TXR.
 * @param context Pointer to a WSProtocolDecoderAcurite_592TXR instance
 */
void ws_protocol_decoder_acurite_592txr_free(void* context);

/**
 * Reset decoder WSProtocolDecoderAcurite_592TXR.
 * @param context Pointer to a WSProtocolDecoderAcurite_592TXR instance
 */
void ws_protocol_decoder_acurite_592txr_reset(void* context);

/**
 * Parse a raw sequence of levels and durations received from the air.
 * @param context Pointer to a WSProtocolDecoderAcurite_592TXR instance
 * @param level Signal level true-high false-low
 * @param duration Duration of this level in, us
 */
void ws_protocol_decoder_acurite_592txr_feed(void* context, bool level, uint32_t duration);

/**
 * Getting the hash sum of the last randomly received parcel.
 * @param context Pointer to a WSProtocolDecoderAcurite_592TXR instance
 * @return hash Hash sum
 */
uint8_t ws_protocol_decoder_acurite_592txr_get_hash_data(void* context);

/**
 * Serialize data WSProtocolDecoderAcurite_592TXR.
 * @param context Pointer to a WSProtocolDecoderAcurite_592TXR instance
 * @param flipper_format Pointer to a FlipperFormat instance
 * @param preset The modulation on which the signal was received, SubGhzRadioPreset
 * @return status
 */
SubGhzProtocolStatus ws_protocol_decoder_acurite_592txr_serialize(
    void* context,
    FlipperFormat* flipper_format,
    SubGhzRadioPreset* preset);

/**
 * Deserialize data WSProtocolDecoderAcurite_592TXR.
 * @param context Pointer to a WSProtocolDecoderAcurite_592TXR instance
 * @param flipper_format Pointer to a FlipperFormat instance
 * @return status
 */
SubGhzProtocolStatus
    ws_protocol_decoder_acurite_592txr_deserialize(void* context, FlipperFormat* flipper_format);

/**
 * Getting a textual representation of the received data.
 * @param context Pointer to a WSProtocolDecoderAcurite_592TXR instance
 * @param output Resulting text
 */
void ws_protocol_decoder_acurite_592txr_get_string(void* context, FuriString* output);
