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
 * Deserialize and generating an upload to send.
 * @param context Pointer to a SubGhzProtocolEncoderLegrand instance
 * @param flipper_format Pointer to a FlipperFormat instance
 * @return status
 */
SubGhzProtocolStatus
    subghz_protocol_encoder_legrand_deserialize(void* context, FlipperFormat* flipper_format);

/**
 * Allocate SubGhzProtocolDecoderLegrand.
 * @param environment Pointer to a SubGhzEnvironment instance
 * @return SubGhzProtocolDecoderLegrand* pointer to a SubGhzProtocolDecoderLegrand instance
 */
void* subghz_protocol_decoder_legrand_alloc(SubGhzEnvironment* environment);

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
