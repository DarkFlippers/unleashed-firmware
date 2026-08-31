#pragma once

#include "base.h"

#define SUBGHZ_PROTOCOL_HONEYWELL_NAME "Honeywell Sec"

typedef struct SubGhzProtocolDecoderHoneywell SubGhzProtocolDecoderHoneywell;
typedef struct SubGhzProtocolEncoderHoneywell SubGhzProtocolEncoderHoneywell;

extern const SubGhzProtocolDecoder subghz_protocol_honeywell_decoder;
extern const SubGhzProtocolEncoder subghz_protocol_honeywell_encoder;
extern const SubGhzProtocol subghz_protocol_honeywell;

/**
 * Allocate SubGhzProtocolEncoderHoneywell.
 * @param environment Pointer to a SubGhzEnvironment instance
 * @return SubGhzProtocolEncoderHoneywell* pointer to a SubGhzProtocolEncoderHoneywell instance
 */
void* subghz_protocol_encoder_honeywell_alloc(SubGhzEnvironment* environment);

/**
 * Deserialize and generating an upload to send.
 * @param context Pointer to a SubGhzProtocolEncoderHoneywell instance
 * @param flipper_format Pointer to a FlipperFormat instance
 * @return status
 */
SubGhzProtocolStatus
    subghz_protocol_encoder_honeywell_deserialize(void* context, FlipperFormat* flipper_format);

/**
 * Allocate SubGhzProtocolDecoderHoneywell.
 * @param environment Pointer to a SubGhzEnvironment instance
 * @return SubGhzProtocolDecoderHoneywell* pointer to a SubGhzProtocolDecoderHoneywell instance
 */
void* subghz_protocol_decoder_honeywell_alloc(SubGhzEnvironment* environment);

/**
 * Reset decoder SubGhzProtocolDecoderHoneywell.
 * @param context Pointer to a SubGhzProtocolDecoderHoneywell instance
 */
void subghz_protocol_decoder_honeywell_reset(void* context);

/**
 * Parse a raw sequence of levels and durations received from the air.
 * @param context Pointer to a SubGhzProtocolDecoderHoneywell instance
 * @param level Signal level true-high false-low
 * @param duration Duration of this level in, us
 */
void subghz_protocol_decoder_honeywell_feed(void* context, bool level, uint32_t duration);

/**
 * Deserialize data SubGhzProtocolDecoderHoneywell.
 * @param context Pointer to a SubGhzProtocolDecoderHoneywell instance
 * @param flipper_format Pointer to a FlipperFormat instance
 * @return status
 */
SubGhzProtocolStatus
    subghz_protocol_decoder_honeywell_deserialize(void* context, FlipperFormat* flipper_format);

/**
 * Getting a textual representation of the received data.
 * @param context Pointer to a SubGhzProtocolDecoderHoneywell instance
 * @param output Resulting text
 */
void subghz_protocol_decoder_honeywell_get_string(void* context, FuriString* output);
