#pragma once

#include "base.h"

#define SUBGHZ_PROTOCOL_REVERS_NAME "Revers"

typedef struct SubGhzProtocolDecoderRevers SubGhzProtocolDecoderRevers;
typedef struct SubGhzProtocolEncoderRevers SubGhzProtocolEncoderRevers;

extern const SubGhzProtocolDecoder subghz_protocol_revers_decoder;
extern const SubGhzProtocolEncoder subghz_protocol_revers_encoder;
extern const SubGhzProtocol subghz_protocol_revers;

/**
 * Allocate SubGhzProtocolEncoderRevers.
 * @param environment Pointer to a SubGhzEnvironment instance
 * @return SubGhzProtocolEncoderRevers* pointer to a SubGhzProtocolEncoderRevers instance
 */
void* subghz_protocol_encoder_revers_alloc(SubGhzEnvironment* environment);

/**
 * Free SubGhzProtocolEncoderRevers.
 * @param context Pointer to a SubGhzProtocolEncoderRevers instance
 */
void subghz_protocol_encoder_revers_free(void* context);

/**
 * Deserialize and generating an upload to send.
 * @param context Pointer to a SubGhzProtocolEncoderRevers instance
 * @param flipper_format Pointer to a FlipperFormat instance
 * @return true On success
 */
bool subghz_protocol_encoder_revers_deserialize(void* context, FlipperFormat* flipper_format);

/**
 * Forced transmission stop.
 * @param context Pointer to a SubGhzProtocolEncoderRevers instance
 */
void subghz_protocol_encoder_revers_stop(void* context);

/**
 * Getting the level and duration of the upload to be loaded into DMA.
 * @param context Pointer to a SubGhzProtocolEncoderRevers instance
 * @return LevelDuration 
 */
LevelDuration subghz_protocol_encoder_revers_yield(void* context);

/**
 * Allocate SubGhzProtocolDecoderRevers.
 * @param environment Pointer to a SubGhzEnvironment instance
 * @return SubGhzProtocolDecoderRevers* pointer to a SubGhzProtocolDecoderRevers instance
 */
void* subghz_protocol_decoder_revers_alloc(SubGhzEnvironment* environment);

/**
 * Free SubGhzProtocolDecoderRevers.
 * @param context Pointer to a SubGhzProtocolDecoderRevers instance
 */
void subghz_protocol_decoder_revers_free(void* context);

/**
 * Reset decoder SubGhzProtocolDecoderRevers.
 * @param context Pointer to a SubGhzProtocolDecoderRevers instance
 */
void subghz_protocol_decoder_revers_reset(void* context);

/**
 * Parse a raw sequence of levels and durations received from the air.
 * @param context Pointer to a SubGhzProtocolDecoderRevers instance
 * @param level Signal level true-high false-low
 * @param duration Duration of this level in, us
 */
void subghz_protocol_decoder_revers_feed(void* context, bool level, uint32_t duration);

/**
 * Getting the hash sum of the last randomly received parcel.
 * @param context Pointer to a SubGhzProtocolDecoderRevers instance
 * @return hash Hash sum
 */
uint8_t subghz_protocol_decoder_revers_get_hash_data(void* context);

/**
 * Serialize data SubGhzProtocolDecoderRevers.
 * @param context Pointer to a SubGhzProtocolDecoderRevers instance
 * @param flipper_format Pointer to a FlipperFormat instance
 * @param preset The modulation on which the signal was received, SubGhzRadioPreset
 * @return true On success
 */
bool subghz_protocol_decoder_revers_serialize(
    void* context,
    FlipperFormat* flipper_format,
    SubGhzRadioPreset* preset);

/**
 * Deserialize data SubGhzProtocolDecoderRevers.
 * @param context Pointer to a SubGhzProtocolDecoderRevers instance
 * @param flipper_format Pointer to a FlipperFormat instance
 * @return true On success
 */
bool subghz_protocol_decoder_revers_deserialize(void* context, FlipperFormat* flipper_format);

/**
 * Getting a textual representation of the received data.
 * @param context Pointer to a SubGhzProtocolDecoderRevers instance
 * @param output Resulting text
 */
void subghz_protocol_decoder_revers_get_string(void* context, FuriString* output);
