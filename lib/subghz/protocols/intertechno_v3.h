#pragma once

#include "base.h"

#define SUBGHZ_PROTOCOL_INTERTECHNO_V3_NAME "Intertechno_V3"

typedef struct SubGhzProtocolDecoderIntertechno_V3 SubGhzProtocolDecoderIntertechno_V3;
typedef struct SubGhzProtocolEncoderIntertechno_V3 SubGhzProtocolEncoderIntertechno_V3;

extern const SubGhzProtocolDecoder subghz_protocol_intertechno_v3_decoder;
extern const SubGhzProtocolEncoder subghz_protocol_intertechno_v3_encoder;
extern const SubGhzProtocol subghz_protocol_intertechno_v3;

/**
 * Allocate SubGhzProtocolEncoderIntertechno_V3.
 * @param environment Pointer to a SubGhzEnvironment instance
 * @return SubGhzProtocolEncoderIntertechno_V3* pointer to a SubGhzProtocolEncoderIntertechno_V3 instance
 */
void* subghz_protocol_encoder_intertechno_v3_alloc(SubGhzEnvironment* environment);

/**
 * Free SubGhzProtocolEncoderIntertechno_V3.
 * @param context Pointer to a SubGhzProtocolEncoderIntertechno_V3 instance
 */
void subghz_protocol_encoder_intertechno_v3_free(void* context);

/**
 * Deserialize and generating an upload to send.
 * @param context Pointer to a SubGhzProtocolEncoderIntertechno_V3 instance
 * @param flipper_format Pointer to a FlipperFormat instance
 * @return true On success
 */
bool subghz_protocol_encoder_intertechno_v3_deserialize(
    void* context,
    FlipperFormat* flipper_format);

/**
 * Forced transmission stop.
 * @param context Pointer to a SubGhzProtocolEncoderIntertechno_V3 instance
 */
void subghz_protocol_encoder_intertechno_v3_stop(void* context);

/**
 * Getting the level and duration of the upload to be loaded into DMA.
 * @param context Pointer to a SubGhzProtocolEncoderIntertechno_V3 instance
 * @return LevelDuration 
 */
LevelDuration subghz_protocol_encoder_intertechno_v3_yield(void* context);

/**
 * Allocate SubGhzProtocolDecoderIntertechno_V3.
 * @param environment Pointer to a SubGhzEnvironment instance
 * @return SubGhzProtocolDecoderIntertechno_V3* pointer to a SubGhzProtocolDecoderIntertechno_V3 instance
 */
void* subghz_protocol_decoder_intertechno_v3_alloc(SubGhzEnvironment* environment);

/**
 * Free SubGhzProtocolDecoderIntertechno_V3.
 * @param context Pointer to a SubGhzProtocolDecoderIntertechno_V3 instance
 */
void subghz_protocol_decoder_intertechno_v3_free(void* context);

/**
 * Reset decoder SubGhzProtocolDecoderIntertechno_V3.
 * @param context Pointer to a SubGhzProtocolDecoderIntertechno_V3 instance
 */
void subghz_protocol_decoder_intertechno_v3_reset(void* context);

/**
 * Parse a raw sequence of levels and durations received from the air.
 * @param context Pointer to a SubGhzProtocolDecoderIntertechno_V3 instance
 * @param level Signal level true-high false-low
 * @param duration Duration of this level in, us
 */
void subghz_protocol_decoder_intertechno_v3_feed(void* context, bool level, uint32_t duration);

/**
 * Getting the hash sum of the last randomly received parcel.
 * @param context Pointer to a SubGhzProtocolDecoderIntertechno_V3 instance
 * @return hash Hash sum
 */
uint8_t subghz_protocol_decoder_intertechno_v3_get_hash_data(void* context);

/**
 * Serialize data SubGhzProtocolDecoderIntertechno_V3.
 * @param context Pointer to a SubGhzProtocolDecoderIntertechno_V3 instance
 * @param flipper_format Pointer to a FlipperFormat instance
 * @param preset The modulation on which the signal was received, SubGhzPresetDefinition
 * @return true On success
 */
bool subghz_protocol_decoder_intertechno_v3_serialize(
    void* context,
    FlipperFormat* flipper_format,
    SubGhzPresetDefinition* preset);

/**
 * Deserialize data SubGhzProtocolDecoderIntertechno_V3.
 * @param context Pointer to a SubGhzProtocolDecoderIntertechno_V3 instance
 * @param flipper_format Pointer to a FlipperFormat instance
 * @return true On success
 */
bool subghz_protocol_decoder_intertechno_v3_deserialize(
    void* context,
    FlipperFormat* flipper_format);

/**
 * Getting a textual representation of the received data.
 * @param context Pointer to a SubGhzProtocolDecoderIntertechno_V3 instance
 * @param output Resulting text
 */
void subghz_protocol_decoder_intertechno_v3_get_string(void* context, string_t output);
