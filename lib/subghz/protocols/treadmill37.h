#pragma once

#include "base.h"

#define SUBGHZ_PROTOCOL_TREADMILL37_NAME "Treadmill37"

typedef struct SubGhzProtocolDecoderTreadmill37 SubGhzProtocolDecoderTreadmill37;
typedef struct SubGhzProtocolEncoderTreadmill37 SubGhzProtocolEncoderTreadmill37;

extern const SubGhzProtocolDecoder subghz_protocol_treadmill37_decoder;
extern const SubGhzProtocolEncoder subghz_protocol_treadmill37_encoder;
extern const SubGhzProtocol subghz_protocol_treadmill37;

/**
 * Allocate SubGhzProtocolEncoderTreadmill37.
 * @param environment Pointer to a SubGhzEnvironment instance
 * @return SubGhzProtocolEncoderTreadmill37* pointer to a SubGhzProtocolEncoderTreadmill37 instance
 */
void* subghz_protocol_encoder_treadmill37_alloc(SubGhzEnvironment* environment);

/**
 * Free SubGhzProtocolEncoderTreadmill37.
 * @param context Pointer to a SubGhzProtocolEncoderTreadmill37 instance
 */
void subghz_protocol_encoder_treadmill37_free(void* context);

/**
 * Deserialize and generating an upload to send.
 * @param context Pointer to a SubGhzProtocolEncoderTreadmill37 instance
 * @param flipper_format Pointer to a FlipperFormat instance
 * @return status
 */
SubGhzProtocolStatus
    subghz_protocol_encoder_treadmill37_deserialize(void* context, FlipperFormat* flipper_format);

/**
 * Forced transmission stop.
 * @param context Pointer to a SubGhzProtocolEncoderTreadmill37 instance
 */
void subghz_protocol_encoder_treadmill37_stop(void* context);

/**
 * Getting the level and duration of the upload to be loaded into DMA.
 * @param context Pointer to a SubGhzProtocolEncoderTreadmill37 instance
 * @return LevelDuration 
 */
LevelDuration subghz_protocol_encoder_treadmill37_yield(void* context);

/**
 * Allocate SubGhzProtocolDecoderTreadmill37.
 * @param environment Pointer to a SubGhzEnvironment instance
 * @return SubGhzProtocolDecoderTreadmill37* pointer to a SubGhzProtocolDecoderTreadmill37 instance
 */
void* subghz_protocol_decoder_treadmill37_alloc(SubGhzEnvironment* environment);

/**
 * Free SubGhzProtocolDecoderTreadmill37.
 * @param context Pointer to a SubGhzProtocolDecoderTreadmill37 instance
 */
void subghz_protocol_decoder_treadmill37_free(void* context);

/**
 * Reset decoder SubGhzProtocolDecoderTreadmill37.
 * @param context Pointer to a SubGhzProtocolDecoderTreadmill37 instance
 */
void subghz_protocol_decoder_treadmill37_reset(void* context);

/**
 * Parse a raw sequence of levels and durations received from the air.
 * @param context Pointer to a SubGhzProtocolDecoderTreadmill37 instance
 * @param level Signal level true-high false-low
 * @param duration Duration of this level in, us
 */
void subghz_protocol_decoder_treadmill37_feed(void* context, bool level, uint32_t duration);

/**
 * Getting the hash sum of the last randomly received parcel.
 * @param context Pointer to a SubGhzProtocolDecoderTreadmill37 instance
 * @return hash Hash sum
 */
uint8_t subghz_protocol_decoder_treadmill37_get_hash_data(void* context);

/**
 * Serialize data SubGhzProtocolDecoderTreadmill37.
 * @param context Pointer to a SubGhzProtocolDecoderTreadmill37 instance
 * @param flipper_format Pointer to a FlipperFormat instance
 * @param preset The modulation on which the signal was received, SubGhzRadioPreset
 * @return status
 */
SubGhzProtocolStatus subghz_protocol_decoder_treadmill37_serialize(
    void* context,
    FlipperFormat* flipper_format,
    SubGhzRadioPreset* preset);

/**
 * Deserialize data SubGhzProtocolDecoderTreadmill37.
 * @param context Pointer to a SubGhzProtocolDecoderTreadmill37 instance
 * @param flipper_format Pointer to a FlipperFormat instance
 * @return status
 */
SubGhzProtocolStatus
    subghz_protocol_decoder_treadmill37_deserialize(void* context, FlipperFormat* flipper_format);

/**
 * Getting a textual representation of the received data.
 * @param context Pointer to a SubGhzProtocolDecoderTreadmill37 instance
 * @param output Resulting text
 */
void subghz_protocol_decoder_treadmill37_get_string(void* context, FuriString* output);
