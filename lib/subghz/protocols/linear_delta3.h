#pragma once

#include "base.h"

#define SUBGHZ_PROTOCOL_LINEAR_DELTA3_NAME "LinearDelta3"

typedef struct SubGhzProtocolDecoderLinearDelta3 SubGhzProtocolDecoderLinearDelta3;
typedef struct SubGhzProtocolEncoderLinearDelta3 SubGhzProtocolEncoderLinearDelta3;

extern const SubGhzProtocolDecoder subghz_protocol_linear_delta3_decoder;
extern const SubGhzProtocolEncoder subghz_protocol_linear_delta3_encoder;
extern const SubGhzProtocol subghz_protocol_linear_delta3;

/**
 * Allocate SubGhzProtocolEncoderLinearDelta3.
 * @param environment Pointer to a SubGhzEnvironment instance
 * @return SubGhzProtocolEncoderLinearDelta3* pointer to a SubGhzProtocolEncoderLinearDelta3 instance
 */
void* subghz_protocol_encoder_linear_delta3_alloc(SubGhzEnvironment* environment);

/**
 * Free SubGhzProtocolEncoderLinearDelta3.
 * @param context Pointer to a SubGhzProtocolEncoderLinearDelta3 instance
 */
void subghz_protocol_encoder_linear_delta3_free(void* context);

/**
 * Deserialize and generating an upload to send.
 * @param context Pointer to a SubGhzProtocolEncoderLinearDelta3 instance
 * @param flipper_format Pointer to a FlipperFormat instance
 * @return status
 */
SubGhzProtocolStatus
    subghz_protocol_encoder_linear_delta3_deserialize(void* context, FlipperFormat* flipper_format);

/**
 * Forced transmission stop.
 * @param context Pointer to a SubGhzProtocolEncoderLinearDelta3 instance
 */
void subghz_protocol_encoder_linear_delta3_stop(void* context);

/**
 * Getting the level and duration of the upload to be loaded into DMA.
 * @param context Pointer to a SubGhzProtocolEncoderLinearDelta3 instance
 * @return LevelDuration 
 */
LevelDuration subghz_protocol_encoder_linear_delta3_yield(void* context);

/**
 * Allocate SubGhzProtocolDecoderLinearDelta3.
 * @param environment Pointer to a SubGhzEnvironment instance
 * @return SubGhzProtocolDecoderLinearDelta3* pointer to a SubGhzProtocolDecoderLinearDelta3 instance
 */
void* subghz_protocol_decoder_linear_delta3_alloc(SubGhzEnvironment* environment);

/**
 * Free SubGhzProtocolDecoderLinearDelta3.
 * @param context Pointer to a SubGhzProtocolDecoderLinearDelta3 instance
 */
void subghz_protocol_decoder_linear_delta3_free(void* context);

/**
 * Reset decoder SubGhzProtocolDecoderLinearDelta3.
 * @param context Pointer to a SubGhzProtocolDecoderLinearDelta3 instance
 */
void subghz_protocol_decoder_linear_delta3_reset(void* context);

/**
 * Parse a raw sequence of levels and durations received from the air.
 * @param context Pointer to a SubGhzProtocolDecoderLinearDelta3 instance
 * @param level Signal level true-high false-low
 * @param duration Duration of this level in, us
 */
void subghz_protocol_decoder_linear_delta3_feed(void* context, bool level, uint32_t duration);

/**
 * Getting the hash sum of the last randomly received parcel.
 * @param context Pointer to a SubGhzProtocolDecoderLinearDelta3 instance
 * @return hash Hash sum
 */
uint8_t subghz_protocol_decoder_linear_delta3_get_hash_data(void* context);

/**
 * Serialize data SubGhzProtocolDecoderLinearDelta3.
 * @param context Pointer to a SubGhzProtocolDecoderLinearDelta3 instance
 * @param flipper_format Pointer to a FlipperFormat instance
 * @param preset The modulation on which the signal was received, SubGhzRadioPreset
 * @return status
 */
SubGhzProtocolStatus subghz_protocol_decoder_linear_delta3_serialize(
    void* context,
    FlipperFormat* flipper_format,
    SubGhzRadioPreset* preset);

/**
 * Deserialize data SubGhzProtocolDecoderLinearDelta3.
 * @param context Pointer to a SubGhzProtocolDecoderLinearDelta3 instance
 * @param flipper_format Pointer to a FlipperFormat instance
 * @return status
 */
SubGhzProtocolStatus
    subghz_protocol_decoder_linear_delta3_deserialize(void* context, FlipperFormat* flipper_format);

/**
 * Getting a textual representation of the received data.
 * @param context Pointer to a SubGhzProtocolDecoderLinearDelta3 instance
 * @param output Resulting text
 */
void subghz_protocol_decoder_linear_delta3_get_string(void* context, FuriString* output);
