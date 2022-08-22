#pragma once

#include "base.h"

#define SUBGHZ_PROTOCOL_MAGELLEN_NAME "Magellen"

typedef struct SubGhzProtocolDecoderMagellen SubGhzProtocolDecoderMagellen;
typedef struct SubGhzProtocolEncoderMagellen SubGhzProtocolEncoderMagellen;

extern const SubGhzProtocolDecoder subghz_protocol_magellen_decoder;
extern const SubGhzProtocolEncoder subghz_protocol_magellen_encoder;
extern const SubGhzProtocol subghz_protocol_magellen;

/**
 * Allocate SubGhzProtocolEncoderMagellen.
 * @param environment Pointer to a SubGhzEnvironment instance
 * @return SubGhzProtocolEncoderMagellen* pointer to a SubGhzProtocolEncoderMagellen instance
 */
void* subghz_protocol_encoder_magellen_alloc(SubGhzEnvironment* environment);

/**
 * Free SubGhzProtocolEncoderMagellen.
 * @param context Pointer to a SubGhzProtocolEncoderMagellen instance
 */
void subghz_protocol_encoder_magellen_free(void* context);

/**
 * Deserialize and generating an upload to send.
 * @param context Pointer to a SubGhzProtocolEncoderMagellen instance
 * @param flipper_format Pointer to a FlipperFormat instance
 * @return true On success
 */
bool subghz_protocol_encoder_magellen_deserialize(void* context, FlipperFormat* flipper_format);

/**
 * Forced transmission stop.
 * @param context Pointer to a SubGhzProtocolEncoderMagellen instance
 */
void subghz_protocol_encoder_magellen_stop(void* context);

/**
 * Getting the level and duration of the upload to be loaded into DMA.
 * @param context Pointer to a SubGhzProtocolEncoderMagellen instance
 * @return LevelDuration 
 */
LevelDuration subghz_protocol_encoder_magellen_yield(void* context);

/**
 * Allocate SubGhzProtocolDecoderMagellen.
 * @param environment Pointer to a SubGhzEnvironment instance
 * @return SubGhzProtocolDecoderMagellen* pointer to a SubGhzProtocolDecoderMagellen instance
 */
void* subghz_protocol_decoder_magellen_alloc(SubGhzEnvironment* environment);

/**
 * Free SubGhzProtocolDecoderMagellen.
 * @param context Pointer to a SubGhzProtocolDecoderMagellen instance
 */
void subghz_protocol_decoder_magellen_free(void* context);

/**
 * Reset decoder SubGhzProtocolDecoderMagellen.
 * @param context Pointer to a SubGhzProtocolDecoderMagellen instance
 */
void subghz_protocol_decoder_magellen_reset(void* context);

/**
 * Parse a raw sequence of levels and durations received from the air.
 * @param context Pointer to a SubGhzProtocolDecoderMagellen instance
 * @param level Signal level true-high false-low
 * @param duration Duration of this level in, us
 */
void subghz_protocol_decoder_magellen_feed(void* context, bool level, uint32_t duration);

/**
 * Getting the hash sum of the last randomly received parcel.
 * @param context Pointer to a SubGhzProtocolDecoderMagellen instance
 * @return hash Hash sum
 */
uint8_t subghz_protocol_decoder_magellen_get_hash_data(void* context);

/**
 * Serialize data SubGhzProtocolDecoderMagellen.
 * @param context Pointer to a SubGhzProtocolDecoderMagellen instance
 * @param flipper_format Pointer to a FlipperFormat instance
 * @param preset The modulation on which the signal was received, SubGhzPresetDefinition
 * @return true On success
 */
bool subghz_protocol_decoder_magellen_serialize(
    void* context,
    FlipperFormat* flipper_format,
    SubGhzPresetDefinition* preset);

/**
 * Deserialize data SubGhzProtocolDecoderMagellen.
 * @param context Pointer to a SubGhzProtocolDecoderMagellen instance
 * @param flipper_format Pointer to a FlipperFormat instance
 * @return true On success
 */
bool subghz_protocol_decoder_magellen_deserialize(void* context, FlipperFormat* flipper_format);

/**
 * Getting a textual representation of the received data.
 * @param context Pointer to a SubGhzProtocolDecoderMagellen instance
 * @param output Resulting text
 */
void subghz_protocol_decoder_magellen_get_string(void* context, string_t output);
