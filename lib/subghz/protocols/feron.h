#pragma once

#include "base.h"

#define SUBGHZ_PROTOCOL_FERON_NAME "Feron"

typedef struct SubGhzProtocolDecoderFeron SubGhzProtocolDecoderFeron;
typedef struct SubGhzProtocolEncoderFeron SubGhzProtocolEncoderFeron;

extern const SubGhzProtocolDecoder subghz_protocol_feron_decoder;
extern const SubGhzProtocolEncoder subghz_protocol_feron_encoder;
extern const SubGhzProtocol subghz_protocol_feron;

/**
 * Allocate SubGhzProtocolEncoderFeron.
 * @param environment Pointer to a SubGhzEnvironment instance
 * @return SubGhzProtocolEncoderFeron* pointer to a SubGhzProtocolEncoderFeron instance
 */
void* subghz_protocol_encoder_feron_alloc(SubGhzEnvironment* environment);

/**
 * Deserialize and generating an upload to send.
 * @param context Pointer to a SubGhzProtocolEncoderFeron instance
 * @param flipper_format Pointer to a FlipperFormat instance
 * @return status
 */
SubGhzProtocolStatus
    subghz_protocol_encoder_feron_deserialize(void* context, FlipperFormat* flipper_format);

/**
 * Allocate SubGhzProtocolDecoderFeron.
 * @param environment Pointer to a SubGhzEnvironment instance
 * @return SubGhzProtocolDecoderFeron* pointer to a SubGhzProtocolDecoderFeron instance
 */
void* subghz_protocol_decoder_feron_alloc(SubGhzEnvironment* environment);

/**
 * Parse a raw sequence of levels and durations received from the air.
 * @param context Pointer to a SubGhzProtocolDecoderFeron instance
 * @param level Signal level true-high false-low
 * @param duration Duration of this level in, us
 */
void subghz_protocol_decoder_feron_feed(void* context, bool level, uint32_t duration);

/**
 * Deserialize data SubGhzProtocolDecoderFeron.
 * @param context Pointer to a SubGhzProtocolDecoderFeron instance
 * @param flipper_format Pointer to a FlipperFormat instance
 * @return status
 */
SubGhzProtocolStatus
    subghz_protocol_decoder_feron_deserialize(void* context, FlipperFormat* flipper_format);

/**
 * Getting a textual representation of the received data.
 * @param context Pointer to a SubGhzProtocolDecoderFeron instance
 * @param output Resulting text
 */
void subghz_protocol_decoder_feron_get_string(void* context, FuriString* output);
