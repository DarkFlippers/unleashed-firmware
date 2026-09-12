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
 * Deserialize and generating an upload to send.
 * @param context Pointer to a SubGhzProtocolEncoderKeyFinder instance
 * @param flipper_format Pointer to a FlipperFormat instance
 * @return status
 */
SubGhzProtocolStatus
    subghz_protocol_encoder_keyfinder_deserialize(void* context, FlipperFormat* flipper_format);

/**
 * Allocate SubGhzProtocolDecoderKeyFinder.
 * @param environment Pointer to a SubGhzEnvironment instance
 * @return SubGhzProtocolDecoderKeyFinder* pointer to a SubGhzProtocolDecoderKeyFinder instance
 */
void* subghz_protocol_decoder_keyfinder_alloc(SubGhzEnvironment* environment);

/**
 * Parse a raw sequence of levels and durations received from the air.
 * @param context Pointer to a SubGhzProtocolDecoderKeyFinder instance
 * @param level Signal level true-high false-low
 * @param duration Duration of this level in, us
 */
void subghz_protocol_decoder_keyfinder_feed(void* context, bool level, uint32_t duration);

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
