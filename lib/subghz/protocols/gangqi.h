#pragma once

#include "base.h"

#define SUBGHZ_PROTOCOL_GANGQI_NAME "GangQi"

typedef struct SubGhzProtocolDecoderGangQi SubGhzProtocolDecoderGangQi;
typedef struct SubGhzProtocolEncoderGangQi SubGhzProtocolEncoderGangQi;

extern const SubGhzProtocolDecoder subghz_protocol_gangqi_decoder;
extern const SubGhzProtocolEncoder subghz_protocol_gangqi_encoder;
extern const SubGhzProtocol subghz_protocol_gangqi;

/**
 * Allocate SubGhzProtocolEncoderGangQi.
 * @param environment Pointer to a SubGhzEnvironment instance
 * @return SubGhzProtocolEncoderGangQi* pointer to a SubGhzProtocolEncoderGangQi instance
 */
void* subghz_protocol_encoder_gangqi_alloc(SubGhzEnvironment* environment);

/**
 * Deserialize and generating an upload to send.
 * @param context Pointer to a SubGhzProtocolEncoderGangQi instance
 * @param flipper_format Pointer to a FlipperFormat instance
 * @return status
 */
SubGhzProtocolStatus
    subghz_protocol_encoder_gangqi_deserialize(void* context, FlipperFormat* flipper_format);

/**
 * Allocate SubGhzProtocolDecoderGangQi.
 * @param environment Pointer to a SubGhzEnvironment instance
 * @return SubGhzProtocolDecoderGangQi* pointer to a SubGhzProtocolDecoderGangQi instance
 */
void* subghz_protocol_decoder_gangqi_alloc(SubGhzEnvironment* environment);

/**
 * Parse a raw sequence of levels and durations received from the air.
 * @param context Pointer to a SubGhzProtocolDecoderGangQi instance
 * @param level Signal level true-high false-low
 * @param duration Duration of this level in, us
 */
void subghz_protocol_decoder_gangqi_feed(void* context, bool level, uint32_t duration);

/**
 * Deserialize data SubGhzProtocolDecoderGangQi.
 * @param context Pointer to a SubGhzProtocolDecoderGangQi instance
 * @param flipper_format Pointer to a FlipperFormat instance
 * @return status
 */
SubGhzProtocolStatus
    subghz_protocol_decoder_gangqi_deserialize(void* context, FlipperFormat* flipper_format);

/**
 * Getting a textual representation of the received data.
 * @param context Pointer to a SubGhzProtocolDecoderGangQi instance
 * @param output Resulting text
 */
void subghz_protocol_decoder_gangqi_get_string(void* context, FuriString* output);
