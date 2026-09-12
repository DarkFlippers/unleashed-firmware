#pragma once

#include "base.h"

#define SUBGHZ_PROTOCOL_ANSONIC_NAME "Ansonic"

typedef struct SubGhzProtocolDecoderAnsonic SubGhzProtocolDecoderAnsonic;
typedef struct SubGhzProtocolEncoderAnsonic SubGhzProtocolEncoderAnsonic;

extern const SubGhzProtocolDecoder subghz_protocol_ansonic_decoder;
extern const SubGhzProtocolEncoder subghz_protocol_ansonic_encoder;
extern const SubGhzProtocol subghz_protocol_ansonic;

/**
 * Allocate SubGhzProtocolEncoderAnsonic.
 * @param environment Pointer to a SubGhzEnvironment instance
 * @return SubGhzProtocolEncoderAnsonic* pointer to a SubGhzProtocolEncoderAnsonic instance
 */
void* subghz_protocol_encoder_ansonic_alloc(SubGhzEnvironment* environment);

/**
 * Deserialize and generating an upload to send.
 * @param context Pointer to a SubGhzProtocolEncoderAnsonic instance
 * @param flipper_format Pointer to a FlipperFormat instance
 * @return true On success
 */
SubGhzProtocolStatus
    subghz_protocol_encoder_ansonic_deserialize(void* context, FlipperFormat* flipper_format);

/**
 * Allocate SubGhzProtocolDecoderAnsonic.
 * @param environment Pointer to a SubGhzEnvironment instance
 * @return SubGhzProtocolDecoderAnsonic* pointer to a SubGhzProtocolDecoderAnsonic instance
 */
void* subghz_protocol_decoder_ansonic_alloc(SubGhzEnvironment* environment);

/**
 * Parse a raw sequence of levels and durations received from the air.
 * @param context Pointer to a SubGhzProtocolDecoderAnsonic instance
 * @param level Signal level true-high false-low
 * @param duration Duration of this level in, us
 */
void subghz_protocol_decoder_ansonic_feed(void* context, bool level, uint32_t duration);

/**
 * Deserialize data SubGhzProtocolDecoderAnsonic.
 * @param context Pointer to a SubGhzProtocolDecoderAnsonic instance
 * @param flipper_format Pointer to a FlipperFormat instance
 * @return true On success
 */
SubGhzProtocolStatus
    subghz_protocol_decoder_ansonic_deserialize(void* context, FlipperFormat* flipper_format);

/**
 * Getting a textual representation of the received data.
 * @param context Pointer to a SubGhzProtocolDecoderAnsonic instance
 * @param output Resulting text
 */
void subghz_protocol_decoder_ansonic_get_string(void* context, FuriString* output);
