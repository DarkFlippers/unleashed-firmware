#pragma once

#include "base.h"

#define SUBGHZ_PROTOCOL_NICE_FLO_NAME "Nice FLO"

typedef struct SubGhzProtocolDecoderNiceFlo SubGhzProtocolDecoderNiceFlo;
typedef struct SubGhzProtocolEncoderNiceFlo SubGhzProtocolEncoderNiceFlo;

extern const SubGhzProtocolDecoder subghz_protocol_nice_flo_decoder;
extern const SubGhzProtocolEncoder subghz_protocol_nice_flo_encoder;
extern const SubGhzProtocol subghz_protocol_nice_flo;

/**
 * Allocate SubGhzProtocolEncoderNiceFlo.
 * @param environment Pointer to a SubGhzEnvironment instance
 * @return SubGhzProtocolEncoderNiceFlo* pointer to a SubGhzProtocolEncoderNiceFlo instance
 */
void* subghz_protocol_encoder_nice_flo_alloc(SubGhzEnvironment* environment);

/**
 * Deserialize and generating an upload to send.
 * @param context Pointer to a SubGhzProtocolEncoderNiceFlo instance
 * @param flipper_format Pointer to a FlipperFormat instance
 * @return status
 */
SubGhzProtocolStatus
    subghz_protocol_encoder_nice_flo_deserialize(void* context, FlipperFormat* flipper_format);

/**
 * Allocate SubGhzProtocolDecoderNiceFlo.
 * @param environment Pointer to a SubGhzEnvironment instance
 * @return SubGhzProtocolDecoderNiceFlo* pointer to a SubGhzProtocolDecoderNiceFlo instance
 */
void* subghz_protocol_decoder_nice_flo_alloc(SubGhzEnvironment* environment);

/**
 * Parse a raw sequence of levels and durations received from the air.
 * @param context Pointer to a SubGhzProtocolDecoderNiceFlo instance
 * @param level Signal level true-high false-low
 * @param duration Duration of this level in, us
 */
void subghz_protocol_decoder_nice_flo_feed(void* context, bool level, uint32_t duration);

/**
 * Deserialize data SubGhzProtocolDecoderNiceFlo.
 * @param context Pointer to a SubGhzProtocolDecoderNiceFlo instance
 * @param flipper_format Pointer to a FlipperFormat instance
 * @return status
 */
SubGhzProtocolStatus
    subghz_protocol_decoder_nice_flo_deserialize(void* context, FlipperFormat* flipper_format);

/**
 * Getting a textual representation of the received data.
 * @param context Pointer to a SubGhzProtocolDecoderNiceFlo instance
 * @param output Resulting text
 */
void subghz_protocol_decoder_nice_flo_get_string(void* context, FuriString* output);
