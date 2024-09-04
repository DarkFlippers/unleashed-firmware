#pragma once

#include "base.h"

#define SUBGHZ_PROTOCOL_GANGQI_NAME "GangQi"

typedef struct SubGhzProtocolDecoderGangQi SubGhzProtocolDecoderGangQi;
typedef struct SubGhzProtocolEncoderGangQi SubGhzProtocolEncoderGangQi;

extern const SubGhzProtocolDecoder subghz_protocol_gangqi_decoder;
extern const SubGhzProtocolEncoder subghz_protocol_gangqi_encoder;
extern const SubGhzProtocol subghz_protocol_gangqi;

/**
 * Allocate SubGhzProtocolEncoderGangQi.
 * @param environment Pointer to a SubGhzEnvironment instance
 * @return SubGhzProtocolEncoderGangQi* pointer to a SubGhzProtocolEncoderGangQi instance
 */
void* subghz_protocol_encoder_gangqi_alloc(SubGhzEnvironment* environment);

/**
 * Free SubGhzProtocolEncoderGangQi.
 * @param context Pointer to a SubGhzProtocolEncoderGangQi instance
 */
void subghz_protocol_encoder_gangqi_free(void* context);

/**
 * Deserialize and generating an upload to send.
 * @param context Pointer to a SubGhzProtocolEncoderGangQi instance
 * @param flipper_format Pointer to a FlipperFormat instance
 * @return status
 */
SubGhzProtocolStatus
    subghz_protocol_encoder_gangqi_deserialize(void* context, FlipperFormat* flipper_format);

/**
 * Forced transmission stop.
 * @param context Pointer to a SubGhzProtocolEncoderGangQi instance
 */
void subghz_protocol_encoder_gangqi_stop(void* context);

/**
 * Getting the level and duration of the upload to be loaded into DMA.
 * @param context Pointer to a SubGhzProtocolEncoderGangQi instance
 * @return LevelDuration 
 */
LevelDuration subghz_protocol_encoder_gangqi_yield(void* context);

/**
 * Allocate SubGhzProtocolDecoderGangQi.
 * @param environment Pointer to a SubGhzEnvironment instance
 * @return SubGhzProtocolDecoderGangQi* pointer to a SubGhzProtocolDecoderGangQi instance
 */
void* subghz_protocol_decoder_gangqi_alloc(SubGhzEnvironment* environment);

/**
 * Free SubGhzProtocolDecoderGangQi.
 * @param context Pointer to a SubGhzProtocolDecoderGangQi instance
 */
void subghz_protocol_decoder_gangqi_free(void* context);

/**
 * Reset decoder SubGhzProtocolDecoderGangQi.
 * @param context Pointer to a SubGhzProtocolDecoderGangQi instance
 */
void subghz_protocol_decoder_gangqi_reset(void* context);

/**
 * Parse a raw sequence of levels and durations received from the air.
 * @param context Pointer to a SubGhzProtocolDecoderGangQi instance
 * @param level Signal level true-high false-low
 * @param duration Duration of this level in, us
 */
void subghz_protocol_decoder_gangqi_feed(void* context, bool level, uint32_t duration);

/**
 * Getting the hash sum of the last randomly received parcel.
 * @param context Pointer to a SubGhzProtocolDecoderGangQi instance
 * @return hash Hash sum
 */
uint8_t subghz_protocol_decoder_gangqi_get_hash_data(void* context);

/**
 * Serialize data SubGhzProtocolDecoderGangQi.
 * @param context Pointer to a SubGhzProtocolDecoderGangQi instance
 * @param flipper_format Pointer to a FlipperFormat instance
 * @param preset The modulation on which the signal was received, SubGhzRadioPreset
 * @return status
 */
SubGhzProtocolStatus subghz_protocol_decoder_gangqi_serialize(
    void* context,
    FlipperFormat* flipper_format,
    SubGhzRadioPreset* preset);

/**
 * Deserialize data SubGhzProtocolDecoderGangQi.
 * @param context Pointer to a SubGhzProtocolDecoderGangQi instance
 * @param flipper_format Pointer to a FlipperFormat instance
 * @return status
 */
SubGhzProtocolStatus
    subghz_protocol_decoder_gangqi_deserialize(void* context, FlipperFormat* flipper_format);

/**
 * Getting a textual representation of the received data.
 * @param context Pointer to a SubGhzProtocolDecoderGangQi instance
 * @param output Resulting text
 */
void subghz_protocol_decoder_gangqi_get_string(void* context, FuriString* output);
