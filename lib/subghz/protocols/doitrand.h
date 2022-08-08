#pragma once

#include "base.h"

#define SUBGHZ_PROTOCOL_DOITRAND_NAME "Doitrand"

typedef struct SubGhzProtocolDecoderDoitrand SubGhzProtocolDecoderDoitrand;
typedef struct SubGhzProtocolEncoderDoitrand SubGhzProtocolEncoderDoitrand;

extern const SubGhzProtocolDecoder subghz_protocol_doitrand_decoder;
extern const SubGhzProtocolEncoder subghz_protocol_doitrand_encoder;
extern const SubGhzProtocol subghz_protocol_doitrand;

/**
 * Allocate SubGhzProtocolEncoderDoitrand.
 * @param environment Pointer to a SubGhzEnvironment instance
 * @return SubGhzProtocolEncoderDoitrand* pointer to a SubGhzProtocolEncoderDoitrand instance
 */
void* subghz_protocol_encoder_doitrand_alloc(SubGhzEnvironment* environment);

/**
 * Free SubGhzProtocolEncoderDoitrand.
 * @param context Pointer to a SubGhzProtocolEncoderDoitrand instance
 */
void subghz_protocol_encoder_doitrand_free(void* context);

/**
 * Deserialize and generating an upload to send.
 * @param context Pointer to a SubGhzProtocolEncoderDoitrand instance
 * @param flipper_format Pointer to a FlipperFormat instance
 * @return true On success
 */
bool subghz_protocol_encoder_doitrand_deserialize(void* context, FlipperFormat* flipper_format);

/**
 * Forced transmission stop.
 * @param context Pointer to a SubGhzProtocolEncoderDoitrand instance
 */
void subghz_protocol_encoder_doitrand_stop(void* context);

/**
 * Getting the level and duration of the upload to be loaded into DMA.
 * @param context Pointer to a SubGhzProtocolEncoderDoitrand instance
 * @return LevelDuration 
 */
LevelDuration subghz_protocol_encoder_doitrand_yield(void* context);

/**
 * Allocate SubGhzProtocolDecoderDoitrand.
 * @param environment Pointer to a SubGhzEnvironment instance
 * @return SubGhzProtocolDecoderDoitrand* pointer to a SubGhzProtocolDecoderDoitrand instance
 */
void* subghz_protocol_decoder_doitrand_alloc(SubGhzEnvironment* environment);

/**
 * Free SubGhzProtocolDecoderDoitrand.
 * @param context Pointer to a SubGhzProtocolDecoderDoitrand instance
 */
void subghz_protocol_decoder_doitrand_free(void* context);

/**
 * Reset decoder SubGhzProtocolDecoderDoitrand.
 * @param context Pointer to a SubGhzProtocolDecoderDoitrand instance
 */
void subghz_protocol_decoder_doitrand_reset(void* context);

/**
 * Parse a raw sequence of levels and durations received from the air.
 * @param context Pointer to a SubGhzProtocolDecoderDoitrand instance
 * @param level Signal level true-high false-low
 * @param duration Duration of this level in, us
 */
void subghz_protocol_decoder_doitrand_feed(void* context, bool level, uint32_t duration);

/**
 * Getting the hash sum of the last randomly received parcel.
 * @param context Pointer to a SubGhzProtocolDecoderDoitrand instance
 * @return hash Hash sum
 */
uint8_t subghz_protocol_decoder_doitrand_get_hash_data(void* context);

/**
 * Serialize data SubGhzProtocolDecoderDoitrand.
 * @param context Pointer to a SubGhzProtocolDecoderDoitrand instance
 * @param flipper_format Pointer to a FlipperFormat instance
 * @param preset The modulation on which the signal was received, SubGhzPresetDefinition
 * @return true On success
 */
bool subghz_protocol_decoder_doitrand_serialize(
    void* context,
    FlipperFormat* flipper_format,
    SubGhzPresetDefinition* preset);

/**
 * Deserialize data SubGhzProtocolDecoderDoitrand.
 * @param context Pointer to a SubGhzProtocolDecoderDoitrand instance
 * @param flipper_format Pointer to a FlipperFormat instance
 * @return true On success
 */
bool subghz_protocol_decoder_doitrand_deserialize(void* context, FlipperFormat* flipper_format);

/**
 * Getting a textual representation of the received data.
 * @param context Pointer to a SubGhzProtocolDecoderDoitrand instance
 * @param output Resulting text
 */
void subghz_protocol_decoder_doitrand_get_string(void* context, string_t output);
