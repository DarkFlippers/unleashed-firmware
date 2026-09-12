#pragma once

#include "base.h"

#define SUBGHZ_PROTOCOL_HAY21_NAME "Hay21"

typedef struct SubGhzProtocolDecoderHay21 SubGhzProtocolDecoderHay21;
typedef struct SubGhzProtocolEncoderHay21 SubGhzProtocolEncoderHay21;

extern const SubGhzProtocolDecoder subghz_protocol_hay21_decoder;
extern const SubGhzProtocolEncoder subghz_protocol_hay21_encoder;
extern const SubGhzProtocol subghz_protocol_hay21;

/**
 * Allocate SubGhzProtocolEncoderHay21.
 * @param environment Pointer to a SubGhzEnvironment instance
 * @return SubGhzProtocolEncoderHay21* pointer to a SubGhzProtocolEncoderHay21 instance
 */
void* subghz_protocol_encoder_hay21_alloc(SubGhzEnvironment* environment);

/**
 * Deserialize and generating an upload to send.
 * @param context Pointer to a SubGhzProtocolEncoderHay21 instance
 * @param flipper_format Pointer to a FlipperFormat instance
 * @return status
 */
SubGhzProtocolStatus
    subghz_protocol_encoder_hay21_deserialize(void* context, FlipperFormat* flipper_format);

/**
 * Allocate SubGhzProtocolDecoderHay21.
 * @param environment Pointer to a SubGhzEnvironment instance
 * @return SubGhzProtocolDecoderHay21* pointer to a SubGhzProtocolDecoderHay21 instance
 */
void* subghz_protocol_decoder_hay21_alloc(SubGhzEnvironment* environment);

/**
 * Parse a raw sequence of levels and durations received from the air.
 * @param context Pointer to a SubGhzProtocolDecoderHay21 instance
 * @param level Signal level true-high false-low
 * @param duration Duration of this level in, us
 */
void subghz_protocol_decoder_hay21_feed(void* context, bool level, uint32_t duration);

/**
 * Deserialize data SubGhzProtocolDecoderHay21.
 * @param context Pointer to a SubGhzProtocolDecoderHay21 instance
 * @param flipper_format Pointer to a FlipperFormat instance
 * @return status
 */
SubGhzProtocolStatus
    subghz_protocol_decoder_hay21_deserialize(void* context, FlipperFormat* flipper_format);

/**
 * Getting a textual representation of the received data.
 * @param context Pointer to a SubGhzProtocolDecoderHay21 instance
 * @param output Resulting text
 */
void subghz_protocol_decoder_hay21_get_string(void* context, FuriString* output);
