#pragma once

#include "base.h"

#define SUBGHZ_PROTOCOL_FANIMATION_NAME "Fanimation"

typedef struct SubGhzProtocolDecoderFanimation SubGhzProtocolDecoderFanimation;
typedef struct SubGhzProtocolEncoderFanimation SubGhzProtocolEncoderFanimation;

extern const SubGhzProtocolDecoder subghz_protocol_fanimation_decoder;
extern const SubGhzProtocolEncoder subghz_protocol_fanimation_encoder;
extern const SubGhzProtocol subghz_protocol_fanimation;

/**
 * Allocate SubGhzProtocolEncoderFanimation.
 * @param environment Pointer to a SubGhzEnvironment instance
 * @return SubGhzProtocolEncoderFanimation* pointer to a SubGhzProtocolEncoderFanimation instance
 */
void* subghz_protocol_encoder_fanimation_alloc(SubGhzEnvironment* environment);

/**
 * Free SubGhzProtocolEncoderFanimation.
 * @param context Pointer to a SubGhzProtocolEncoderFanimation instance
 */
void subghz_protocol_encoder_fanimation_free(void* context);

/**
 * Deserialize and generating an upload to send.
 * @param context Pointer to a SubGhzProtocolEncoderFanimation instance
 * @param flipper_format Pointer to a FlipperFormat instance
 * @return status
 */
SubGhzProtocolStatus
    subghz_protocol_encoder_fanimation_deserialize(void* context, FlipperFormat* flipper_format);

/**
 * Forced transmission stop.
 * @param context Pointer to a SubGhzProtocolEncoderFanimation instance
 */
void subghz_protocol_encoder_fanimation_stop(void* context);

/**
 * Getting the level and duration of the upload to be loaded into DMA.
 * @param context Pointer to a SubGhzProtocolEncoderFanimation instance
 * @return LevelDuration 
 */
LevelDuration subghz_protocol_encoder_fanimation_yield(void* context);

/**
 * Allocate SubGhzProtocolDecoderFanimation.
 * @param environment Pointer to a SubGhzEnvironment instance
 * @return SubGhzProtocolDecoderFanimation* pointer to a SubGhzProtocolDecoderFanimation instance
 */
void* subghz_protocol_decoder_fanimation_alloc(SubGhzEnvironment* environment);

/**
 * Free SubGhzProtocolDecoderFanimation.
 * @param context Pointer to a SubGhzProtocolDecoderFanimation instance
 */
void subghz_protocol_decoder_fanimation_free(void* context);

/**
 * Reset decoder SubGhzProtocolDecoderFanimation.
 * @param context Pointer to a SubGhzProtocolDecoderFanimation instance
 */
void subghz_protocol_decoder_fanimation_reset(void* context);

/**
 * Parse a raw sequence of levels and durations received from the air.
 * @param context Pointer to a SubGhzProtocolDecoderFanimation instance
 * @param level Signal level true-high false-low
 * @param duration Duration of this level in, us
 */
void subghz_protocol_decoder_fanimation_feed(void* context, bool level, uint32_t duration);

/**
 * Getting the hash sum of the last randomly received parcel.
 * @param context Pointer to a SubGhzProtocolDecoderFanimation instance
 * @return hash Hash sum
 */
uint8_t subghz_protocol_decoder_fanimation_get_hash_data(void* context);

/**
 * Serialize data SubGhzProtocolDecoderFanimation.
 * @param context Pointer to a SubGhzProtocolDecoderFanimation instance
 * @param flipper_format Pointer to a FlipperFormat instance
 * @param preset The modulation on which the signal was received, SubGhzRadioPreset
 * @return status
 */
SubGhzProtocolStatus subghz_protocol_decoder_fanimation_serialize(
    void* context,
    FlipperFormat* flipper_format,
    SubGhzRadioPreset* preset);

/**
 * Deserialize data SubGhzProtocolDecoderFanimation.
 * @param context Pointer to a SubGhzProtocolDecoderFanimation instance
 * @param flipper_format Pointer to a FlipperFormat instance
 * @return status
 */
SubGhzProtocolStatus
    subghz_protocol_decoder_fanimation_deserialize(void* context, FlipperFormat* flipper_format);

/**
 * Getting a textual representation of the received data.
 * @param context Pointer to a SubGhzProtocolDecoderFanimation instance
 * @param output Resulting text
 */
void subghz_protocol_decoder_fanimation_get_string(void* context, FuriString* output);
