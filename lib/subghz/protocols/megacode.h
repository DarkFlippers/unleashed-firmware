#pragma once

#include "base.h"

#define SUBGHZ_PROTOCOL_MEGACODE_NAME "MegaCode"

typedef struct SubGhzProtocolDecoderMegaCode SubGhzProtocolDecoderMegaCode;
typedef struct SubGhzProtocolEncoderMegaCode SubGhzProtocolEncoderMegaCode;

extern const SubGhzProtocolDecoder subghz_protocol_megacode_decoder;
extern const SubGhzProtocolEncoder subghz_protocol_megacode_encoder;
extern const SubGhzProtocol subghz_protocol_megacode;

/**
 * Allocate SubGhzProtocolEncoderMegaCode.
 * @param environment Pointer to a SubGhzEnvironment instance
 * @return SubGhzProtocolEncoderMegaCode* pointer to a SubGhzProtocolEncoderMegaCode instance
 */
void* subghz_protocol_encoder_megacode_alloc(SubGhzEnvironment* environment);

/**
 * Deserialize and generating an upload to send.
 * @param context Pointer to a SubGhzProtocolEncoderMegaCode instance
 * @param flipper_format Pointer to a FlipperFormat instance
 * @return status
 */
SubGhzProtocolStatus
    subghz_protocol_encoder_megacode_deserialize(void* context, FlipperFormat* flipper_format);

/**
 * Allocate SubGhzProtocolDecoderMegaCode.
 * @param environment Pointer to a SubGhzEnvironment instance
 * @return SubGhzProtocolDecoderMegaCode* pointer to a SubGhzProtocolDecoderMegaCode instance
 */
void* subghz_protocol_decoder_megacode_alloc(SubGhzEnvironment* environment);

/**
 * Parse a raw sequence of levels and durations received from the air.
 * @param context Pointer to a SubGhzProtocolDecoderMegaCode instance
 * @param level Signal level true-high false-low
 * @param duration Duration of this level in, us
 */
void subghz_protocol_decoder_megacode_feed(void* context, bool level, uint32_t duration);

/**
 * Deserialize data SubGhzProtocolDecoderMegaCode.
 * @param context Pointer to a SubGhzProtocolDecoderMegaCode instance
 * @param flipper_format Pointer to a FlipperFormat instance
 * @return status
 */
SubGhzProtocolStatus
    subghz_protocol_decoder_megacode_deserialize(void* context, FlipperFormat* flipper_format);

/**
 * Getting a textual representation of the received data.
 * @param context Pointer to a SubGhzProtocolDecoderMegaCode instance
 * @param output Resulting text
 */
void subghz_protocol_decoder_megacode_get_string(void* context, FuriString* output);
