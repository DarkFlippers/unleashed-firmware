#pragma once

#include <lib/subghz/protocols/base.h>

#include <lib/subghz/blocks/const.h>
#include <lib/subghz/blocks/decoder.h>
#include <lib/subghz/blocks/encoder.h>
#include "ws_generic.h"
#include <lib/subghz/blocks/math.h>

#define WS_PROTOCOL_AMBIENT_WEATHER_NAME "Ambient_Weather"

typedef struct WSProtocolDecoderAmbient_Weather WSProtocolDecoderAmbient_Weather;
typedef struct WSProtocolEncoderAmbient_Weather WSProtocolEncoderAmbient_Weather;

extern const SubGhzProtocolDecoder ws_protocol_ambient_weather_decoder;
extern const SubGhzProtocolEncoder ws_protocol_ambient_weather_encoder;
extern const SubGhzProtocol ws_protocol_ambient_weather;

/**
 * Allocate WSProtocolDecoderAmbient_Weather.
 * @param environment Pointer to a SubGhzEnvironment instance
 * @return WSProtocolDecoderAmbient_Weather* pointer to a WSProtocolDecoderAmbient_Weather instance
 */
void* ws_protocol_decoder_ambient_weather_alloc(SubGhzEnvironment* environment);

/**
 * Free WSProtocolDecoderAmbient_Weather.
 * @param context Pointer to a WSProtocolDecoderAmbient_Weather instance
 */
void ws_protocol_decoder_ambient_weather_free(void* context);

/**
 * Reset decoder WSProtocolDecoderAmbient_Weather.
 * @param context Pointer to a WSProtocolDecoderAmbient_Weather instance
 */
void ws_protocol_decoder_ambient_weather_reset(void* context);

/**
 * Parse a raw sequence of levels and durations received from the air.
 * @param context Pointer to a WSProtocolDecoderAmbient_Weather instance
 * @param level Signal level true-high false-low
 * @param duration Duration of this level in, us
 */
void ws_protocol_decoder_ambient_weather_feed(void* context, bool level, uint32_t duration);

/**
 * Getting the hash sum of the last randomly received parcel.
 * @param context Pointer to a WSProtocolDecoderAmbient_Weather instance
 * @return hash Hash sum
 */
uint8_t ws_protocol_decoder_ambient_weather_get_hash_data(void* context);

/**
 * Serialize data WSProtocolDecoderAmbient_Weather.
 * @param context Pointer to a WSProtocolDecoderAmbient_Weather instance
 * @param flipper_format Pointer to a FlipperFormat instance
 * @param preset The modulation on which the signal was received, SubGhzRadioPreset
 * @return status
 */
SubGhzProtocolStatus ws_protocol_decoder_ambient_weather_serialize(
    void* context,
    FlipperFormat* flipper_format,
    SubGhzRadioPreset* preset);

/**
 * Deserialize data WSProtocolDecoderAmbient_Weather.
 * @param context Pointer to a WSProtocolDecoderAmbient_Weather instance
 * @param flipper_format Pointer to a FlipperFormat instance
 * @return status
 */
SubGhzProtocolStatus
    ws_protocol_decoder_ambient_weather_deserialize(void* context, FlipperFormat* flipper_format);

/**
 * Getting a textual representation of the received data.
 * @param context Pointer to a WSProtocolDecoderAmbient_Weather instance
 * @param output Resulting text
 */
void ws_protocol_decoder_ambient_weather_get_string(void* context, FuriString* output);
