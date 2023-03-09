#pragma once

#include "base.h"

#define SUBGHZ_PROTOCOL_CAME_NAME "CAME"

typedef struct SubGhzProtocolDecoderCame SubGhzProtocolDecoderCame;
typedef struct SubGhzProtocolEncoderCame SubGhzProtocolEncoderCame;

extern const SubGhzProtocolDecoder subghz_protocol_came_decoder;
extern const SubGhzProtocolEncoder subghz_protocol_came_encoder;
extern const SubGhzProtocol subghz_protocol_came;

/**
 * Allocate SubGhzProtocolEncoderCame.
 * @param environment Pointer to a SubGhzEnvironment instance
 * @return SubGhzProtocolEncoderCame* pointer to a SubGhzProtocolEncoderCame instance
 */
void* subghz_protocol_encoder_came_alloc(SubGhzEnvironment* environment);

/**
 * Free SubGhzProtocolEncoderCame.
 * @param context Pointer to a SubGhzProtocolEncoderCame instance
 */
void subghz_protocol_encoder_came_free(void* context);

/**
 * Deserialize and generating an upload to send.
 * @param context Pointer to a SubGhzProtocolEncoderCame instance
 * @param flipper_format Pointer to a FlipperFormat instance
 * @return status
 */
SubGhzProtocolStatus
    subghz_protocol_encoder_came_deserialize(void* context, FlipperFormat* flipper_format);

/**
 * Forced transmission stop.
 * @param context Pointer to a SubGhzProtocolEncoderCame instance
 */
void subghz_protocol_encoder_came_stop(void* context);

/**
 * Getting the level and duration of the upload to be loaded into DMA.
 * @param context Pointer to a SubGhzProtocolEncoderCame instance
 * @return LevelDuration 
 */
LevelDuration subghz_protocol_encoder_came_yield(void* context);

/**
 * Allocate SubGhzProtocolDecoderCame.
 * @param environment Pointer to a SubGhzEnvironment instance
 * @return SubGhzProtocolDecoderCame* pointer to a SubGhzProtocolDecoderCame instance
 */
void* subghz_protocol_decoder_came_alloc(SubGhzEnvironment* environment);

/**
 * Free SubGhzProtocolDecoderCame.
 * @param context Pointer to a SubGhzProtocolDecoderCame instance
 */
void subghz_protocol_decoder_came_free(void* context);

/**
 * Reset decoder SubGhzProtocolDecoderCame.
 * @param context Pointer to a SubGhzProtocolDecoderCame instance
 */
void subghz_protocol_decoder_came_reset(void* context);

/**
 * Parse a raw sequence of levels and durations received from the air.
 * @param context Pointer to a SubGhzProtocolDecoderCame instance
 * @param level Signal level true-high false-low
 * @param duration Duration of this level in, us
 */
void subghz_protocol_decoder_came_feed(void* context, bool level, uint32_t duration);

/**
 * Getting the hash sum of the last randomly received parcel.
 * @param context Pointer to a SubGhzProtocolDecoderCame instance
 * @return hash Hash sum
 */
uint8_t subghz_protocol_decoder_came_get_hash_data(void* context);

/**
 * Serialize data SubGhzProtocolDecoderCame.
 * @param context Pointer to a SubGhzProtocolDecoderCame instance
 * @param flipper_format Pointer to a FlipperFormat instance
 * @param preset The modulation on which the signal was received, SubGhzRadioPreset
 * @return status
 */
SubGhzProtocolStatus subghz_protocol_decoder_came_serialize(
    void* context,
    FlipperFormat* flipper_format,
    SubGhzRadioPreset* preset);

/**
 * Deserialize data SubGhzProtocolDecoderCame.
 * @param context Pointer to a SubGhzProtocolDecoderCame instance
 * @param flipper_format Pointer to a FlipperFormat instance
 * @return status
 */
SubGhzProtocolStatus
    subghz_protocol_decoder_came_deserialize(void* context, FlipperFormat* flipper_format);

/**
 * Getting a textual representation of the received data.
 * @param context Pointer to a SubGhzProtocolDecoderCame instance
 * @param output Resulting text
 */
void subghz_protocol_decoder_came_get_string(void* context, FuriString* output);
