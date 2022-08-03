#pragma once

#include "base.h"

#define SUBGHZ_PROTOCOL_LINEAR_NAME "Linear"

typedef struct SubGhzProtocolDecoderLinear SubGhzProtocolDecoderLinear;
typedef struct SubGhzProtocolEncoderLinear SubGhzProtocolEncoderLinear;

extern const SubGhzProtocolDecoder subghz_protocol_linear_decoder;
extern const SubGhzProtocolEncoder subghz_protocol_linear_encoder;
extern const SubGhzProtocol subghz_protocol_linear;

/**
 * Allocate SubGhzProtocolEncoderLinear.
 * @param environment Pointer to a SubGhzEnvironment instance
 * @return SubGhzProtocolEncoderLinear* pointer to a SubGhzProtocolEncoderLinear instance
 */
void* subghz_protocol_encoder_linear_alloc(SubGhzEnvironment* environment);

/**
 * Free SubGhzProtocolEncoderLinear.
 * @param context Pointer to a SubGhzProtocolEncoderLinear instance
 */
void subghz_protocol_encoder_linear_free(void* context);

/**
 * Deserialize and generating an upload to send.
 * @param context Pointer to a SubGhzProtocolEncoderLinear instance
 * @param flipper_format Pointer to a FlipperFormat instance
 * @return true On success
 */
bool subghz_protocol_encoder_linear_deserialize(void* context, FlipperFormat* flipper_format);

/**
 * Forced transmission stop.
 * @param context Pointer to a SubGhzProtocolEncoderLinear instance
 */
void subghz_protocol_encoder_linear_stop(void* context);

/**
 * Getting the level and duration of the upload to be loaded into DMA.
 * @param context Pointer to a SubGhzProtocolEncoderLinear instance
 * @return LevelDuration 
 */
LevelDuration subghz_protocol_encoder_linear_yield(void* context);

/**
 * Allocate SubGhzProtocolDecoderLinear.
 * @param environment Pointer to a SubGhzEnvironment instance
 * @return SubGhzProtocolDecoderLinear* pointer to a SubGhzProtocolDecoderLinear instance
 */
void* subghz_protocol_decoder_linear_alloc(SubGhzEnvironment* environment);

/**
 * Free SubGhzProtocolDecoderLinear.
 * @param context Pointer to a SubGhzProtocolDecoderLinear instance
 */
void subghz_protocol_decoder_linear_free(void* context);

/**
 * Reset decoder SubGhzProtocolDecoderLinear.
 * @param context Pointer to a SubGhzProtocolDecoderLinear instance
 */
void subghz_protocol_decoder_linear_reset(void* context);

/**
 * Parse a raw sequence of levels and durations received from the air.
 * @param context Pointer to a SubGhzProtocolDecoderLinear instance
 * @param level Signal level true-high false-low
 * @param duration Duration of this level in, us
 */
void subghz_protocol_decoder_linear_feed(void* context, bool level, uint32_t duration);

/**
 * Getting the hash sum of the last randomly received parcel.
 * @param context Pointer to a SubGhzProtocolDecoderLinear instance
 * @return hash Hash sum
 */
uint8_t subghz_protocol_decoder_linear_get_hash_data(void* context);

/**
 * Serialize data SubGhzProtocolDecoderLinear.
 * @param context Pointer to a SubGhzProtocolDecoderLinear instance
 * @param flipper_format Pointer to a FlipperFormat instance
 * @param preset The modulation on which the signal was received, SubGhzPresetDefinition
 * @return true On success
 */
bool subghz_protocol_decoder_linear_serialize(
    void* context,
    FlipperFormat* flipper_format,
    SubGhzPresetDefinition* preset);

/**
 * Deserialize data SubGhzProtocolDecoderLinear.
 * @param context Pointer to a SubGhzProtocolDecoderLinear instance
 * @param flipper_format Pointer to a FlipperFormat instance
 * @return true On success
 */
bool subghz_protocol_decoder_linear_deserialize(void* context, FlipperFormat* flipper_format);

/**
 * Getting a textual representation of the received data.
 * @param context Pointer to a SubGhzProtocolDecoderLinear instance
 * @param output Resulting text
 */
void subghz_protocol_decoder_linear_get_string(void* context, string_t output);
