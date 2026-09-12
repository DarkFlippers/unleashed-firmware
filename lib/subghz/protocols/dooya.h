#pragma once

#include "base.h"

#define SUBGHZ_PROTOCOL_DOOYA_NAME "Dooya"

typedef struct SubGhzProtocolDecoderDooya SubGhzProtocolDecoderDooya;
typedef struct SubGhzProtocolEncoderDooya SubGhzProtocolEncoderDooya;

extern const SubGhzProtocolDecoder subghz_protocol_dooya_decoder;
extern const SubGhzProtocolEncoder subghz_protocol_dooya_encoder;
extern const SubGhzProtocol subghz_protocol_dooya;

/**
 * Allocate SubGhzProtocolEncoderDooya.
 * @param environment Pointer to a SubGhzEnvironment instance
 * @return SubGhzProtocolEncoderDooya* pointer to a SubGhzProtocolEncoderDooya instance
 */
void* subghz_protocol_encoder_dooya_alloc(SubGhzEnvironment* environment);

/**
 * Deserialize and generating an upload to send.
 * @param context Pointer to a SubGhzProtocolEncoderDooya instance
 * @param flipper_format Pointer to a FlipperFormat instance
 * @return status
 */
SubGhzProtocolStatus
    subghz_protocol_encoder_dooya_deserialize(void* context, FlipperFormat* flipper_format);

/**
 * Allocate SubGhzProtocolDecoderDooya.
 * @param environment Pointer to a SubGhzEnvironment instance
 * @return SubGhzProtocolDecoderDooya* pointer to a SubGhzProtocolDecoderDooya instance
 */
void* subghz_protocol_decoder_dooya_alloc(SubGhzEnvironment* environment);

/**
 * Parse a raw sequence of levels and durations received from the air.
 * @param context Pointer to a SubGhzProtocolDecoderDooya instance
 * @param level Signal level true-high false-low
 * @param duration Duration of this level in, us
 */
void subghz_protocol_decoder_dooya_feed(void* context, bool level, uint32_t duration);

/**
 * Deserialize data SubGhzProtocolDecoderDooya.
 * @param context Pointer to a SubGhzProtocolDecoderDooya instance
 * @param flipper_format Pointer to a FlipperFormat instance
 * @return status
 */
SubGhzProtocolStatus
    subghz_protocol_decoder_dooya_deserialize(void* context, FlipperFormat* flipper_format);

/**
 * Getting a textual representation of the received data.
 * @param context Pointer to a SubGhzProtocolDecoderDooya instance
 * @param output Resulting text
 */
void subghz_protocol_decoder_dooya_get_string(void* context, FuriString* output);
