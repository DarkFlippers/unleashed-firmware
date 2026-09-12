#pragma once

#include "base.h"

#define SUBGHZ_PROTOCOL_HOLLARM_NAME "Hollarm"

typedef struct SubGhzProtocolDecoderHollarm SubGhzProtocolDecoderHollarm;
typedef struct SubGhzProtocolEncoderHollarm SubGhzProtocolEncoderHollarm;

extern const SubGhzProtocolDecoder subghz_protocol_hollarm_decoder;
extern const SubGhzProtocolEncoder subghz_protocol_hollarm_encoder;
extern const SubGhzProtocol subghz_protocol_hollarm;

/**
 * Allocate SubGhzProtocolEncoderHollarm.
 * @param environment Pointer to a SubGhzEnvironment instance
 * @return SubGhzProtocolEncoderHollarm* pointer to a SubGhzProtocolEncoderHollarm instance
 */
void* subghz_protocol_encoder_hollarm_alloc(SubGhzEnvironment* environment);

/**
 * Deserialize and generating an upload to send.
 * @param context Pointer to a SubGhzProtocolEncoderHollarm instance
 * @param flipper_format Pointer to a FlipperFormat instance
 * @return status
 */
SubGhzProtocolStatus
    subghz_protocol_encoder_hollarm_deserialize(void* context, FlipperFormat* flipper_format);

/**
 * Allocate SubGhzProtocolDecoderHollarm.
 * @param environment Pointer to a SubGhzEnvironment instance
 * @return SubGhzProtocolDecoderHollarm* pointer to a SubGhzProtocolDecoderHollarm instance
 */
void* subghz_protocol_decoder_hollarm_alloc(SubGhzEnvironment* environment);

/**
 * Parse a raw sequence of levels and durations received from the air.
 * @param context Pointer to a SubGhzProtocolDecoderHollarm instance
 * @param level Signal level true-high false-low
 * @param duration Duration of this level in, us
 */
void subghz_protocol_decoder_hollarm_feed(void* context, bool level, uint32_t duration);

/**
 * Deserialize data SubGhzProtocolDecoderHollarm.
 * @param context Pointer to a SubGhzProtocolDecoderHollarm instance
 * @param flipper_format Pointer to a FlipperFormat instance
 * @return status
 */
SubGhzProtocolStatus
    subghz_protocol_decoder_hollarm_deserialize(void* context, FlipperFormat* flipper_format);

/**
 * Getting a textual representation of the received data.
 * @param context Pointer to a SubGhzProtocolDecoderHollarm instance
 * @param output Resulting text
 */
void subghz_protocol_decoder_hollarm_get_string(void* context, FuriString* output);
