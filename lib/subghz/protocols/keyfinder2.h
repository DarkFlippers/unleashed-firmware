#pragma once

#include "base.h"

#define SUBGHZ_PROTOCOL_KEYFINDER2_NAME "KeyFinder2"

typedef struct SubGhzProtocolDecoderKeyFinder2 SubGhzProtocolDecoderKeyFinder2;
typedef struct SubGhzProtocolEncoderKeyFinder2 SubGhzProtocolEncoderKeyFinder2;

extern const SubGhzProtocolDecoder subghz_protocol_keyfinder2_decoder;
extern const SubGhzProtocolEncoder subghz_protocol_keyfinder2_encoder;
extern const SubGhzProtocol subghz_protocol_keyfinder2;

/**
 * Allocate SubGhzProtocolEncoderKeyFinder2.
 * @param environment Pointer to a SubGhzEnvironment instance
 * @return SubGhzProtocolEncoderKeyFinder2* pointer to a SubGhzProtocolEncoderKeyFinder2 instance
 */
void* subghz_protocol_encoder_keyfinder2_alloc(SubGhzEnvironment* environment);

/**
 * Deserialize and generating an upload to send.
 * @param context Pointer to a SubGhzProtocolEncoderKeyFinder2 instance
 * @param flipper_format Pointer to a FlipperFormat instance
 * @return status
 */
SubGhzProtocolStatus
    subghz_protocol_encoder_keyfinder2_deserialize(void* context, FlipperFormat* flipper_format);

/**
 * Allocate SubGhzProtocolDecoderKeyFinder2.
 * @param environment Pointer to a SubGhzEnvironment instance
 * @return SubGhzProtocolDecoderKeyFinder2* pointer to a SubGhzProtocolDecoderKeyFinder2 instance
 */
void* subghz_protocol_decoder_keyfinder2_alloc(SubGhzEnvironment* environment);

/**
 * Parse a raw sequence of levels and durations received from the air.
 * @param context Pointer to a SubGhzProtocolDecoderKeyFinder2 instance
 * @param level Signal level true-high false-low
 * @param duration Duration of this level in, us
 */
void subghz_protocol_decoder_keyfinder2_feed(void* context, bool level, uint32_t duration);

/**
 * Deserialize data SubGhzProtocolDecoderKeyFinder2.
 * @param context Pointer to a SubGhzProtocolDecoderKeyFinder2 instance
 * @param flipper_format Pointer to a FlipperFormat instance
 * @return status
 */
SubGhzProtocolStatus
    subghz_protocol_decoder_keyfinder2_deserialize(void* context, FlipperFormat* flipper_format);

/**
 * Getting a textual representation of the received data.
 * @param context Pointer to a SubGhzProtocolDecoderKeyFinder2 instance
 * @param output Resulting text
 */
void subghz_protocol_decoder_keyfinder2_get_string(void* context, FuriString* output);
