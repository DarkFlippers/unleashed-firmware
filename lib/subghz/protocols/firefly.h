#pragma once

#include "base.h"

#define SUBGHZ_PROTOCOL_FIREFLY_NAME "Firefly"

typedef struct SubGhzProtocolDecoderFirefly SubGhzProtocolDecoderFirefly;
typedef struct SubGhzProtocolEncoderFirefly SubGhzProtocolEncoderFirefly;

extern const SubGhzProtocolDecoder subghz_protocol_firefly_decoder;
extern const SubGhzProtocolEncoder subghz_protocol_firefly_encoder;
extern const SubGhzProtocol subghz_protocol_firefly;

/**
 * Allocate SubGhzProtocolEncoderFirefly.
 * @param environment Pointer to a SubGhzEnvironment instance
 * @return SubGhzProtocolEncoderFirefly* pointer to a SubGhzProtocolEncoderFirefly instance
 */
void* subghz_protocol_encoder_firefly_alloc(SubGhzEnvironment* environment);

/**
 * Free SubGhzProtocolEncoderFirefly.
 * @param context Pointer to a SubGhzProtocolEncoderFirefly instance
 */
void subghz_protocol_encoder_firefly_free(void* context);

/**
 * Deserialize and generating an upload to send.
 * @param context Pointer to a SubGhzProtocolEncoderFirefly instance
 * @param flipper_format Pointer to a FlipperFormat instance
 * @return true On success
 */
bool subghz_protocol_encoder_firefly_deserialize(void* context, FlipperFormat* flipper_format);

/**
 * Forced transmission stop.
 * @param context Pointer to a SubGhzProtocolEncoderFirefly instance
 */
void subghz_protocol_encoder_firefly_stop(void* context);

/**
 * Getting the level and duration of the upload to be loaded into DMA.
 * @param context Pointer to a SubGhzProtocolEncoderFirefly instance
 * @return LevelDuration 
 */
LevelDuration subghz_protocol_encoder_firefly_yield(void* context);

/**
 * Allocate SubGhzProtocolDecoderFirefly.
 * @param environment Pointer to a SubGhzEnvironment instance
 * @return SubGhzProtocolDecoderFirefly* pointer to a SubGhzProtocolDecoderFirefly instance
 */
void* subghz_protocol_decoder_firefly_alloc(SubGhzEnvironment* environment);

/**
 * Free SubGhzProtocolDecoderFirefly.
 * @param context Pointer to a SubGhzProtocolDecoderFirefly instance
 */
void subghz_protocol_decoder_firefly_free(void* context);

/**
 * Reset decoder SubGhzProtocolDecoderFirefly.
 * @param context Pointer to a SubGhzProtocolDecoderFirefly instance
 */
void subghz_protocol_decoder_firefly_reset(void* context);

/**
 * Parse a raw sequence of levels and durations received from the air.
 * @param context Pointer to a SubGhzProtocolDecoderFirefly instance
 * @param level Signal level true-high false-low
 * @param duration Duration of this level in, us
 */
void subghz_protocol_decoder_firefly_feed(void* context, bool level, uint32_t duration);

/**
 * Getting the hash sum of the last randomly received parcel.
 * @param context Pointer to a SubGhzProtocolDecoderFirefly instance
 * @return hash Hash sum
 */
uint8_t subghz_protocol_decoder_firefly_get_hash_data(void* context);

/**
 * Serialize data SubGhzProtocolDecoderFirefly.
 * @param context Pointer to a SubGhzProtocolDecoderFirefly instance
 * @param flipper_format Pointer to a FlipperFormat instance
 * @param frequency The frequency at which the signal was received, Hz
 * @param preset The modulation on which the signal was received, FuriHalSubGhzPreset
 * @return true On success
 */
bool subghz_protocol_decoder_firefly_serialize(
    void* context,
    FlipperFormat* flipper_format,
    uint32_t frequency,
    FuriHalSubGhzPreset preset);

/**
 * Deserialize data SubGhzProtocolDecoderFirefly.
 * @param context Pointer to a SubGhzProtocolDecoderFirefly instance
 * @param flipper_format Pointer to a FlipperFormat instance
 * @return true On success
 */
bool subghz_protocol_decoder_firefly_deserialize(void* context, FlipperFormat* flipper_format);

/**
 * Getting a textual representation of the received data.
 * @param context Pointer to a SubGhzProtocolDecoderFirefly instance
 * @param output Resulting text
 */
void subghz_protocol_decoder_firefly_get_string(void* context, string_t output);
