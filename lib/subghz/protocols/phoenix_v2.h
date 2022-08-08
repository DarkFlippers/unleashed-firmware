#pragma once

#include "base.h"

#define SUBGHZ_PROTOCOL_PHOENIX_V2_NAME "Phoenix_V2"

typedef struct SubGhzProtocolDecoderPhoenix_V2 SubGhzProtocolDecoderPhoenix_V2;
typedef struct SubGhzProtocolEncoderPhoenix_V2 SubGhzProtocolEncoderPhoenix_V2;

extern const SubGhzProtocolDecoder subghz_protocol_phoenix_v2_decoder;
extern const SubGhzProtocolEncoder subghz_protocol_phoenix_v2_encoder;
extern const SubGhzProtocol subghz_protocol_phoenix_v2;

/**
 * Allocate SubGhzProtocolEncoderPhoenix_V2.
 * @param environment Pointer to a SubGhzEnvironment instance
 * @return SubGhzProtocolEncoderPhoenix_V2* pointer to a SubGhzProtocolEncoderPhoenix_V2 instance
 */
void* subghz_protocol_encoder_phoenix_v2_alloc(SubGhzEnvironment* environment);

/**
 * Free SubGhzProtocolEncoderPhoenix_V2.
 * @param context Pointer to a SubGhzProtocolEncoderPhoenix_V2 instance
 */
void subghz_protocol_encoder_phoenix_v2_free(void* context);

/**
 * Deserialize and generating an upload to send.
 * @param context Pointer to a SubGhzProtocolEncoderPhoenix_V2 instance
 * @param flipper_format Pointer to a FlipperFormat instance
 * @return true On success
 */
bool subghz_protocol_encoder_phoenix_v2_deserialize(void* context, FlipperFormat* flipper_format);

/**
 * Forced transmission stop.
 * @param context Pointer to a SubGhzProtocolEncoderPhoenix_V2 instance
 */
void subghz_protocol_encoder_phoenix_v2_stop(void* context);

/**
 * Getting the level and duration of the upload to be loaded into DMA.
 * @param context Pointer to a SubGhzProtocolEncoderPhoenix_V2 instance
 * @return LevelDuration 
 */
LevelDuration subghz_protocol_encoder_phoenix_v2_yield(void* context);

/**
 * Allocate SubGhzProtocolDecoderPhoenix_V2.
 * @param environment Pointer to a SubGhzEnvironment instance
 * @return SubGhzProtocolDecoderPhoenix_V2* pointer to a SubGhzProtocolDecoderPhoenix_V2 instance
 */
void* subghz_protocol_decoder_phoenix_v2_alloc(SubGhzEnvironment* environment);

/**
 * Free SubGhzProtocolDecoderPhoenix_V2.
 * @param context Pointer to a SubGhzProtocolDecoderPhoenix_V2 instance
 */
void subghz_protocol_decoder_phoenix_v2_free(void* context);

/**
 * Reset decoder SubGhzProtocolDecoderPhoenix_V2.
 * @param context Pointer to a SubGhzProtocolDecoderPhoenix_V2 instance
 */
void subghz_protocol_decoder_phoenix_v2_reset(void* context);

/**
 * Parse a raw sequence of levels and durations received from the air.
 * @param context Pointer to a SubGhzProtocolDecoderPhoenix_V2 instance
 * @param level Signal level true-high false-low
 * @param duration Duration of this level in, us
 */
void subghz_protocol_decoder_phoenix_v2_feed(void* context, bool level, uint32_t duration);

/**
 * Getting the hash sum of the last randomly received parcel.
 * @param context Pointer to a SubGhzProtocolDecoderPhoenix_V2 instance
 * @return hash Hash sum
 */
uint8_t subghz_protocol_decoder_phoenix_v2_get_hash_data(void* context);

/**
 * Serialize data SubGhzProtocolDecoderPhoenix_V2.
 * @param context Pointer to a SubGhzProtocolDecoderPhoenix_V2 instance
 * @param flipper_format Pointer to a FlipperFormat instance
 * @param preset The modulation on which the signal was received, SubGhzPresetDefinition
 * @return true On success
 */
bool subghz_protocol_decoder_phoenix_v2_serialize(
    void* context,
    FlipperFormat* flipper_format,
    SubGhzPresetDefinition* preset);

/**
 * Deserialize data SubGhzProtocolDecoderPhoenix_V2.
 * @param context Pointer to a SubGhzProtocolDecoderPhoenix_V2 instance
 * @param flipper_format Pointer to a FlipperFormat instance
 * @return true On success
 */
bool subghz_protocol_decoder_phoenix_v2_deserialize(void* context, FlipperFormat* flipper_format);

/**
 * Getting a textual representation of the received data.
 * @param context Pointer to a SubGhzProtocolDecoderPhoenix_V2 instance
 * @param output Resulting text
 */
void subghz_protocol_decoder_phoenix_v2_get_string(void* context, string_t output);
