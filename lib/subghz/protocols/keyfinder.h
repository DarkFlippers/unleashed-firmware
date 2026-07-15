#pragma once

#include "base.h"

#define SUBGHZ_PROTOCOL_KEYFINDER_NAME "KeyFinder"

typedef struct SubGhzProtocolDecoderKeyFinder SubGhzProtocolDecoderKeyFinder;
typedef struct SubGhzProtocolEncoderKeyFinder SubGhzProtocolEncoderKeyFinder;

extern const SubGhzProtocolDecoder subghz_protocol_keyfinder_decoder;
extern const SubGhzProtocolEncoder subghz_protocol_keyfinder_encoder;
extern const SubGhzProtocol subghz_protocol_keyfinder;

/**
 * Allocate SubGhzProtocolEncoderKeyFinder.
 * @param environment Pointer to a SubGhzEnvironment instance
 * @return SubGhzProtocolEncoderKeyFinder* pointer to a SubGhzProtocolEncoderKeyFinder instance
 */
void* subghz_protocol_encoder_keyfinder_alloc(SubGhzEnvironment* environment);

/**
 * Free SubGhzProtocolEncoderKeyFinder.
 * @param context Pointer to a SubGhzProtocolEncoderKeyFinder instance
 */
void subghz_protocol_encoder_keyfinder_free(void* context);

/**
 * Deserialize and generating an upload to send.
 * @param context Pointer to a SubGhzProtocolEncoderKeyFinder instance
 * @param flipper_format Pointer to a FlipperFormat instance
 * @return status
 */
SubGhzProtocolStatus
    subghz_protocol_encoder_keyfinder_deserialize(void* context, FlipperFormat* flipper_format);

/**
 * Forced transmission stop.
 * @param context Pointer to a SubGhzProtocolEncoderKeyFinder instance
 */
void subghz_protocol_encoder_keyfinder_stop(void* context);

/**
 * Getting the level and duration of the upload to be loaded into DMA.
 * @param context Pointer to a SubGhzProtocolEncoderKeyFinder instance
 * @return LevelDuration 
 */
LevelDuration subghz_protocol_encoder_keyfinder_yield(void* context);

/**
 * Allocate SubGhzProtocolDecoderKeyFinder.
 * @param environment Pointer to a SubGhzEnvironment instance
 * @return SubGhzProtocolDecoderKeyFinder* pointer to a SubGhzProtocolDecoderKeyFinder instance
 */
void* subghz_protocol_decoder_keyfinder_alloc(SubGhzEnvironment* environment);

/**
 * Free SubGhzProtocolDecoderKeyFinder.
 * @param context Pointer to a SubGhzProtocolDecoderKeyFinder instance
 */
void subghz_protocol_decoder_keyfinder_free(void* context);

/**
 * Reset decoder SubGhzProtocolDecoderKeyFinder.
 * @param context Pointer to a SubGhzProtocolDecoderKeyFinder instance
 */
void subghz_protocol_decoder_keyfinder_reset(void* context);

/**
 * Parse a raw sequence of levels and durations received from the air.
 * @param context Pointer to a SubGhzProtocolDecoderKeyFinder instance
 * @param level Signal level true-high false-low
 * @param duration Duration of this level in, us
 */
void subghz_protocol_decoder_keyfinder_feed(void* context, bool level, uint32_t duration);

/**
 * Getting the hash sum of the last randomly received parcel.
 * @param context Pointer to a SubGhzProtocolDecoderKeyFinder instance
 * @return hash Hash sum
 */
uint8_t subghz_protocol_decoder_keyfinder_get_hash_data(void* context);

/**
 * Serialize data SubGhzProtocolDecoderKeyFinder.
 * @param context Pointer to a SubGhzProtocolDecoderKeyFinder instance
 * @param flipper_format Pointer to a FlipperFormat instance
 * @param preset The modulation on which the signal was received, SubGhzRadioPreset
 * @return status
 */
SubGhzProtocolStatus subghz_protocol_decoder_keyfinder_serialize(
    void* context,
    FlipperFormat* flipper_format,
    SubGhzRadioPreset* preset);

/**
 * Deserialize data SubGhzProtocolDecoderKeyFinder.
 * @param context Pointer to a SubGhzProtocolDecoderKeyFinder instance
 * @param flipper_format Pointer to a FlipperFormat instance
 * @return status
 */
SubGhzProtocolStatus
    subghz_protocol_decoder_keyfinder_deserialize(void* context, FlipperFormat* flipper_format);

/**
 * Getting a textual representation of the received data.
 * @param context Pointer to a SubGhzProtocolDecoderKeyFinder instance
 * @param output Resulting text
 */
void subghz_protocol_decoder_keyfinder_get_string(void* context, FuriString* output);
