#pragma once

#include <lib/subghz/protocols/base.h>

#include <lib/subghz/blocks/const.h>
#include <lib/subghz/blocks/decoder.h>
#include <lib/subghz/blocks/encoder.h>
#include "ws_generic.h"
#include <lib/subghz/blocks/math.h>

#define WS_PROTOCOL_WENDOX_W6726_NAME "Wendox W6726"

typedef struct WSProtocolDecoderWendoxW6726 WSProtocolDecoderWendoxW6726;
typedef struct WSProtocolEncoderWendoxW6726 WSProtocolEncoderWendoxW6726;

extern const SubGhzProtocolDecoder ws_protocol_wendox_w6726_decoder;
extern const SubGhzProtocolEncoder ws_protocol_wendox_w6726_encoder;
extern const SubGhzProtocol ws_protocol_wendox_w6726;

/**
 * Allocate WSProtocolDecoderWendoxW6726.
 * @param environment Pointer to a SubGhzEnvironment instance
 * @return WSProtocolDecoderWendoxW6726* pointer to a WSProtocolDecoderWendoxW6726 instance
 */
void* ws_protocol_decoder_wendox_w6726_alloc(SubGhzEnvironment* environment);

/**
 * Free WSProtocolDecoderWendoxW6726.
 * @param context Pointer to a WSProtocolDecoderWendoxW6726 instance
 */
void ws_protocol_decoder_wendox_w6726_free(void* context);

/**
 * Reset decoder WSProtocolDecoderWendoxW6726.
 * @param context Pointer to a WSProtocolDecoderWendoxW6726 instance
 */
void ws_protocol_decoder_wendox_w6726_reset(void* context);

/**
 * Parse a raw sequence of levels and durations received from the air.
 * @param context Pointer to a WSProtocolDecoderWendoxW6726 instance
 * @param level Signal level true-high false-low
 * @param duration Duration of this level in, us
 */
void ws_protocol_decoder_wendox_w6726_feed(void* context, bool level, uint32_t duration);

/**
 * Getting the hash sum of the last randomly received parcel.
 * @param context Pointer to a WSProtocolDecoderWendoxW6726 instance
 * @return hash Hash sum
 */
uint8_t ws_protocol_decoder_wendox_w6726_get_hash_data(void* context);

/**
 * Serialize data WSProtocolDecoderWendoxW6726.
 * @param context Pointer to a WSProtocolDecoderWendoxW6726 instance
 * @param flipper_format Pointer to a FlipperFormat instance
 * @param preset The modulation on which the signal was received, SubGhzRadioPreset
 * @return status
 */
SubGhzProtocolStatus ws_protocol_decoder_wendox_w6726_serialize(
    void* context,
    FlipperFormat* flipper_format,
    SubGhzRadioPreset* preset);

/**
 * Deserialize data WSProtocolDecoderWendoxW6726.
 * @param context Pointer to a WSProtocolDecoderWendoxW6726 instance
 * @param flipper_format Pointer to a FlipperFormat instance
 * @return status
 */
SubGhzProtocolStatus
    ws_protocol_decoder_wendox_w6726_deserialize(void* context, FlipperFormat* flipper_format);

/**
 * Getting a textual representation of the received data.
 * @param context Pointer to a WSProtocolDecoderWendoxW6726 instance
 * @param output Resulting text
 */
void ws_protocol_decoder_wendox_w6726_get_string(void* context, FuriString* output);
