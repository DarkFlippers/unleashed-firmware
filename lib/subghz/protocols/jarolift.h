#pragma once
#include "base.h"

#define SUBGHZ_PROTOCOL_JAROLIFT_NAME "Jarolift"

typedef struct SubGhzProtocolDecoderJarolift SubGhzProtocolDecoderJarolift;
typedef struct SubGhzProtocolEncoderJarolift SubGhzProtocolEncoderJarolift;

extern const SubGhzProtocolDecoder subghz_protocol_jarolift_decoder;
extern const SubGhzProtocolEncoder subghz_protocol_jarolift_encoder;
extern const SubGhzProtocol subghz_protocol_jarolift;

/**
 * Allocate SubGhzProtocolEncoderJarolift.
 * @param environment Pointer to a SubGhzEnvironment instance
 * @return SubGhzProtocolEncoderJarolift* pointer to a SubGhzProtocolEncoderJarolift instance
 */
void* subghz_protocol_encoder_jarolift_alloc(SubGhzEnvironment* environment);

/**
 * Free SubGhzProtocolEncoderJarolift.
 * @param context Pointer to a SubGhzProtocolEncoderJarolift instance
 */
void subghz_protocol_encoder_jarolift_free(void* context);

/**
 * Deserialize and generating an upload to send.
 * @param context Pointer to a SubGhzProtocolEncoderJarolift instance
 * @param flipper_format Pointer to a FlipperFormat instance
 * @return true On success
 */
SubGhzProtocolStatus
    subghz_protocol_encoder_jarolift_deserialize(void* context, FlipperFormat* flipper_format);

/**
 * Forced transmission stop.
 * @param context Pointer to a SubGhzProtocolEncoderJarolift instance
 */
void subghz_protocol_encoder_jarolift_stop(void* context);

/**
 * Getting the level and duration of the upload to be loaded into DMA.
 * @param context Pointer to a SubGhzProtocolEncoderJarolift instance
 * @return LevelDuration 
 */
LevelDuration subghz_protocol_encoder_jarolift_yield(void* context);

/**
 * Allocate SubGhzProtocolDecoderJarolift.
 * @param environment Pointer to a SubGhzEnvironment instance
 * @return SubGhzProtocolDecoderJarolift* pointer to a SubGhzProtocolDecoderJarolift instance
 */
void* subghz_protocol_decoder_jarolift_alloc(SubGhzEnvironment* environment);

/**
 * Free SubGhzProtocolDecoderJarolift.
 * @param context Pointer to a SubGhzProtocolDecoderJarolift instance
 */
void subghz_protocol_decoder_jarolift_free(void* context);

/**
 * Reset decoder SubGhzProtocolDecoderJarolift.
 * @param context Pointer to a SubGhzProtocolDecoderJarolift instance
 */
void subghz_protocol_decoder_jarolift_reset(void* context);

/**
 * Parse a raw sequence of levels and durations received from the air.
 * @param context Pointer to a SubGhzProtocolDecoderJarolift instance
 * @param level Signal level true-high false-low
 * @param duration Duration of this level in, us
 */
void subghz_protocol_decoder_jarolift_feed(void* context, bool level, uint32_t duration);

/**
 * Getting the hash sum of the last randomly received parcel.
 * @param context Pointer to a SubGhzProtocolDecoderJarolift instance
 * @return hash Hash sum
 */
uint8_t subghz_protocol_decoder_jarolift_get_hash_data(void* context);

/**
 * Serialize data SubGhzProtocolDecoderJarolift.
 * @param context Pointer to a SubGhzProtocolDecoderJarolift instance
 * @param flipper_format Pointer to a FlipperFormat instance
 * @param preset The modulation on which the signal was received, SubGhzRadioPreset
 * @return status
 */
SubGhzProtocolStatus subghz_protocol_decoder_jarolift_serialize(
    void* context,
    FlipperFormat* flipper_format,
    SubGhzRadioPreset* preset);

/**
 * Deserialize data SubGhzProtocolDecoderJarolift.
 * @param context Pointer to a SubGhzProtocolDecoderJarolift instance
 * @param flipper_format Pointer to a FlipperFormat instance
 * @return status
 */
SubGhzProtocolStatus
    subghz_protocol_decoder_jarolift_deserialize(void* context, FlipperFormat* flipper_format);

/**
 * Getting a textual representation of the received data.
 * @param context Pointer to a SubGhzProtocolDecoderJarolift instance
 * @param output Resulting text
 */
void subghz_protocol_decoder_jarolift_get_string(void* context, FuriString* output);
