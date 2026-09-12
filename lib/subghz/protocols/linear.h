#pragma once

#include "base.h"

#define SUBGHZ_PROTOCOL_LINEAR_NAME "Linear"

typedef struct SubGhzProtocolDecoderLinear SubGhzProtocolDecoderLinear;
typedef struct SubGhzProtocolEncoderLinear SubGhzProtocolEncoderLinear;

extern const SubGhzProtocolDecoder subghz_protocol_linear_decoder;
extern const SubGhzProtocolEncoder subghz_protocol_linear_encoder;
extern const SubGhzProtocol subghz_protocol_linear;

/**
 * Allocate SubGhzProtocolEncoderLinear.
 * @param environment Pointer to a SubGhzEnvironment instance
 * @return SubGhzProtocolEncoderLinear* pointer to a SubGhzProtocolEncoderLinear instance
 */
void* subghz_protocol_encoder_linear_alloc(SubGhzEnvironment* environment);

/**
 * Deserialize and generating an upload to send.
 * @param context Pointer to a SubGhzProtocolEncoderLinear instance
 * @param flipper_format Pointer to a FlipperFormat instance
 * @return status
 */
SubGhzProtocolStatus
    subghz_protocol_encoder_linear_deserialize(void* context, FlipperFormat* flipper_format);

/**
 * Allocate SubGhzProtocolDecoderLinear.
 * @param environment Pointer to a SubGhzEnvironment instance
 * @return SubGhzProtocolDecoderLinear* pointer to a SubGhzProtocolDecoderLinear instance
 */
void* subghz_protocol_decoder_linear_alloc(SubGhzEnvironment* environment);

/**
 * Parse a raw sequence of levels and durations received from the air.
 * @param context Pointer to a SubGhzProtocolDecoderLinear instance
 * @param level Signal level true-high false-low
 * @param duration Duration of this level in, us
 */
void subghz_protocol_decoder_linear_feed(void* context, bool level, uint32_t duration);

/**
 * Deserialize data SubGhzProtocolDecoderLinear.
 * @param context Pointer to a SubGhzProtocolDecoderLinear instance
 * @param flipper_format Pointer to a FlipperFormat instance
 * @return status
 */
SubGhzProtocolStatus
    subghz_protocol_decoder_linear_deserialize(void* context, FlipperFormat* flipper_format);

/**
 * Getting a textual representation of the received data.
 * @param context Pointer to a SubGhzProtocolDecoderLinear instance
 * @param output Resulting text
 */
void subghz_protocol_decoder_linear_get_string(void* context, FuriString* output);
