#pragma once

#include "base.h"

#define SUBGHZ_PROTOCOL_PRINCETON_NAME "Princeton"

typedef struct SubGhzProtocolDecoderPrinceton SubGhzProtocolDecoderPrinceton;
typedef struct SubGhzProtocolEncoderPrinceton SubGhzProtocolEncoderPrinceton;

extern const SubGhzProtocolDecoder subghz_protocol_princeton_decoder;
extern const SubGhzProtocolEncoder subghz_protocol_princeton_encoder;
extern const SubGhzProtocol subghz_protocol_princeton;

/**
 * Allocate SubGhzProtocolEncoderPrinceton.
 * @param environment Pointer to a SubGhzEnvironment instance
 * @return SubGhzProtocolEncoderPrinceton* pointer to a SubGhzProtocolEncoderPrinceton instance
 */
void* subghz_protocol_encoder_princeton_alloc(SubGhzEnvironment* environment);

/**
 * Free SubGhzProtocolEncoderPrinceton.
 * @param context Pointer to a SubGhzProtocolEncoderPrinceton instance
 */
void subghz_protocol_encoder_princeton_free(void* context);

/**
 * Deserialize and generating an upload to send.
 * @param context Pointer to a SubGhzProtocolEncoderPrinceton instance
 * @param flipper_format Pointer to a FlipperFormat instance
 * @return status
 */
SubGhzProtocolStatus
    subghz_protocol_encoder_princeton_deserialize(void* context, FlipperFormat* flipper_format);

/**
 * Forced transmission stop.
 * @param context Pointer to a SubGhzProtocolEncoderPrinceton instance
 */
void subghz_protocol_encoder_princeton_stop(void* context);

/**
 * Getting the level and duration of the upload to be loaded into DMA.
 * @param context Pointer to a SubGhzProtocolEncoderPrinceton instance
 * @return LevelDuration 
 */
LevelDuration subghz_protocol_encoder_princeton_yield(void* context);

/**
 * Allocate SubGhzProtocolDecoderPrinceton.
 * @param environment Pointer to a SubGhzEnvironment instance
 * @return SubGhzProtocolDecoderPrinceton* pointer to a SubGhzProtocolDecoderPrinceton instance
 */
void* subghz_protocol_decoder_princeton_alloc(SubGhzEnvironment* environment);

/**
 * Free SubGhzProtocolDecoderPrinceton.
 * @param context Pointer to a SubGhzProtocolDecoderPrinceton instance
 */
void subghz_protocol_decoder_princeton_free(void* context);

/**
 * Reset decoder SubGhzProtocolDecoderPrinceton.
 * @param context Pointer to a SubGhzProtocolDecoderPrinceton instance
 */
void subghz_protocol_decoder_princeton_reset(void* context);

/**
 * Parse a raw sequence of levels and durations received from the air.
 * @param context Pointer to a SubGhzProtocolDecoderPrinceton instance
 * @param level Signal level true-high false-low
 * @param duration Duration of this level in, us
 */
void subghz_protocol_decoder_princeton_feed(void* context, bool level, uint32_t duration);

/**
 * Getting the hash sum of the last randomly received parcel.
 * @param context Pointer to a SubGhzProtocolDecoderPrinceton instance
 * @return hash Hash sum
 */
uint8_t subghz_protocol_decoder_princeton_get_hash_data(void* context);

/**
 * Serialize data SubGhzProtocolDecoderPrinceton.
 * @param context Pointer to a SubGhzProtocolDecoderPrinceton instance
 * @param flipper_format Pointer to a FlipperFormat instance
 * @param preset The modulation on which the signal was received, SubGhzRadioPreset
 * @return status
 */
SubGhzProtocolStatus subghz_protocol_decoder_princeton_serialize(
    void* context,
    FlipperFormat* flipper_format,
    SubGhzRadioPreset* preset);

/**
 * Deserialize data SubGhzProtocolDecoderPrinceton.
 * @param context Pointer to a SubGhzProtocolDecoderPrinceton instance
 * @param flipper_format Pointer to a FlipperFormat instance
 * @return status
 */
SubGhzProtocolStatus
    subghz_protocol_decoder_princeton_deserialize(void* context, FlipperFormat* flipper_format);

/**
 * Getting a textual representation of the received data.
 * @param context Pointer to a SubGhzProtocolDecoderPrinceton instance
 * @param output Resulting text
 */
void subghz_protocol_decoder_princeton_get_string(void* context, FuriString* output);
