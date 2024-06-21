#pragma once

#include "base.h"

#define SUBGHZ_PROTOCOL_LEGRAND_NAME "Legrand"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct SubGhzProtocolDecoderLegrand SubGhzProtocolDecoderLegrand;
typedef struct SubGhzProtocolEncoderLegrand SubGhzProtocolEncoderLegrand;

extern const SubGhzProtocolDecoder subghz_protocol_legrand_decoder;
extern const SubGhzProtocolEncoder subghz_protocol_legrand_encoder;
extern const SubGhzProtocol subghz_protocol_legrand;

/**
 * Allocate SubGhzProtocolEncoderLegrand.
 * @param environment Pointer to a SubGhzEnvironment instance
 * @return SubGhzProtocolEncoderLegrand* pointer to a SubGhzProtocolEncoderLegrand instance
 */
void* subghz_protocol_encoder_legrand_alloc(SubGhzEnvironment* environment);

/**
 * Free SubGhzProtocolEncoderLegrand.
 * @param context Pointer to a SubGhzProtocolEncoderLegrand instance
 */
void subghz_protocol_encoder_legrand_free(void* context);

/**
 * Deserialize and generating an upload to send.
 * @param context Pointer to a SubGhzProtocolEncoderLegrand instance
 * @param flipper_format Pointer to a FlipperFormat instance
 * @return status
 */
SubGhzProtocolStatus
    subghz_protocol_encoder_legrand_deserialize(void* context, FlipperFormat* flipper_format);

/**
 * Forced transmission stop.
 * @param context Pointer to a SubGhzProtocolEncoderLegrand instance
 */
void subghz_protocol_encoder_legrand_stop(void* context);

/**
 * Getting the level and duration of the upload to be loaded into DMA.
 * @param context Pointer to a SubGhzProtocolEncoderLegrand instance
 * @return LevelDuration 
 */
LevelDuration subghz_protocol_encoder_legrand_yield(void* context);

/**
 * Allocate SubGhzProtocolDecoderLegrand.
 * @param environment Pointer to a SubGhzEnvironment instance
 * @return SubGhzProtocolDecoderLegrand* pointer to a SubGhzProtocolDecoderLegrand instance
 */
void* subghz_protocol_decoder_legrand_alloc(SubGhzEnvironment* environment);

/**
 * Free SubGhzProtocolDecoderLegrand.
 * @param context Pointer to a SubGhzProtocolDecoderLegrand instance
 */
void subghz_protocol_decoder_legrand_free(void* context);

/**
 * Reset decoder SubGhzProtocolDecoderLegrand.
 * @param context Pointer to a SubGhzProtocolDecoderLegrand instance
 */
void subghz_protocol_decoder_legrand_reset(void* context);

/**
 * Parse a raw sequence of levels and durations received from the air.
 * @param context Pointer to a SubGhzProtocolDecoderLegrand instance
 * @param level Signal level true-high false-low
 * @param duration Duration of this level in, us
 */
void subghz_protocol_decoder_legrand_feed(void* context, bool level, uint32_t duration);

/**
 * Getting the hash sum of the last randomly received parcel.
 * @param context Pointer to a SubGhzProtocolDecoderLegrand instance
 * @return hash Hash sum
 */
uint8_t subghz_protocol_decoder_legrand_get_hash_data(void* context);

/**
 * Serialize data SubGhzProtocolDecoderLegrand.
 * @param context Pointer to a SubGhzProtocolDecoderLegrand instance
 * @param flipper_format Pointer to a FlipperFormat instance
 * @param preset The modulation on which the signal was received, SubGhzRadioPreset
 * @return status
 */
SubGhzProtocolStatus subghz_protocol_decoder_legrand_serialize(
    void* context,
    FlipperFormat* flipper_format,
    SubGhzRadioPreset* preset);

/**
 * Deserialize data SubGhzProtocolDecoderLegrand.
 * @param context Pointer to a SubGhzProtocolDecoderLegrand instance
 * @param flipper_format Pointer to a FlipperFormat instance
 * @return status
 */
SubGhzProtocolStatus
    subghz_protocol_decoder_legrand_deserialize(void* context, FlipperFormat* flipper_format);

/**
 * Getting a textual representation of the received data.
 * @param context Pointer to a SubGhzProtocolDecoderLegrand instance
 * @param output Resulting text
 */
void subghz_protocol_decoder_legrand_get_string(void* context, FuriString* output);

#ifdef __cplusplus
}
#endif
