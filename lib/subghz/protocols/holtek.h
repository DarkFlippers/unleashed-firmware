#pragma once

#include "base.h"

#define SUBGHZ_PROTOCOL_HOLTEK_NAME "Holtek"

typedef struct SubGhzProtocolDecoderHoltek SubGhzProtocolDecoderHoltek;
typedef struct SubGhzProtocolEncoderHoltek SubGhzProtocolEncoderHoltek;

extern const SubGhzProtocolDecoder subghz_protocol_holtek_decoder;
extern const SubGhzProtocolEncoder subghz_protocol_holtek_encoder;
extern const SubGhzProtocol subghz_protocol_holtek;

/**
 * Allocate SubGhzProtocolEncoderHoltek.
 * @param environment Pointer to a SubGhzEnvironment instance
 * @return SubGhzProtocolEncoderHoltek* pointer to a SubGhzProtocolEncoderHoltek instance
 */
void* subghz_protocol_encoder_holtek_alloc(SubGhzEnvironment* environment);

/**
 * Free SubGhzProtocolEncoderHoltek.
 * @param context Pointer to a SubGhzProtocolEncoderHoltek instance
 */
void subghz_protocol_encoder_holtek_free(void* context);

/**
 * Deserialize and generating an upload to send.
 * @param context Pointer to a SubGhzProtocolEncoderHoltek instance
 * @param flipper_format Pointer to a FlipperFormat instance
 * @return true On success
 */
bool subghz_protocol_encoder_holtek_deserialize(void* context, FlipperFormat* flipper_format);

/**
 * Forced transmission stop.
 * @param context Pointer to a SubGhzProtocolEncoderHoltek instance
 */
void subghz_protocol_encoder_holtek_stop(void* context);

/**
 * Getting the level and duration of the upload to be loaded into DMA.
 * @param context Pointer to a SubGhzProtocolEncoderHoltek instance
 * @return LevelDuration 
 */
LevelDuration subghz_protocol_encoder_holtek_yield(void* context);

/**
 * Allocate SubGhzProtocolDecoderHoltek.
 * @param environment Pointer to a SubGhzEnvironment instance
 * @return SubGhzProtocolDecoderHoltek* pointer to a SubGhzProtocolDecoderHoltek instance
 */
void* subghz_protocol_decoder_holtek_alloc(SubGhzEnvironment* environment);

/**
 * Free SubGhzProtocolDecoderHoltek.
 * @param context Pointer to a SubGhzProtocolDecoderHoltek instance
 */
void subghz_protocol_decoder_holtek_free(void* context);

/**
 * Reset decoder SubGhzProtocolDecoderHoltek.
 * @param context Pointer to a SubGhzProtocolDecoderHoltek instance
 */
void subghz_protocol_decoder_holtek_reset(void* context);

/**
 * Parse a raw sequence of levels and durations received from the air.
 * @param context Pointer to a SubGhzProtocolDecoderHoltek instance
 * @param level Signal level true-high false-low
 * @param duration Duration of this level in, us
 */
void subghz_protocol_decoder_holtek_feed(void* context, bool level, uint32_t duration);

/**
 * Getting the hash sum of the last randomly received parcel.
 * @param context Pointer to a SubGhzProtocolDecoderHoltek instance
 * @return hash Hash sum
 */
uint8_t subghz_protocol_decoder_holtek_get_hash_data(void* context);

/**
 * Serialize data SubGhzProtocolDecoderHoltek.
 * @param context Pointer to a SubGhzProtocolDecoderHoltek instance
 * @param flipper_format Pointer to a FlipperFormat instance
 * @param preset The modulation on which the signal was received, SubGhzPresetDefinition
 * @return true On success
 */
bool subghz_protocol_decoder_holtek_serialize(
    void* context,
    FlipperFormat* flipper_format,
    SubGhzPresetDefinition* preset);

/**
 * Deserialize data SubGhzProtocolDecoderHoltek.
 * @param context Pointer to a SubGhzProtocolDecoderHoltek instance
 * @param flipper_format Pointer to a FlipperFormat instance
 * @return true On success
 */
bool subghz_protocol_decoder_holtek_deserialize(void* context, FlipperFormat* flipper_format);

/**
 * Getting a textual representation of the received data.
 * @param context Pointer to a SubGhzProtocolDecoderHoltek instance
 * @param output Resulting text
 */
void subghz_protocol_decoder_holtek_get_string(void* context, string_t output);
