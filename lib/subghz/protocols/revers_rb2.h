#pragma once

#include "base.h"

#define SUBGHZ_PROTOCOL_REVERSRB2_NAME "Revers_RB2"

typedef struct SubGhzProtocolDecoderRevers_RB2 SubGhzProtocolDecoderRevers_RB2;
typedef struct SubGhzProtocolEncoderRevers_RB2 SubGhzProtocolEncoderRevers_RB2;

extern const SubGhzProtocolDecoder subghz_protocol_revers_rb2_decoder;
extern const SubGhzProtocolEncoder subghz_protocol_revers_rb2_encoder;
extern const SubGhzProtocol subghz_protocol_revers_rb2;

/**
 * Allocate SubGhzProtocolEncoderRevers_RB2.
 * @param environment Pointer to a SubGhzEnvironment instance
 * @return SubGhzProtocolEncoderRevers_RB2* pointer to a SubGhzProtocolEncoderRevers_RB2 instance
 */
void* subghz_protocol_encoder_revers_rb2_alloc(SubGhzEnvironment* environment);

/**
 * Deserialize and generating an upload to send.
 * @param context Pointer to a SubGhzProtocolEncoderRevers_RB2 instance
 * @param flipper_format Pointer to a FlipperFormat instance
 * @return status
 */
SubGhzProtocolStatus
    subghz_protocol_encoder_revers_rb2_deserialize(void* context, FlipperFormat* flipper_format);

/**
 * Allocate SubGhzProtocolDecoderRevers_RB2.
 * @param environment Pointer to a SubGhzEnvironment instance
 * @return SubGhzProtocolDecoderRevers_RB2* pointer to a SubGhzProtocolDecoderRevers_RB2 instance
 */
void* subghz_protocol_decoder_revers_rb2_alloc(SubGhzEnvironment* environment);

/**
 * Reset decoder SubGhzProtocolDecoderRevers_RB2.
 * @param context Pointer to a SubGhzProtocolDecoderRevers_RB2 instance
 */
void subghz_protocol_decoder_revers_rb2_reset(void* context);

/**
 * Parse a raw sequence of levels and durations received from the air.
 * @param context Pointer to a SubGhzProtocolDecoderRevers_RB2 instance
 * @param level Signal level true-high false-low
 * @param duration Duration of this level in, us
 */
void subghz_protocol_decoder_revers_rb2_feed(void* context, bool level, uint32_t duration);

/**
 * Deserialize data SubGhzProtocolDecoderRevers_RB2.
 * @param context Pointer to a SubGhzProtocolDecoderRevers_RB2 instance
 * @param flipper_format Pointer to a FlipperFormat instance
 * @return status
 */
SubGhzProtocolStatus
    subghz_protocol_decoder_revers_rb2_deserialize(void* context, FlipperFormat* flipper_format);

/**
 * Getting a textual representation of the received data.
 * @param context Pointer to a SubGhzProtocolDecoderRevers_RB2 instance
 * @param output Resulting text
 */
void subghz_protocol_decoder_revers_rb2_get_string(void* context, FuriString* output);
